
#include <cstdio>
#include "GFGarbageCollector.h"

#include "../Materials/GFFunctions.h"
#include <cstring>

// global var
static TGFGarbageCollector *GFGlobalVar_GFGarbageCollector = 0;

TGFGarbageStamp::TGFGarbageStamp( TGFFreeable *anObject, __int64 iTimeToLive ) {
   this->anObject       = anObject;
   this->iCreationTime  = GFGetTimestamp();
   if ( iTimeToLive == GC_LIVEFOREVER ) {
      this->iDeletionTime  = GC_LIVEFOREVER;
   } else {
      this->iDeletionTime  = this->iCreationTime + iTimeToLive;
   }
}

TGFGarbageStamp::~TGFGarbageStamp() {
   if ( this->anObject != 0 ) {
      delete anObject;
   }
}

__int64 TGFGarbageStamp::getCreationTime() {
   return this->iCreationTime;
}

__int64 TGFGarbageStamp::getDeletionTime() {
   return this->iDeletionTime;
}

// ----------------------------------------------------------------------------

TGFGarbageCollector::TGFGarbageCollector() {
   iObjectsAdded     = 0;
   iObjectsDeleted   = 0;

   aGarbageCollection = new TGFVector( 5 );
   aGarbageCollection->autoClear = true;

   this->iDefaultTimeToLive = 60;   // a minute
   this->setInterval( 5000 );       // 5 second interval, we don't wanna be too much trouble
}

TGFGarbageCollector::~TGFGarbageCollector() {
   stopAndWait();
   delete aGarbageCollection;
}

void TGFGarbageCollector::setDefaultTimeToLive( unsigned int iDefaultTimeToLive ) {
   this->iDefaultTimeToLive = iDefaultTimeToLive;
}

void TGFGarbageCollector::addToQueue( TGFFreeable *anObject, unsigned int iTimeToLive ) {
   TGFGarbageStamp *stampedObject = 0;

   if ( iTimeToLive != 0 ) {
      stampedObject = new TGFGarbageStamp( anObject, iTimeToLive );
   } else {
      stampedObject = new TGFGarbageStamp( anObject, iDefaultTimeToLive );
   }

   aGarbageCollection->addElement( stampedObject );

   iObjectsAdded++;   
}

void TGFGarbageCollector::execute() {
   unsigned int iDeleted = 0;

   unsigned int c = aGarbageCollection->size();

   for ( unsigned int i = 0; i < c; i++ ) {
      TGFGarbageStamp *aStamp = static_cast<TGFGarbageStamp *>( aGarbageCollection->elementAt( i ) );

      if ( aStamp != 0 ) {
         if ( aStamp->getDeletionTime() != GC_LIVEFOREVER ) {
            if ( GFGetTimestamp() >= aStamp->getDeletionTime() ) {
               delete aGarbageCollection->removeElement( i );

               iDeleted++;
            }
         }
      }

      GFMillisleep( 10 );
   }

   if ( iDeleted > 0 ) {
      aGarbageCollection->compress();
   }

   iObjectsDeleted += iDeleted;
}

unsigned int TGFGarbageCollector::getItemsAdded() {
   return iObjectsAdded;
}

unsigned int TGFGarbageCollector::getItemsDeleted() {
   return iObjectsDeleted;
}


// ----------------------------------------------------------------------------

void GFGarbage_InitialisationError() {
   printf( "Warning: Garbage Collector not yet initialized!\n" );
}

bool initGlobalGarbageCollector( unsigned int iInterval ) {
   if ( GFGlobalVar_GFGarbageCollector != 0 ) {
      finiGlobalGarbageCollector();
   }

   GFGlobalVar_GFGarbageCollector = new TGFGarbageCollector();

   GFGlobalVar_GFGarbageCollector->setDefaultTimeToLive( 60 );
   GFGlobalVar_GFGarbageCollector->setInterval( iInterval );

   GFGlobalVar_GFGarbageCollector->start();
   
   return true;
}

void finiGlobalGarbageCollector() {
   if ( GFGlobalVar_GFGarbageCollector != 0 ) {
      delete GFGlobalVar_GFGarbageCollector;
   } else {
      GFGarbage_InitialisationError();
   }
}

TGFFreeable *GFDisposable( TGFFreeable *object, unsigned int iTTL ) {
   if ( GFGlobalVar_GFGarbageCollector != 0 ) {
      GFGlobalVar_GFGarbageCollector->addToQueue( object, iTTL );
   } else {
      GFGarbage_InitialisationError();
   }

   return object;
}

TGFString *GFDisposable( TGFString *object, unsigned int iTTL ) {
   if ( GFGlobalVar_GFGarbageCollector != 0 ) {
      GFGlobalVar_GFGarbageCollector->addToQueue( object, iTTL );
   } else {
      GFGarbage_InitialisationError();
   }

   return object;
}

TGFString *GFDisposableStr( const char *str, unsigned int iTTL ) {
   TGFString *object = new TGFString( str, strlen(str) );
   
   if ( GFGlobalVar_GFGarbageCollector != 0 ) {
      GFGlobalVar_GFGarbageCollector->addToQueue( object, iTTL );
   } else {
      GFGarbage_InitialisationError();
   }

   return object;
}

TGFBValue *GFDisposableStrValue( const char *str, unsigned int iTTL ) {
   TGFBValue *object = new TGFBValue();
   object->setString( str );

   if ( GFGlobalVar_GFGarbageCollector != 0 ) {
      GFGlobalVar_GFGarbageCollector->addToQueue( object, iTTL );
   } else {
      GFGarbage_InitialisationError();
   }

   return object;
}

unsigned int GFGarbageStats_itemsAdded() {
   if ( GFGlobalVar_GFGarbageCollector != 0 ) {
      return GFGlobalVar_GFGarbageCollector->getItemsAdded();
   } else {
      GFGarbage_InitialisationError();
   }

   return 0;
}

unsigned int GFGarbageStats_itemsDeleted() {
   if ( GFGlobalVar_GFGarbageCollector != 0 ) {
      return GFGlobalVar_GFGarbageCollector->getItemsDeleted();
   } else {
      GFGarbage_InitialisationError();
   }

   return 0;
}

