#include "Minet.h"

Connection * packetToConnection(const Packet p);
Connection * invert(Connection con);
Connection * listenConnection(unsigned short port);
