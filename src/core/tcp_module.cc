#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#include <iostream>

#include "Minet.h"
#include "tcpstate.h"
#include "tcp_event.h"
#include "tcp_transit.h"
#include "packet_factory.h"
#include "connection_factory.h"



using std::cout;
using std::endl;
using std::cerr;
using std::string;

struct context {
  MinetHandle mux;
  MinetHandle sock;
};

struct message {
  Packet packet;
  SockRequestResponse req;
  bool timeout;
};

const unsigned short HC_PORT = 8500;

ConnectionToStateMapping<TCPState> * associateConnection(const Connection con);
ConnectionToStateMapping<TCPState> * findTimeoutConnection();
ConnectionToStateMapping<TCPState> * addClosedConnection(const Connection con);
void cleanClosedConnection();
void connectionStateTransfer(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout, MinetHandle * mux, MinetHandle * sock);
unsigned int getSeq();

ConnectionList<TCPState> clist;

int main(int argc, char *argv[])
{
  MinetHandle mux, sock;
  MinetInit(MINET_TCP_MODULE);

  mux=MinetIsModuleInConfig(MINET_IP_MUX) ? MinetConnect(MINET_IP_MUX) : MINET_NOHANDLE;
  sock=MinetIsModuleInConfig(MINET_SOCK_MODULE) ? MinetAccept(MINET_SOCK_MODULE) : MINET_NOHANDLE;

  if (MinetIsModuleInConfig(MINET_IP_MUX) && mux==MINET_NOHANDLE) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't connect to mux"));
    return -1;
  }

  if (MinetIsModuleInConfig(MINET_SOCK_MODULE) && sock==MINET_NOHANDLE) {
    MinetSendToMonitor(MinetMonitoringEvent("Can't accept from sock module"));
    return -1;
  }

  MinetSendToMonitor(MinetMonitoringEvent("tcp_module handling TCP traffic"));

  MinetEvent event;
  ConnectionToStateMapping<TCPState> * csm;
  
  cerr << "[main] Start event catching\n";
  while (MinetGetNextEvent(event)==0) {
    if(event.eventtype == MinetEvent::Dataflow && event.direction == MinetEvent::IN){
       if(event.handle == mux){
	  cerr << "[main] Event from mux\n";
          Packet p;
	  MinetReceive(mux, p);
          parseTCP(&p);
          printPacket(p);
          csm = associateConnection(*( packetToConnection(p) ) );
          if(csm != NULL){
             cerr << "[main] match to connection:" << csm->connection.src << " : " << csm->connection.srcport << " --> " << csm->connection.dest << " : " << csm->connection.destport << "\n";
             connectionStateTransfer(csm, &p, NULL, false, &mux, &sock);
          } else{
             cerr << "[main] Serious error No connection match!!!\n";
          }
  	} else if (event.handle == sock){
	  cerr << "[main] Event from sock\n";
          SockRequestResponse req;
	  MinetReceive(sock, req);
          cerr << "[main] Received Socket Request:" << req << endl;
          if(req.type == CONNECT || req.type == ACCEPT){
             addClosedConnection(req.connection); 
          } 
          csm = associateConnection(req.connection);
	  if(csm != NULL){
	    cerr << "[main] match to connection:" << csm->connection.src << " : " << csm->connection.srcport << " --> " << csm->connection.dest << " : " << csm->connection.destport << "\n"; 
            connectionStateTransfer(csm, NULL, &req, false, &mux, &sock);
	  } else {
            cerr << "[main] Serious Error No connection match!!!\n";
	  }
        } else{ }
    } else if(event.eventtype == MinetEvent::Timeout) {
      cerr << "[main] Time out event\n";
      csm = findTimeoutConnection();
      connectionStateTransfer(csm, NULL, NULL, true, &mux, &sock);

    } else{
      cerr << "[main] Unknown event\n";
      MinetSendToMonitor(MinetMonitoringEvent("Unknown event ignored."));
    }
    cleanClosedConnection();
    cerr << "[main] Waiting for event coming ...\n";
    
  }
  return 0;
}




ConnectionToStateMapping<TCPState> * associateConnection(const Connection con){
  ConnectionToStateMapping<TCPState> * listen = NULL;
  for (ConnectionList<TCPState>::iterator i = clist.begin(); i != clist.end(); ++i) {
    if((*i).state.GetState() == LISTEN && (*i).MatchesSource(con)){
      listen = &(*i);
    } else if(i->Matches(con)){
      return &(*i);
    }        
  }
  return listen;
}
//----------------------modified-----------------------
ConnectionToStateMapping<TCPState> * findTimeoutConnection(){
  ConnectionToStateMapping<TCPState> * timeCon = NULL;
  Time currTime;
  for (ConnectionList<TCPState>::iterator i = clist.begin(); i != clist.end(); ++i) {
    if ((*i).bTmrActive && ((*i).timeout < currTime)) {
      timeCon = &(*i);
      cerr << "[timeout] status:" << timeCon-> state <<endl;
      return timeCon;
    }
  }
  return timeCon;
}

ConnectionToStateMapping<TCPState> * addClosedConnection(const Connection con){
   Time t;
   TCPState state(getSeq(), CLOSED, 0);     
   ConnectionToStateMapping<TCPState> * csm = new ConnectionToStateMapping<TCPState>(con, t, state, true);
   clist.push_back(*csm);
   return csm;
}

