
#ifndef _GFFifoVector_H
#define _GFFifoVector_H

#include "../Atoms/GFLockable.h"
#include "../Molecules/GFVector.h"

/** TGFFifoVector is an extension to TGFVector to
 *  be able to loop its own objects with a fixed vector size.
 *  It uses TGFLockable objects to be thread-safe.
 */
class TGFFifoVector: public TGFVector {
   protected:
      TGFLockable *aReadLock;
      TGFLockable *aWriteLock;
      bool bWaitingForSpace;
      
      int iCurrent;
      int iLastAdded;

      void init();
   public:
      TGFFifoVector();
      TGFFifoVector( unsigned int iPrealloc );
      ~TGFFifoVector();

      /// Checks whether or not there are still objects to pop-off the vector
      bool isEmpty();

      /// Checks whether or not the vector is full.
      /** If this is true, it might be wise to pop() something off of the vector instead of adding something. */
      bool isFull();

      /// Push an object onto the vector.
      /** If the vector is full, it will wait until there has been a pop() */
      void push( TGFFreeable *obj );
      /// Pop an object off that was pushed first onto the vector.
      TGFFreeable *pop();
      /// Retreive the object that is next to Pop off the vector, but only returns it and does not remove it from the vector.
      TGFFreeable *trypop();
};

#endif // _GFFifoVector_H
