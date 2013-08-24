#include "Minet.h"

void closedToListen(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);
  
void closedToSynSent(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void listenToSynRcvd(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void listenToSynSent(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void synRcvdToEstablished(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void synRcvdToListen(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void synRcvdToClosed(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void synRcvdToFinWait1(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void synSentToEstablished(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void synSentToSynRcvd(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void synSentToClosed(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void establishedToFinWait1(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void finWait1ToFinWait2(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void finWait1ToClosing(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void finWait1ToTimeWait(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void finWait2ToTimeWait(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void closingToTimeWait(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void timeWaitToClosed(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void establishedToCloseWait(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void closeWaitToLastAck(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void lastAckToClosed(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);

void establishedLoop(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse *req, MinetHandle * mux, MinetHandle * sock);