void cleanClosedConnection(){
  ConnectionList<TCPState>::iterator i;
  for (i = clist.begin(); i != clist.end(); ++i) {
    if((*i).state.GetState() == CLOSED){
      break;
    }
  }
  if(i != clist.end()){
    clist.erase(i);
  }
}

void connectionStateTransfer(ConnectionToStateMapping<TCPState> * csm, Packet * p, SockRequestResponse * req, bool timeout, MinetHandle * mux, MinetHandle * sock){
  cerr << "[state transfer] Now transit state \n";

  switch(csm->state.GetState()){
    case CLOSED:
      cerr << "[state transfer] Closed state\n";
      if(closedToSynSentEnabled(csm, p, req, timeout)){
        cerr << "[state transfer] Closed --> SynSent\n";
        closedToSynSent(csm, p, req, mux, sock);         
      } else if(closedToListenEnabled(csm, p, req, timeout)){
        cerr << "[state transfer] Closed --> Listen\n";
        closedToListen(csm, p, req, mux, sock);
      } 
      break;	
    case LISTEN:
      cerr << "[state transfer] Listen state\n";
      if(listenToSynRcvdEnabled(csm, p, req, timeout)){
        cerr << "[state transfer] Listen --> SynRcvd\n";
        listenToSynRcvd(csm, p, req, mux, sock);
      } else if(listenToSynSentEnabled(csm, p, req, timeout)){
        //TODO
      }
      break;
    case SYN_SENT:
      cerr << "[state transfer] Syn_sent state\n";
      if(synSentToEstablishedEnabled(csm, p, req, timeout)){
	cerr << "[state transfer] Syn_sent --> Established\n";
        synSentToEstablished(csm, p, req, mux, sock);
      } else if(synSentToSynRcvdEnabled(csm, p, req, timeout)){
	//TODO
      } else if(synSentToClosedEnabled(csm, p, req, timeout)){
        synSentToClosed(csm, p, req, mux, sock);
      }
      break;
    case SYN_RCVD:
      cerr << "[state transfer] Syn_rcvd state\n";
      if(synRcvdToEstablishedEnabled(csm, p, req, timeout)){
        cerr << "[state transfer] syn_rcvd --> established\n";
        synRcvdToEstablished(csm, p, req, mux, sock);
        cerr << "[state transfer] Established !\n";
      } else if(synRcvdToClosedEnabled(csm, p, req, timeout)){
        cerr << "[state transfer] syn_rcvd --> Closed\n"; 
        synRcvdToClosed(csm, p, req, mux, sock);
      }
      break;
    case ESTABLISHED:
      cerr << "[state transfer] Established\n";
      if(establishedToCloseWaitEnabled(csm, p, req, timeout)){
         cerr << "[state transfer] established --> close_wait\n";
         establishedToCloseWait(csm, p, req, mux, sock);
      } else if(establishedToFinWait1Enabled(csm, p, req, timeout)){
         cerr << "[state transfer] established --> fin_wait_1\n";
         establishedToFinWait1(csm, p, req, mux, sock);
      } else if(establishedLoopEnabled(csm, p, req, timeout)){
         cerr << "[state transfer] stay in established\n";
         establishedLoop(csm, p, req, mux, sock);
      } 
      break;
    case FIN_WAIT1:
      cerr << "[state transfer] FIN WAIT 1\n";
      if(finWait1ToFinWait2Enabled(csm, p, req, timeout)){
	 finWait1ToFinWait2(csm, p, req, mux, sock);
      } else if(finWait1ToClosingEnabled(csm, p, req, timeout)){
	 finWait1ToClosing(csm, p, req, mux, sock);
      } else if(finWait1ToTimeWaitEnabled(csm, p, req, timeout)){
	 finWait1ToTimeWait(csm, p, req, mux, sock);
      }
      break;
    case FIN_WAIT2:
      cerr << "[state transfer] FIN WAIT 2\n";
      if(finWait2ToTimeWaitEnabled(csm, p, req, timeout)){
	finWait2ToTimeWait(csm, p, req, mux, sock);
      }
      break;
    case CLOSING:
      cerr << "[state transfer] CLOSING\n";
      if(closingToTimeWaitEnabled(csm, p, req, timeout)){
	closingToTimeWait(csm, p, req, mux, sock);
      }
      break;
    case TIME_WAIT:
      cerr << "[state transfer] TIME WAIT\n";
      if(timeWaitToClosedEnabled(csm, p, req, timeout)){
	 timeWaitToClosed(csm, p, req, mux, sock);
      }
      break;
    case CLOSE_WAIT:
      cerr << "[state transfer] CLOSE WAIT\n";
      if(closeWaitToLastAckEnabled(csm, p, req, timeout)){
	 closeWaitToLastAck(csm, p, req, mux, sock);
      }
      break;
    case LAST_ACK:
      cerr << "[state transfer] LAST ACK\n";
      if(lastAckToClosedEnabled(csm, p, req, timeout)){
	 lastAckToClosed(csm, p, req, mux, sock);
      }
      break;
    default:
      cerr << "[state transfer] No match\n";
      break;
  }
}



unsigned int getSeq(){
  return (unsigned int) (1000 + rand() % 1000);
}
