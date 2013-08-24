#include <stdlib.h>
#include "Minet.h"
#include "tcpstate.h"
#include "tcp_transit.h"
#include "packet_factory.h"
#include "tcp_timer.cc"
#include <iostream>

#define time_timeWait 5
#define time_established 10

using namespace std;

// generate a random source port
unsigned short getPort(){
  return rand()%10000 + 50000;
}

// a pretty delicate function. Be careful to change it.
// change lastAcked and lastRecvd after receiving packet
// Return true if it accepts the data.
bool postRecv(ConnectionToStateMapping<TCPState> *csm, Packet * p){
  
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned int datalen;
  unsigned char flags;
  tcph.GetFlags(flags);
  if(IS_SYN(flags)){
    datalen = 1;
  } else if (IS_FIN(flags)){
    datalen = 1;
  } else if (IS_ACK(flags)){
    datalen = extractPayload(p).GetSize();
  }

  unsigned int seqnum;
  unsigned int acknum;
  tcph.GetSeqNum(seqnum);
  tcph.GetAckNum(acknum);
  
  unsigned int lastAcked, lastRecvd;
  lastAcked = csm->state.GetLastAcked();
  cerr << "[postRecv] last ack number " << lastAcked << " packet ack number " << acknum << "\n";
  csm->state.last_acked = (acknum > lastAcked + 1 ? acknum - 1 :lastAcked );
  cerr << "[postRecv] last ack number now is " << csm->state.GetLastAcked() << "\n";
  lastRecvd = csm->state.GetLastRecvd();
  if(seqnum == lastRecvd + 1 || lastRecvd == 0){ // important lastRecvd == 0 initial case!!!
    csm->state.SetLastRecvd(seqnum + datalen - 1);
    return true;
  } else{
    return false;
  }

}

// change lastSent after sending.
void postSend(ConnectionToStateMapping<TCPState> *csm, Packet * p){
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned int datalen;
  unsigned char flags;
  tcph.GetFlags(flags);
  if(IS_SYN(flags)){
    datalen = 1;
  } else if(IS_ACK(flags)){
    datalen = extractPayload(p).GetSize();
  }

  unsigned int seqnum;
  tcph.GetSeqNum(seqnum);

  unsigned int lastSent;
  lastSent = csm->state.GetLastSent();
  csm->state.SetLastSent(lastSent > seqnum + datalen - 1 ? lastSent : seqnum + datalen - 1);
}

// The number of bytes allowed to be sent
unsigned int preSend(ConnectionToStateMapping<TCPState> * csm, Buffer & data){
  return csm->state.GetN() - (csm->state.GetLastSent() - csm->state.GetLastAcked());
}

void closedToListen(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){
  // Reply a status with only error code set
  SockRequestResponse status;
  status.type = STATUS;
  status.error = EOK;
  MinetSend(*sock, status);

  csm->state.SetState(LISTEN);
}
  
void closedToSynSent(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){

  // Something special here, change the src port number of the connection
  // csm->connection.srcport = getPort();
  
  // Send out syn for active open
  Packet * out = syn(*csm);
  cerr << "[closedToSynSent] packet to be sent out\n";
  printPacket(*out);
  // a fake one 
  MinetSend(*mux, *out);
  sleep(2);
  // a real one
  MinetSend(*mux, *out);

  // Post sent
  postSend(csm, out);
 
  // Sock reply, return the STATUS with same conn, no data, no bytes count, error code
  SockRequestResponse status;
  status.type = STATUS;
  status.connection = csm->connection;
  status.error = EOK;
  MinetSend(*sock, status);
  
  // State Transfer
  csm->state.SetState(SYN_SENT);
}

void listenToSynRcvd(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){
  cerr << "[listenToSynRcvd] start\n";
  
  //Something special here, augment the tuple for the listen connection
  IPHeader iph = p->FindHeader(Headers::IPHeader);
  iph.GetSourceIP(csm->connection.dest);
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  tcph.GetSourcePort(csm->connection.destport);

  // Post Recv
  postRecv(csm, p);
  cerr << "[listenToSynRcvd] post recv success\n";
  
  // Send the synack data
  cerr << "[listenToSynRcvd] Send from " << csm->connection.src << " to  " <<csm->connection.dest << "\n";
  Packet * out = synack(*csm);
  cerr << "[listenToSynRcvd] packet to be sent out\n"; 
  printPacket(*out);
  MinetSend(*mux, *out);
  
  // Post Send
  postSend(csm, out);

  // Timer

  // State Transfer
  csm->state.SetState(SYN_RCVD);
}

