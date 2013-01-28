
#include "GFFifoVector.h"

#include <cstdio>


void TGFFifoVector::init() {
   this->autoClear = false;
   bWaitingForSpace = false;

   aReadLock   = new TGFLockable();
   aWriteLock  = new TGFLockable();
   

   iCurrent    = -1;
   iLastAdded  = -1;
}

TGFFifoVector::TGFFifoVector() : TGFVector() {
   init();
}

TGFFifoVector::TGFFifoVector( unsigned int iPrealloc ) : TGFVector( iPrealloc ) {
   init();
} 

TGFFifoVector::~TGFFifoVector() {
   delete aWriteLock;
   delete aReadLock;
}

bool TGFFifoVector::isEmpty() {
   return ((iCurrent == -1) && (iLastAdded == -1));
}

bool TGFFifoVector::isFull() {
   return ( (iLastAdded + 1) == iCurrent );
}

void TGFFifoVector::push( TGFFreeable *obj ) {
   aWriteLock->lockWhenAvailable( GFLOCK_INFINITEWAIT );
	aReadLock->lockWhenAvailable( GFLOCK_INFINITEWAIT );
   try {
      if ( iLastAdded >= iCurrent ) {

         if ( (iCurrent > 0) && (iLastAdded == this->allocated() ) ) {
            this->replaceElement( 0, obj );
            iLastAdded = 0;
         } else {
            this->insertAt( iLastAdded + 1, obj );
            iLastAdded++;
         }
   
      } else { // iLastAdded < iCurrent
         while ( (iLastAdded + 1) == iCurrent ) {
				aReadLock->unlock();
				GFMillisleep(10);
				aReadLock->lockWhenAvailable( GFLOCK_INFINITEWAIT );
         }
         
         this->replaceElement( iLastAdded + 1, obj );
         iLastAdded++;
      }
   } catch ( ... ) {
   }
	aReadLock->unlock();
   aWriteLock->unlock();
}

TGFFreeable *TGFFifoVector::pop() {
   TGFFreeable *obj = 0;

   aReadLock->lockWhenAvailable( GFLOCK_INFINITEWAIT );
   try {
      if ( iLastAdded != -1 ) {
         obj = this->elementAt( iCurrent + 1 );
         this->replaceElement( iCurrent + 1, 0 );

         iCurrent++;
         
         if ( iCurrent == this->size() ) {
            iCurrent = -1;
         }

         if ( iCurrent == iLastAdded ) {
            iCurrent    = -1;
            iLastAdded  = -1;
         }
      }
   } catch ( ... ) {
   }
   aReadLock->unlock();
   
   return obj;
}


TGFFreeable *TGFFifoVector::trypop() {
   TGFFreeable *obj = NULL;
   
   aReadLock->lockWhenAvailable( GFLOCK_INFINITEWAIT );
   try {
      if ( iLastAdded != -1 ) {
         obj = this->elementAt( iCurrent + 1 );
      }
   } catch ( ... ) {
   }
   aReadLock->unlock();

   return obj;
}
