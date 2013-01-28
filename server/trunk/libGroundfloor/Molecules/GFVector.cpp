
#include "GFVector.h"

#include <cstdlib>

#ifdef __BORLANDC__
	#include <mem.h>
#else
	#include <memory.h>
#endif

#ifndef min
   #define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
	#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif


//---------------------------------------------------------------------------


TGFVector::TGFVector() : TGFFreeable() {
   this->iPreAllocateCount = 1;

   this->paVector = NULL;
   this->iAllocated = 0;

   this->iElementCount = 0;

   this->autoClear = true;       // clear and Delete objects in vector at Free
   this->forceInsert = false;    // look for empty spots at insert

   this->iRemovedCount = 0;

   // bug: length of pointer was assumed to be 4
   // in i386 this returns 4 bytes, but in x86_64 it returns 8 bytes
   this->ptrsize = sizeof( void * );
}

TGFVector::TGFVector( unsigned int iPreAlloc ) : TGFFreeable() {
   this->iPreAllocateCount = iPreAlloc;

   this->paVector = NULL;
   this->iAllocated = 0;

   this->iElementCount = 0;

   this->autoClear = true;       // clear and Delete objects in vector at Free
   this->forceInsert = false;    // look for empty spots at insert

   this->iRemovedCount = 0;

   this->ptrsize = sizeof( void * );
}

TGFVector::~TGFVector() {
   if ( this->paVector != NULL ) {
      if ( this->autoClear ) {
         clear();
      }

      GFFree( this->paVector );
   }
}

void TGFVector::clear() {
   for ( unsigned int i = 0; i < this->iElementCount; i++ ) {
      if ( this->paVector[i] != NULL ) {
         TGFFreeable *obj = this->paVector[i];
         this->paVector[i] = NULL;
         delete obj;
      }
   }

   // niet vertex array zelf freeen, scheelt tijd in alloceren

   this->iElementCount = 0;
}

void TGFVector::setElementCount( unsigned int count ) {
   this->iElementCount = count;
}

bool TGFVector::resizeVector( unsigned int size ) {
   // alloceer nieuwe array
   void *pNewArr = GFMalloc( size * ptrsize );

   if ( pNewArr != NULL ) {
      GFnullifyMem( pNewArr, size * ptrsize );

      unsigned int iOldCount = this->iElementCount;
      this->iElementCount = 0;
      void *pOldArr = this->paVector;

      if ( pOldArr != NULL ) {
         // kopieer eventuele eerder toegevoegde elementen
         iOldCount = min( size, iOldCount );
         memcpy( pNewArr, this->paVector, iOldCount * ptrsize );
      }

      // de nieuwe array
      this->paVector       = static_cast<TGFFreeable**>(pNewArr);
      this->iElementCount  = iOldCount;
      this->iAllocated     = size;

      GFFree( pOldArr );

      return true;
   }

   // te weinig geheugen oid
   return false;
}


int TGFVector::addElement( TGFFreeable *pElement ) {
   if ( this->iAllocated <= this->iElementCount ) {
      // automatisch expanden indien array te klein is
      if ( !resizeVector( this->iAllocated + this->iPreAllocateCount ) ) {
         return -1;
      }
   }

   this->paVector[iElementCount] = pElement;   // adres kopieren
   this->iElementCount++;

   return iElementCount - 1;
}


int TGFVector::insertSomewhere( TGFFreeable *pElement ) {
   if ( (this->iRemovedCount > 0) || (this->forceInsert) ) {
      for ( unsigned int i = 0; i < this->iElementCount; i++ ) {
         if ( this->paVector[i] == NULL ) {
            this->paVector[i] = pElement;

            if ( this->iRemovedCount > 0 ) {
               this->iRemovedCount--;
            }

            return i;
         }
      }
   }

   return addElement( pElement );
}


int TGFVector::insertAt( unsigned int iAt, TGFFreeable *pElement ) {
   TGFFreeable *tmpElemB = NULL;
   TGFFreeable *tmpElemA = reinterpret_cast<TGFFreeable *>(-1);    // is om niet NULL te zijn, thats all (negeer idiote compiler msgs)

#ifndef GFVECTOR_NOINDEXCHECK
   if ( iAt < this->iElementCount ) {
#endif

      if ( this->paVector[iAt] == NULL ) {
         this->paVector[iAt] = pElement;
      } else {

         unsigned int i = iAt;
         tmpElemB = pElement;
         while ( (tmpElemA != NULL) && (i < this->iElementCount) ) {
            tmpElemA = this->paVector[i];
            this->paVector[i] = tmpElemB;
            tmpElemB = tmpElemA;
            i++;
         }

         if ( tmpElemB != NULL ) {
            addElement( tmpElemB );
         }
      }

      if ( this->iRemovedCount > 0 ) {
         this->iRemovedCount--;
      }

      return iAt;

#ifndef GFVECTOR_NOINDEXCHECK
   } else {
      // just insert at end
      return addElement( pElement );
   }
#endif
}


