#include "Minet.h"
#include "tcpstate.h"

bool closedToListenEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);
  
bool closedToSynSentEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool listenToSynRcvdEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool listenToSynSentEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool synRcvdToEstablishedEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool synRcvdToListenEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool synRcvdToClosedEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool synRcvdToFinWait1Enabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool synSentToEstablishedEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool synSentToSynRcvdEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool synSentToClosedEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool establishedToFinWait1Enabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool finWait1ToFinWait2Enabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool finWait1ToClosingEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool finWait1ToTimeWaitEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool finWait2ToTimeWaitEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool closingToTimeWaitEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool timeWaitToClosedEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool establishedToCloseWaitEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool closeWaitToLastAckEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool lastAckToClosedEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);

bool establishedLoopEnabled(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout);