void listenToSynSent(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){
  //TODO
}

void synRcvdToEstablished(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){ 
  // Post Recv
  postRecv(csm, p);

  //inform the sock layer about the established state
  SockRequestResponse write;
  write.type = WRITE;
  write.bytes = 0;
  write.connection =  csm->connection;
  write.error = EOK;
  MinetSend(*sock, write);
  cerr << "[synRcvdToEstablished] Tell Sock I am ready !! " << write << "\n";

  // State Transfer; 
  csm->state.SetState(ESTABLISHED);

}  

void synRcvdToListen(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){ }

void synRcvdToClosed(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){ }
//----------modified---------
void synRcvdToFinWait1(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){ 
  Packet * out = finack(*csm);
  cerr << "[synRcvdToFinWait1] packet to be sent out\n";
  printPacket(*out);
  // send a packet
  MinetSend(*mux, *out);

  // Post sent
  postSend(csm, out);
 
  // Sock reply, return the STATUS with same conn, no data, no bytes count, error code
  SockRequestResponse status;
  status.type = STATUS;
  status.connection = csm->connection;
  status.error = EOK;
  MinetSend(*sock, status);
  
  // State Transfer
  csm->state.SetState(FIN_WAIT1);
}

void synSentToEstablished(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){
   // Post Recv
   postRecv(csm, p);
   cerr << "[synSentToEstablished] after receving synack connection state last ack " << csm->state.GetLastAcked() << "\n";
   // Send the data
   Packet * out = ack(*csm, NULL);
   cerr << "[synSentToEstablished] packet to be sent out\n";
   printPacket(*out);
   MinetSend(*mux, *out);
   
   // Post Send
   postSend(csm, out);
   
   // check the seqnum
   cerr << "[synSentToEstablished] lastSent is " << csm->state.GetLastSent() << " lastAcked " << csm->state.GetLastAcked() << "\n";
   // inform the sock layer about the established state.
   SockRequestResponse write;
   write.type = WRITE;
   write.connection = csm->connection;
   write.bytes = 0;
   write.error = EOK;
   MinetSend(*sock, write);
   cerr << "[synSentToEstablished] Tell sock I am ready !!" << write << "\n";
    
   // State Transfer
   csm->state.SetState(ESTABLISHED);    

   // TODO Hard code now, delete later

}

void synSentToSynRcvd(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){

}

void synSentToClosed(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){ 
   csm->state.SetState(CLOSED);
}
//-------------modified----------------
void establishedToFinWait1(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){ 
  Packet * out = fin(*csm);//Only send a finish message.
  cerr << "[establishedToFinWait1] packet to be sent out\n";
  printPacket(*out);
  // send a packet
  MinetSend(*mux, *out);

  // Post sent
  postSend(csm, out);
 
  // Sock reply, return the STATUS with same conn, no data, no bytes count, error code
  SockRequestResponse status;
  status.type = STATUS;
  status.connection = csm->connection;
  status.error = EOK;
  MinetSend(*sock, status);
  
  // State Transfer
  csm->state.SetState(FIN_WAIT1);

}
//-------------------modified----------
void finWait1ToFinWait2(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){ 
  //post Recv
  postRecv(csm, p);
  // TODO sock layer is closed, no need to inform the status transfer.
  cerr <<"[finWait1ToFinWait2] "<<"status is changed"<<endl;
  // State Transfer
  csm->state.SetState(FIN_WAIT2);
}

void finWait1ToClosing(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){
}