TGFFreeable *TGFVector::elementAt( unsigned int i ) const {
#ifndef GFVECTOR_NOINDEXCHECK
   if ( i < this->iElementCount ) {
#endif

      return this->paVector[i];

#ifndef GFVECTOR_NOINDEXCHECK
   }
   return NULL;
#endif
}


// zelf deleten aub indien dit gebruikt wordt! etc.
TGFFreeable *TGFVector::removeElement( unsigned int i ) {
   TGFFreeable *ret = NULL;

#ifndef GFVECTOR_NOINDEXCHECK
   if ( i < this->iElementCount ) {
#endif
      if ( this->paVector[i] != NULL ) {
         ret = this->paVector[i];
         this->paVector[i] = NULL;

         this->iRemovedCount++;

// hier kan niet meer vanuit worden gegaan vanwege de vele manieren
//  van verwijderen en toevoegen van elementen.
/*
         if ( this->iRemovedCount == this->iElementCount ) {
            // reset pointer to if everything's deleted
            this->iElementCount = 0;
         }
*/
      }
#ifndef GFVECTOR_NOINDEXCHECK
   }
#endif

   return ret;
}

// element vervangen op positie i, met object obj - huidige element dat vervangen is wordt teruggegeven
TGFFreeable *TGFVector::replaceElement( unsigned int i, TGFFreeable *pElement ) {
   TGFFreeable *ret = NULL;

#ifndef GFVECTOR_NOINDEXCHECK
   if ( i < this->iElementCount ) {
#endif
      ret = this->paVector[i];

      this->paVector[i] = pElement;
#ifndef GFVECTOR_NOINDEXCHECK
   }
#endif

   return ret;
}

void TGFVector::swapElements( unsigned int i, unsigned int j ) {
   TGFFreeable *a;

   unsigned int c = this->iElementCount;
#ifndef GFVECTOR_NOINDEXCHECK
   if ( ( i < c ) && ( j < c ) ) {
#endif
      a = this->paVector[i];

      this->paVector[i] = this->paVector[j];
      this->paVector[j] = a;
#ifndef GFVECTOR_NOINDEXCHECK
   }
#endif
}

void TGFVector::removeElement( TGFFreeable *pElement ) {
   for ( unsigned int i = 0; i < this->iElementCount; i++ ) {
      if ( this->paVector[i] == pElement ) {
         this->paVector[i] = NULL;

         this->iRemovedCount++;


// hier kan niet meer vanuit worden gegaan vanwege de vele manieren
//  van verwijderen en toevoegen van elementen.
/*
         if ( this->iRemovedCount == this->iElementCount ) {
            // reset pointer to if everything's deleted
            this->iElementCount = 0;
         }
*/
         break;
      }
   }
}

unsigned int TGFVector::size() const {
   return iElementCount;
}

unsigned int TGFVector::allocated() const {
   return iAllocated;
}

int TGFVector::findElement( TGFFreeable *pElement ) const {
   for ( unsigned int i = 0; i < this->iElementCount; i++ ) {
      if ( this->paVector[i] == pElement ) {
         return i;
      }
   }

   return -1;
}

/* Compress()
 *  compress total vector size by removing NULL entries
 */
void TGFVector::compress() {
   unsigned int deleted = 0;

   for ( unsigned int i = 0; i < this->iElementCount; i++ ) {
      if ( this->paVector[i] != NULL ) {
         this->paVector[i-deleted] = this->paVector[i];
      } else {
         deleted++;
      }
   }

   this->iRemovedCount = 0;
   this->iElementCount -= deleted;
   resizeVector( this->iElementCount );
}






// NOT YET PROPERLY TESTED
//  14-05-2005 - pq: Werkt nu!
bool TGFVector::fastCopy( TGFVector *into ) {
   void *pNewArr = GFMalloc( this->iElementCount * ptrsize );

   if ( pNewArr != NULL ) {
      into->iElementCount = 0;
      void *pOldArr = into->paVector;

      // kopieer naar nieuwe vector
      if ( this->paVector != NULL ) {
         memcpy( pNewArr, this->paVector, this->iElementCount * ptrsize );
      }

      into->paVector       = static_cast<TGFFreeable**>(pNewArr);
      into->iElementCount  = this->iElementCount;
      into->iAllocated     = this->iElementCount;

      // free huidige vector
      GFFree( pOldArr );

      return true;
   }

   // te weinig geheugen oid
   return false;
}

