#include "Minet.h"
#include <netinet/in.h>
#include "connection_factory.h"

Connection * packetToConnection(Packet p){
  Connection * c = new Connection();
  IPHeader iph=p.FindHeader(Headers::IPHeader);
  TCPHeader tcph=p.FindHeader(Headers::TCPHeader);
  iph.GetDestIP(c->src);
  iph.GetSourceIP(c->dest);
  iph.GetProtocol(c->protocol);
  tcph.GetDestPort(c->srcport);
  tcph.GetSourcePort(c->destport);
  return c;
}

Connection * invert(Connection con){
  Connection * rcon = new Connection(con.dest, con.src, con.destport, con.srcport, IP_PROTO_TCP);
  return rcon;
}
Connection * listenConnection(unsigned short port){
  Connection * c = new Connection(INADDR_ANY, IP_ADDRESS_ANY, port, PORT_ANY, IP_PROTO_TCP);
  return c;
}
