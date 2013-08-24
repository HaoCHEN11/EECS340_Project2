#include "Minet.h"
#include "packet_factory.h"
#include <iostream>

#define DF_WINSIZE 14600

using namespace std;

void printPacket(const Packet p){
  IPHeader iph=p.FindHeader(Headers::IPHeader);
  TCPHeader tcph=p.FindHeader(Headers::TCPHeader);
	
  cerr << "TCP Packet: IP Header is "<<iph<<" and ";
  cerr << "TCP Header is "<<tcph << " and ";
  cerr << "Checksum is " << (tcph.IsCorrectChecksum(p) ? "VALID" : "INVALID") << " and ";
  cerr << "Payload is ";
  Packet tmp(p);
  tmp.GetPayload().Print(cerr);  // the const type p cannot call GetPayload()
  cerr << "\n";
}

Buffer & extractPayload(Packet * p){
  // a safe function
  IPHeader iph = p->FindHeader(Headers::IPHeader);
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned char iphlen, tcphlen;
  unsigned short len;
  iph.GetTotalLength(len);
  iph.GetHeaderLength(iphlen);
  tcph.GetHeaderLen(tcphlen);
  unsigned int datalen = len - iphlen * 4 - tcphlen * 4;  
  
  return p->GetPayload().ExtractFront(datalen);

}

void parseTCP(Packet * p){
  unsigned tcphlen=TCPHeader::EstimateTCPHeaderLength(*p);
  p->ExtractHeaderFromPayload<TCPHeader>(tcphlen);
}

Packet * syn(ConnectionToStateMapping<TCPState> csm){
  Packet * out = new Packet();
  IPHeader * ih = new IPHeader();
  ih->SetProtocol(IP_PROTO_TCP);
  ih->SetSourceIP(csm.connection.src);
  ih->SetDestIP(csm.connection.dest);
  ih->SetTotalLength(0 + TCP_HEADER_BASE_LENGTH + IP_HEADER_BASE_LENGTH);
  out->PushFrontHeader(*ih);

  TCPHeader * th = new TCPHeader();
  th->SetSourcePort(csm.connection.srcport, *out);
  th->SetDestPort(csm.connection.destport, *out);
  th->SetHeaderLen(TCP_HEADER_BASE_LENGTH / 4, *out);
  th->SetSeqNum(csm.state.GetLastSent() + 1, *out); //Initial sequence number
  unsigned char fo = 0;
  SET_SYN(fo);
  th->SetFlags(fo, *out);
  th->SetWinSize(csm.state.GetN(), *out);
  out->PushBackHeader(*th);
   
  return out;
}

Packet * synack(ConnectionToStateMapping<TCPState> csm){
  Packet * out = new Packet();
  IPHeader * ih = new IPHeader();
  ih->SetProtocol(IP_PROTO_TCP);
  ih->SetSourceIP(csm.connection.src);
  ih->SetDestIP(csm.connection.dest);
  ih->SetTotalLength(0 + TCP_HEADER_BASE_LENGTH + IP_HEADER_BASE_LENGTH);
  out->PushFrontHeader(*ih);

  TCPHeader * th = new TCPHeader();
  th->SetSourcePort(csm.connection.srcport, *out);
  th->SetDestPort(csm.connection.destport, *out);
  th->SetHeaderLen(TCP_HEADER_BASE_LENGTH / 4, *out);
  th->SetSeqNum(csm.state.GetLastSent() + 1, *out); //Initial sequence number
  th->SetAckNum(csm.state.GetLastRecvd() + 1, *out);
  unsigned char fo = 0;
  SET_SYN(fo);
  SET_ACK(fo);
  th->SetFlags(fo, *out);
  th->SetWinSize(csm.state.GetN(), *out);
  out->PushBackHeader(*th);
 
  return out;
}

Packet * ack(ConnectionToStateMapping<TCPState> csm, Buffer * data){
  Packet * out = NULL;
  unsigned short datalen;
  if(data == NULL){
    out = new Packet();
    datalen = 0;
  } else if(data->GetSize() == 0){
    out = new Packet();
    datalen = 0;
  } else{
    out = new Packet(*data);
    datalen = (unsigned short) data->GetSize();
  }

  IPHeader * ih = new IPHeader();
  ih->SetProtocol(IP_PROTO_TCP);
  ih->SetSourceIP(csm.connection.src);
  ih->SetDestIP(csm.connection.dest);
  ih->SetTotalLength(datalen + TCP_HEADER_BASE_LENGTH + IP_HEADER_BASE_LENGTH);
  out->PushFrontHeader(*ih);

  TCPHeader * th = new TCPHeader();
  th->SetSourcePort(csm.connection.srcport, *out);
  th->SetDestPort(csm.connection.destport, *out);
  th->SetHeaderLen(TCP_HEADER_BASE_LENGTH / 4, *out);
  th->SetSeqNum(csm.state.GetLastSent() + 1, *out); 
  th->SetAckNum(csm.state.GetLastRecvd() + 1, *out);
  unsigned char fo = 0;
  SET_ACK(fo);
  th->SetFlags(fo, *out);
  th->SetWinSize(csm.state.GetN(), *out);
  out->PushBackHeader(*th);
  
  return out;
}

Packet * fin(ConnectionToStateMapping<TCPState> csm){
  Packet * out = new Packet();
  IPHeader * ih = new IPHeader();
  ih->SetProtocol(IP_PROTO_TCP);
  ih->SetSourceIP(csm.connection.src);
  ih->SetDestIP(csm.connection.dest);
  ih->SetTotalLength(0 + TCP_HEADER_BASE_LENGTH + IP_HEADER_BASE_LENGTH);
  out->PushFrontHeader(*ih);

  TCPHeader * th = new TCPHeader();
  th->SetSourcePort(csm.connection.srcport, *out);
  th->SetDestPort(csm.connection.destport, *out);
  th->SetHeaderLen(TCP_HEADER_BASE_LENGTH / 4, *out);
  th->SetSeqNum(csm.state.GetLastSent(), *out); 
  unsigned char fo = 0;
  SET_FIN(fo);
  th->SetFlags(fo, *out);
  th->SetWinSize(csm.state.GetN(), *out);
  out->PushBackHeader(*th);

  return out; 

}
//-------modified------------
Packet * finack(ConnectionToStateMapping<TCPState> csm){
  Packet * out = new Packet();
  IPHeader * ih = new IPHeader();
  ih->SetProtocol(IP_PROTO_TCP);
  ih->SetSourceIP(csm.connection.src);
  ih->SetDestIP(csm.connection.dest);
  ih->SetTotalLength(0 + TCP_HEADER_BASE_LENGTH + IP_HEADER_BASE_LENGTH);
  out->PushFrontHeader(*ih);

  TCPHeader * th = new TCPHeader();
  th->SetSourcePort(csm.connection.srcport, *out);
  th->SetDestPort(csm.connection.destport, *out);
  th->SetHeaderLen(TCP_HEADER_BASE_LENGTH / 4, *out);
  th->SetSeqNum(csm.state.GetLastSent() + 1, *out); //Initial sequence number
  th->SetAckNum(csm.state.GetLastRecvd() + 1, *out);
  unsigned char fo = 0;
  SET_FIN(fo);
  SET_ACK(fo);
  th->SetFlags(fo, *out);
  th->SetWinSize(csm.state.GetN(), *out);
  out->PushBackHeader(*th);
  return out;
}

Packet * rst(ConnectionToStateMapping<TCPState> csm){
  //TODO
  return NULL;
}
