#ifndef __GFGarbageCollector_H
#define __GFGarbageCollector_H

#include "../Atoms/GFFreeable.h"
#include "../Molecules/GFVector.h"
#include "../Materials/GFThread.h"

#include "../Molecules/GFString.h"
#include "../Bookshelfs/GFBValue.h"


#define GC_LIVEFOREVER 4294967295UL

/** TGFGarbageStamp class
 */
class TGFGarbageStamp: public TGFFreeable {
   protected:
      __int64 iCreationTime;
      __int64 iDeletionTime;
      TGFFreeable *anObject;

   public:
      TGFGarbageStamp( TGFFreeable *anObject, __int64 iTimeToLive );
      ~TGFGarbageStamp();

      __int64 getCreationTime();
      __int64 getDeletionTime();
};

/** TGFGarbageCollector class.
  * The TGFGarbageCollector is not a typical kind of garbage collection mechanism,
  *  does not depend on usage or references to the object, but on a predefined TTL (time-to-live).
  * Therefor the garbage collector should not be used on a regular basis, but only in situations
  *  where manually deleting an object is either not possible or too complicated.
  */
class TGFGarbageCollector: public TGFThread {
   protected:
      TGFVector *aGarbageCollection;
      unsigned int iDefaultTimeToLive;

      unsigned int iObjectsAdded;
      unsigned int iObjectsDeleted;

   public:
      /// initializes vector, variables and sets the default time-to-live to 5 seconds
      TGFGarbageCollector();
      /// automatically deletes all objects
      ~TGFGarbageCollector();

      /// adds a new TGFGarbageStamp with given object to the vector.
      /// If given iTimeToLive is set to GC_LIVEFOREVER, the object will be kept
      ///  alive untill the garbagecollector is deleted.
      void addToQueue( TGFFreeable *anObject, unsigned int iTimeToLive = 0 );

      /// the default ttl that is used when no custom ttl is given
      void setDefaultTimeToLive( unsigned int iDefaultTimeToLive );

      unsigned int getItemsAdded();
      unsigned int getItemsDeleted();

      /// implementation of thread-loop, disposing of garbage when timers have expired
      void execute();
};


/// instantiates the primary TGFGarbageCollector object
bool initGlobalGarbageCollector( unsigned int iInterval = 5000 );
/// deletes the garbagecollector, freeing all remaining garbage
void finiGlobalGarbageCollector();

/// returns the number of objects added to the garbagecollector
unsigned int GFGarbageStats_itemsAdded();
/// returns the number of objects that have been disposed
unsigned int GFGarbageStats_itemsDeleted();


/// adds the given object to the garbagecollector, return the same object
TGFFreeable *GFDisposable( TGFFreeable *object, unsigned int iTTL = 0 );
/// adds the given TGFString-object to the garbagecollector, return the same object
TGFString *GFDisposable( TGFString *object, unsigned int iTTL = 0 );

/// creates a new TGFString from given str, adds it to the garbagecollector, and returns the object
TGFString *GFDisposableStr( const char *str, unsigned int iTTL = 0 );
/// creates a new TGFBValue from given str, adds it to the garbagecollector, and returns the object
TGFBValue *GFDisposableStrValue( const char *str, unsigned int iTTL = 0 );


#endif // __GFGarbageCollector_H