void finWait1ToTimeWait(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){
}
//-------------modified-----------timeout
void finWait2ToTimeWait(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){
  //post p to csm
  postRecv(csm, p);
  cerr << "[finWait2ToTimeWait] after receving finack connection state lastack "<<csm->state.GetLastAcked() <<endl;
  //sent the data
  Packet * out = ack(*csm, NULL);
  cerr << "[finWait2ToTimeWait] packet to be sent out" <<endl;
  printPacket(*out);
  MinetSend(*mux, *out);

  //Post send
  postSend(csm, out);

  // check the seqNum;
  cerr << "[finWait2ToTimeWait] lastSent is " << csm->state.GetLastSent() << " lastAcked " << csm->state.GetLastAcked() <<endl;
  //inform sock about the closing stateus.
  //SockRequestResponse close;
  //close.type = WRITE;
  //close.bytes = 0;
  //close.connection = csm->connection;
  //close.error = EOK;
  //MinetSend(*sock, close);
  //cerr << "[finWait2ToTimeWait] Tell sock I am ready !" << close <<endl;
  //state transfer
  csm-> state.SetState(TIME_WAIT);
  //set a timer 
  setTimer(csm, time_timeWait);
}

void closingToTimeWait(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){
}
// -------modified--------------
void timeWaitToClosed(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){ 
  // After the 2 timeouts.
  cerr <<"[timeWaitToClosed] Is closed." <<endl;
  csm->state.SetState(CLOSED);
}
//-------modified-----
void establishedToCloseWait(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){
  //post p to csm.
  postRecv(csm, p);
   cerr << "[establishedToCloseWait] after receving finack connection state last ack " << csm->state.GetLastAcked() << "\n";
   // Send the data
   Packet * out = ack(*csm, NULL);
   cerr << "[establishedToCloseWait] packet to be sent out\n";
   printPacket(*out);
   MinetSend(*mux, *out);
   
   // Post Send
   postSend(csm, out);
   
   // check the seqnum
   cerr << "[establishedToCloseWait] lastSent is " << csm->state.GetLastSent() << " lastAcked " << csm->state.GetLastAcked() << "\n"; 
   //inform sock about the closing status.
   SockRequestResponse write;
   write.type = WRITE;
   write.bytes = 0;
   write.connection =  csm->connection;
   write.error = EOK;
   MinetSend(*sock, write);
   cerr << "[establishedToCloseWait] Tell Sock I am ready !! " << write << "\n";
   // State Transfer
   csm->state.SetState(CLOSE_WAIT);    
}
//-------------modified----------------------
void closeWaitToLastAck(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){
  Packet * out = fin(*csm);
  cerr << "[closeWaitToLastAck] packet to be sent out" <<endl;
  printPacket(*out);
  //send a packet.
  MinetSend(*mux, *out);
  
  //Post sent 
  postSend (csm, out);
  //replying sock, return a STATUS.
  SockRequestResponse status;
  status.type = STATUS;
  status.connection = csm->connection;
  status.error = EOK;
  MinetSend(*sock, status);

  //state Transfer
  csm->state.SetState(LAST_ACK); 
}
// -------------modified-----------
void lastAckToClosed(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){
  // post Recv
  postRecv (csm, p);
  //no need to inform sock layer. It`s closed.
  cerr <<"[lastAckToClosed]" <<endl;
  csm->state.SetState(CLOSED);
}

