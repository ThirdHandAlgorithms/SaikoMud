
#include "JRThreadedServer.h"
#include "JRBaseSocket.h"
#include <cstdio>

TJRThreadedServer::TJRThreadedServer() : TGFThread() {
    socket = new TJRServerSocket();
    bSocketAssigned = false;
}

TJRThreadedServer::TJRThreadedServer( TJRServerSocket *aSocket ) : TGFThread() {
    bSocketAssigned = true;
    this->socket = aSocket;
}

TJRThreadedServer::~TJRThreadedServer() {
    if ( !bSocketAssigned && (socket != 0) ) {
        delete socket;
    }
}

void TJRThreadedServer::startListening( int iPort, int iMax ) {
   socket->localPort.set( iPort );
   socket->maxconnections.set( iMax );
   
   socket->connect();

   start();
}

void TJRThreadedServer::startListening( int iPort ) {
   startListening( iPort, 2114125311 );
}

void TJRThreadedServer::execute() {
    TJRBaseSocket *client = socket->waitForNewClient();

    if ( client != 0 ) {
        newClientConnection( client );
    }
}

