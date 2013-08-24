#include "Minet.h"
#include "tcp_event.h"


bool closedToListenEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(req == NULL)  
    return false;
  return req->type == ACCEPT;
}

bool closedToSynSentEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(req == NULL)
    return false;
  return req->type == CONNECT;
}

bool listenToSynRcvdEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(p == NULL)
    return false;
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned char flag;
  tcph.GetFlags(flag);
  return IS_SYN(flag);
}

bool listenToSynSentEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(req == NULL)
    return false;
  return req->type == CONNECT;  
}

bool synRcvdToEstablishedEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(p == NULL)
    return false;
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned char flag;
  tcph.GetFlags(flag);
  return IS_ACK(flag);
}

bool synRcvdToListenEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(p == NULL)
    return false;
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned char flag;
  tcph.GetFlags(flag);
  return IS_RST(flag); 
}

bool synRcvdToClosedEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  return timeout;
}

bool synRcvdToFinWait1Enabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(req == NULL)
    return false;
  return req->type == CLOSE;
}


bool synSentToEstablishedEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(p == NULL)
    return false;
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned char flag;
  tcph.GetFlags(flag); 
  return IS_SYN(flag) && IS_ACK(flag);
}

bool synSentToSynRcvdEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(p == NULL)
    return false;
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned char flag;
  tcph.GetFlags(flag); 
  return IS_SYN(flag) && !IS_ACK(flag);

}

bool synSentToClosedEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(req != NULL && req->type == CLOSE)
    return true;
  return timeout;
}

bool establishedToFinWait1Enabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(req == NULL)
    return false;
  return req->type == CLOSE;
}

bool finWait1ToFinWait2Enabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(p == NULL)
    return false;
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned char flag;
  tcph.GetFlags(flag);
  return IS_ACK(flag) && !IS_FIN(flag);
}

bool finWait1ToClosingEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(p == NULL)
    return false;
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned char flag;
  tcph.GetFlags(flag); 
  return IS_FIN(flag) && !IS_ACK(flag);
}

bool finWait1ToTimeWaitEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(p == NULL)
    return false;
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned char flag;
  tcph.GetFlags(flag);
  return IS_FIN(flag) && IS_ACK(flag);
}

bool finWait2ToTimeWaitEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(p == NULL)
    return false;
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned char flag;
  tcph.GetFlags(flag);
  return IS_FIN(flag);
}

bool closingToTimeWaitEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(p == NULL)
    return false;
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned char flag;
  tcph.GetFlags(flag); 
  return IS_ACK(flag);
}
//----modified---
bool timeWaitToClosedEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  return timeout;
}

bool establishedToCloseWaitEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(p == NULL)
    return false;
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned char flag;
  tcph.GetFlags(flag); 
  return IS_FIN(flag);
}

bool closeWaitToLastAckEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(req == NULL)
    return false;
  return req->type == CLOSE;
}

bool lastAckToClosedEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(p == NULL)
    return false;
  TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
  unsigned char flag;
  tcph.GetFlags(flag); 
  return IS_ACK(flag);  
}

bool establishedLoopEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout){
  if(p == NULL && req != NULL){
    return req->type == WRITE || req->type == STATUS;
  } else if( p != NULL && req == NULL){
    TCPHeader tcph = p->FindHeader(Headers::TCPHeader);
    unsigned char flag;
    tcph.GetFlags(flag);
    return IS_ACK(flag) && !IS_FIN(flag) && !IS_SYN(flag);
  } else {
    return false;
  } 
}
