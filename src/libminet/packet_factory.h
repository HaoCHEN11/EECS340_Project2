#include "Minet.h"
#include "tcpstate.h"
#include "tcpstate.h"
#include "tcpstate.h"

void printPacket(const Packet p);

void parseTCP(Packet * p);

Buffer & extractPayload(Packet * p);

Packet * syn(ConnectionToStateMapping<TCPState> csm);

Packet * synack(ConnectionToStateMapping<TCPState> csm);

Packet * ack(ConnectionToStateMapping<TCPState> csm, Buffer * data);

Packet * fin(ConnectionToStateMapping<TCPState> csm);

Packet * finack(ConnectionToStateMapping<TCPState> csm);

Packet * rst(ConnectionToStateMapping<TCPState> csm);


