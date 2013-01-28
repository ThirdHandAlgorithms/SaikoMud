
#include "JRHttpConnection.h"

#include <cstdio>

TJRHttpConnection::TJRHttpConnection( TJRBaseSocket *aSocket ) : TJRThreadedConnection(aSocket) {
//   receivedData.setSize( 1024 );
}

TJRHttpConnection::~TJRHttpConnection() {
}

void TJRHttpConnection::newMessageReceived( const TGFString *sMessage ) {
   bufferlock.lockWhenAvailable( GFLOCK_INFINITEWAIT );

   TGFString *sMessageCopy = new TGFString( sMessage );
   
   receivedData.addChunk( sMessageCopy );
   
   bufferlock.unlock();
}
