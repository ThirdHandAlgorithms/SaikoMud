
#include "GFCallbackThread.h"


TGFCallbackThread::TGFCallbackThread( TGFCallable *anObject, int iSleeptime ) : TGFThread() {
    this->setInterval( iSleeptime );
    this->aCallableObject   = anObject;
}

TGFCallbackThread::~TGFCallbackThread() {
}

void TGFCallbackThread::execute() {
    aCallableObject->execute();
}


TGFTimer::TGFTimer() {
   setInterval( 1000 );
}

TGFTimer::~TGFTimer() {
}

void TGFTimer::execute() {
   onTimerEvent.execute( this );
}
