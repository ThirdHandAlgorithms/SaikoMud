
#include "JRThreadedConnection.h"
#include <cstdio>

TJRThreadedConnection::TJRThreadedConnection( TJRBaseSocket *aSocket ) : TGFThread() {
    this->socket = aSocket;

    sMessage = new TGFString( "", 0 );
    sMessage->setSize( 1024 );

    this->setInterval( 50 );
}

TJRThreadedConnection::~TJRThreadedConnection() {
   stopAndWait();
   delete sMessage;

// freeing the socket is not this class's responsibility ????
   if ( socket != 0 ) {
      delete socket;
   }
}

TJRBaseSocket *TJRThreadedConnection::setSocket( TJRBaseSocket *aSocket ) {
   stopAndWait();

   TJRBaseSocket *oldSock = this->socket;
   this->socket = aSocket;

   return oldSock;
}

void TJRThreadedConnection::execute() {
    if ( socket->receive( sMessage ) ) {
        if ( sMessage->getLength() > 0 ) {
            newMessageReceived( sMessage );
        }
    } else {
        stop();
    }
}

TJRBaseSocket *TJRThreadedConnection::getSocket() {
    return socket;
};
