
#ifndef __JRTHREADEDSENDER_H
#define __JRTHREADEDSENDER_H

#include <Groundfloor/Materials/GFThreadedBuffer.h>
#include "JRBaseSocket.h"

class TJRThreadedSender: public TGFThreadedBuffer {
   protected:
      TJRBaseSocket *socket;
   public:
      TJRThreadedSender( TJRBaseSocket *aSocket );
      ~TJRThreadedSender();

      void add( TGFFreeable *obj );

      bool processObject( TGFFreeable *obj );
};

#endif // __JRTHREADEDSENDER_H