void establishedLoop(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock){ 
  // assume ACK packet arrives here in integrity
  if(p != NULL && req == NULL){
    TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
    Buffer &data = extractPayload(p);    
    unsigned short datalen = data.GetSize();
    unsigned int lastRecvd, lastAcked;
    lastRecvd = csm->state.GetLastRecvd();
    lastAcked = csm->state.GetLastAcked();
    bool accept = postRecv(csm, p);
    
    if(datalen > 0 && accept){
      bool empty = (csm->state.RecvBuffer.GetSize() == 0);
      // load the packet data into recv buffer assume no overflow here, see the implementation of buffer
      csm->state.RecvBuffer.AddBack(data);
      // if the buffer header is not blocked, send it, the head of recv buffer is GBN !!
      if(empty){
        // greedy ! Send all the data in buffer to sock
        SockRequestResponse write(WRITE, csm->connection, csm->state.RecvBuffer, csm->state.RecvBuffer.GetSize(), EOK);
        MinetSend(*sock, write);
	cerr << "[establishedLoop] data to be submited " << write << "\n";
      }
    }

    if(datalen > 0){
      Packet * out = ack(*csm, NULL );
      cerr << "[establishedLoop] packet to be sent out\n";
      printPacket(*out);
      MinetSend(*mux, *out);
      postSend(csm, out);  
    } 

    // ack move window
    csm->state.SendBuffer.Erase(0, csm->state.GetLastAcked() - lastAcked);
    if(csm->state.GetLastAcked() == csm->state.GetLastSent()){
    // stop timer
      stopTimer(csm);
    
    } else{
    // set timer
      setTimer(csm, time_established);
    }
    
    // Now send out the left over data in the buffer, it may be blocked by window size of buffer size
    // TODO improve this so that the data and the previous ACK can be one packet
    unsigned int unacked = csm->state.GetLastSent() - csm->state.GetLastAcked();
    unsigned int room = csm->state.SendBuffer.GetSize() < csm->state.GetN() ? csm->state.SendBuffer.GetSize() - unacked : csm->state.GetN() - unacked; 
    while(room > 0){
      if(csm->state.GetLastAcked() == csm->state.GetLastSent()){
      //set timer
        setTimer(csm, time_established);
      }
      unsigned int bytes = room < TCP_MAXIMUM_SEGMENT_SIZE ? room : TCP_MAXIMUM_SEGMENT_SIZE;
      Buffer & payload = csm->state.SendBuffer.Extract(csm->state.GetLastSent() - csm->state.GetLastAcked(), bytes);
      Packet * out = ack(*csm, &payload);
      cerr <<"[establishedLoop] packet to be sent out\n";
      printPacket(*out);
      MinetSend(*mux, *out);
      postSend(csm, out);
      room -= bytes;
    }

    
  } else if( p == NULL && req != NULL){
    switch (req->type){
      case WRITE:{
	// add to the unlimited send buffer
	csm->state.SendBuffer.AddBack(req->data);
        cerr << "[establishedLoop] I got your WRITE, I know it is in length of " << req->data.GetSize() << "\n";      
  	
	// send back status, informing sock I accepted everything in my buffer
	SockRequestResponse reply;
	reply.type = STATUS;
	reply.connection = csm->connection;
	reply.bytes = req->data.GetSize(); // TODO Confusion here, assume that the tcp will accept everything from sock
	reply.error = EOK;
	MinetSend(*sock, reply);
        cerr << "[establishedLoop] reply to sock I got your data " << reply <<"\n";

	// Now send out the data in the buffer, it may be blocked by window size of buffer size
	unsigned int unacked = csm->state.GetLastSent() - csm->state.GetLastAcked();
	cerr << "[establishedLoop] unack packet number " << unacked <<"\n";
        unsigned int room = csm->state.SendBuffer.GetSize() < csm->state.GetN() ? csm->state.SendBuffer.GetSize() - unacked : csm->state.GetN() - unacked; 
	cerr << "[establishedLoop] buffer size " << csm->state.SendBuffer.GetSize() << " Available room for sending " << room << "\n";
	while(room > 0){
	  if(csm->state.GetLastAcked() == csm->state.GetLastSent()){
          //set timer
	    setTimer(csm, time_established);
          }
          unsigned int bytes = room < TCP_MAXIMUM_SEGMENT_SIZE ? room : TCP_MAXIMUM_SEGMENT_SIZE;
          Buffer & payload = csm->state.SendBuffer.Extract(csm->state.GetLastSent() - csm->state.GetLastAcked(), bytes);
          Packet * out = ack(*csm, &payload);
          cerr <<"[establishedLoop] packet to be sent out\n";
          printPacket(*out);
          MinetSend(*mux, *out);
          postSend(csm, out);
          room -= bytes;
	}
	cerr << "[establishedLoop] buffer size now " << csm->state.SendBuffer.GetSize() << "\n";
	break;
      }
      case STATUS:{
        // how many bytes read last round
	unsigned int bytes = req->bytes;
        csm->state.RecvBuffer.Erase(0, bytes);
        // unblocking here
	if(csm->state.RecvBuffer.GetSize() > 0){
          // greedy ! Send all the data in buffer to sock
          SockRequestResponse write(WRITE, csm->connection, csm->state.RecvBuffer, csm->state.RecvBuffer.GetSize(), EOK);
          MinetSend(*sock, write);
	  cerr << "[establishedLoop] data to be submitted " << write << "\n";
	} else{
	  // No byte in the window this round, wait for next WRITE
	}
	break;
      }
      default:
	break;
    }  
  } else{}

}



