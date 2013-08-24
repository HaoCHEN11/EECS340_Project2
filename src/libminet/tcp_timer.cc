//#include "constate.h"
#include "Minet.h"
#include "tcpstate.h"

#include <sys/time.h>
#include <iostream>
#include <typeinfo>

//method to set timer, stop timer;

void setTimer (ConnectionToStateMapping<TCPState> * timeoutCon, double lenOfTimeout) { 
    Time currTimeSec; 
    double targetTime = (double)(currTimeSec) + lenOfTimeout;
    Time newTimer(targetTime);
    timeoutCon->timeout = newTimer; //set time;
    timeoutCon->bTmrActive = true;  // set timer be active;
}

void stopTimer (ConnectionToStateMapping<TCPState> * Con) {Con->bTmrActive = false;} 
