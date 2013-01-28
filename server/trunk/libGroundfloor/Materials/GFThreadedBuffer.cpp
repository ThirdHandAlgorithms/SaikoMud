
#include "GFThreadedBuffer.h"

TGFThreadedBuffer::TGFThreadedBuffer() : TGFThread() {
   aBuffer = new TGFFifoVector( 1 );

   bDeleteAfterProcess = false;
   bStopWhenEmpty = false;
}

TGFThreadedBuffer::~TGFThreadedBuffer() {
   stop();

   while ( bRunning ) {
      GFMillisleep( 10 );
   }

   delete aBuffer;
}

void TGFThreadedBuffer::setStopWhenEmpty( bool bSet ) {
    bStopWhenEmpty = bSet;
}

void TGFThreadedBuffer::add( TGFFreeable *obj ) {
   lockBuffer.lockWhenAvailable();
   aBuffer->push( obj );
   lockBuffer.unlock();
}

void TGFThreadedBuffer::setDeleteAfterProcess( bool bDelete ) {
   bDeleteAfterProcess = bDelete;
}

void TGFThreadedBuffer::execute() {
   lockBuffer.lockWhenAvailable();
   try {
      if ( !aBuffer->isEmpty() ) {
         // in order for this to work properly, no other object should access aBuffer!
         TGFFreeable *obj = aBuffer->trypop();

         // if your self-implemented function returns false, the object is not removed from the vector
         //  and will be processed Again next time the execute() function is called.
         // yet: if you don't want your program to deadlock at the add() function, it is recommended
         //  to occasionally pop something off of the vector.
         if ( processObject( obj ) ) {
            aBuffer->pop();

            if ( bDeleteAfterProcess ) {
               delete obj;
            }
         }
      }

      if ( bStopWhenEmpty && aBuffer->isEmpty() ) {
          stop();
      }

   } catch ( ... ) {
   }
   lockBuffer.unlock();
}

bool TGFThreadedBuffer::isEmpty() {
   return aBuffer->isEmpty();
}

bool TGFThreadedBuffer::isFull() {
   return aBuffer->isFull();
}

