
#include "JRThreadedSender.h"
#include <Groundfloor/Atoms/GFDefines.h>

TJRThreadedSender::TJRThreadedSender( TJRBaseSocket *aSocket ) : TGFThreadedBuffer() {
   socket = aSocket;

   aBuffer->resizeVector( 100 );   // niet te laag instellen
   this->setInterval( 100 );
}

TJRThreadedSender::~TJRThreadedSender() {
}


void TJRThreadedSender::add( TGFFreeable *obj ) {
   TGFString *sDataCopy = new TGFString( static_cast<TGFString *>(obj) );
   
   TGFThreadedBuffer::add( sDataCopy );
}

bool TJRThreadedSender::processObject( TGFFreeable *obj ) {
   TGFString *str = static_cast<TGFString *>(obj);

   if ( socket->send( str ) ) {
      delete str;
      
      return true;
   }

   return false;
}

