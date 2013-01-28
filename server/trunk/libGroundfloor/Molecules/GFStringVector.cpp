
#include "GFStringVector.h"
#include "../Atoms/GFDefines.h"

TGFStringVectorRange::TGFStringVectorRange( unsigned int a, unsigned long b, unsigned int c, unsigned long d ) : TGFFreeable() {
   this->start_ind   = a;
   this->start_pos   = b;
   this->end_ind     = c;
   this->end_pos     = d;
}

TGFStringVectorRange::TGFStringVectorRange( TGFStringVectorRange *aRange ) {
   copyValues( aRange );
}

void TGFStringVectorRange::copyValues( TGFStringVectorRange *aRange ) {
   this->start_ind   = aRange->start_ind;
   this->start_pos   = aRange->start_pos;
   this->end_ind     = aRange->end_ind;
   this->end_pos     = aRange->end_pos;
}

TGFStringVectorRange::~TGFStringVectorRange() {
}

//----------------------------------------------------------------------------

TGFStringVector::TGFStringVector() : TGFVector() {

}

TGFStringVector::~TGFStringVector() {
}

void TGFStringVector::addChunk( TGFString *pString ) {
   this->addElement( pString );
}

TGFString *TGFStringVector::getChunk( unsigned int iChunkIndex ) {
   return static_cast<TGFString *>( elementAt(iChunkIndex) );
}

TGFString *TGFStringVector::copy( unsigned int iStartChunk, unsigned long iStartPos, unsigned int iLength ) {
   unsigned int cChunkLen = 0;
   unsigned int iCurChunk = iStartChunk;
   unsigned int cCurrentLen = 0;
   unsigned long iCurPos = iStartPos;
   unsigned int iLenTodo = iLength;

   TGFString *sCopy = new TGFString();
   if ( iLength != static_cast<unsigned int>(-1) ) {
      sCopy->setSize( iLength );
   } else {
      // we can't actually reallocate to unsigned -1 bytes,
      //  but append() will still work so that "everything" is copied into the new string
      sCopy->setSize( 1024 );
   }

   while ( iLenTodo != 0 ) {
      TGFString *sChunk = static_cast<TGFString *>( elementAt(iCurChunk) );
      if ( sChunk != NULL ) {
         cChunkLen = sChunk->getLength();
         if ( iCurPos >= cChunkLen ) {
            iCurPos -= cChunkLen;
            iCurChunk++;
         } else {
            cChunkLen -= iCurPos;

            if ( iLenTodo <= cChunkLen ) {
               sCopy->append( sChunk->getPointer(iCurPos), iLenTodo );
               break;
            } else {
               sCopy->append( sChunk->getPointer(iCurPos), cChunkLen );
               cCurrentLen += cChunkLen;
               iCurChunk++;
               iCurPos = 0;
            }
         }

         iLenTodo = iLength - cCurrentLen;
      } else {
         break;
      }
   }

   return sCopy;
}

TGFString *TGFStringVector::copy( TGFStringVectorRange *aRange ) {
   TGFString *sCopy = new TGFString();

   unsigned int iCurChunk  = aRange->start_ind;
   unsigned long iCurPos   = aRange->start_pos;
   unsigned int cChunkLen  = 0;

   unsigned int iMaxEndChunk = aRange->end_ind;
   unsigned long iMaxEndPos  = aRange->end_pos;
   unsigned int c = this->size();

   while ( iCurChunk < c ) {
      TGFString *sChunk = static_cast<TGFString *>( elementAt(iCurChunk) );
      if ( sChunk != NULL ) {
         cChunkLen = sChunk->getLength();

         if ( iCurPos >= cChunkLen ) {
            iCurPos -= cChunkLen;

            if ( iCurChunk == iMaxEndChunk ) {
               iMaxEndChunk++;

               if ( iMaxEndPos >= cChunkLen ) {
                  iMaxEndPos -= cChunkLen;
               }
            }

            iCurChunk++;
         } else {
            if ( iCurChunk == iMaxEndChunk ) {
               if ( iMaxEndPos >= cChunkLen ) {
                  sCopy->append( sChunk->getPointer(iCurPos), cChunkLen - iCurPos );

                  iMaxEndPos -= cChunkLen;
                  iCurChunk++;
                  iMaxEndChunk++;
               } else {
                  sCopy->append( sChunk->getPointer(iCurPos), iMaxEndPos - iCurPos + 1 );
                  break;
               }
            } else {
               sCopy->append( sChunk->getPointer(iCurPos), cChunkLen - iCurPos );

               iCurChunk++;
            }

            iCurPos = 0;
         }
      } else {
         break;
      }
   }

   return sCopy;
}


bool TGFStringVector::pos( TGFStringVectorRange *aRange, const char *sNeedle, unsigned int iNeedleLen ) {
   unsigned int cMatch = 0;
   unsigned long iReturnPos = aRange->start_pos;
   unsigned int iReturnChunk = aRange->start_ind;
   unsigned int iCurChunk = aRange->start_ind;

   unsigned long iCurPos = aRange->start_pos;
   unsigned int cChunks = this->size();


   // auto fix ranges
   TGFString *sChunk;


   sChunk = static_cast<TGFString *>( elementAt(iCurChunk) );
   if ( sChunk == NULL ) {
      return false;
   }

   while ( iCurPos >= sChunk->getLength() ) {
      iCurChunk++;
      iCurPos -= sChunk->getLength();

      sChunk = static_cast<TGFString *>( elementAt(iCurChunk) );
      if ( sChunk == NULL ) {
         break;
      }
   }

   iReturnChunk = iCurChunk;
   iReturnPos = iCurPos;

   // start search
   while ( iCurChunk < cChunks ) {

      sChunk = static_cast<TGFString *>( elementAt(iCurChunk) );
      if ( sChunk != NULL ) {
         char *val = sChunk->getValue();
         unsigned long cLen = sChunk->getLength();

         while ( iCurPos < cLen ) {
            if ( val[iCurPos] == sNeedle[cMatch] ) {
               if ( cMatch == 0 ) {
                  iReturnChunk   = iCurChunk;
                  iReturnPos     = iCurPos + 1;
                  if ( iReturnPos >= cLen ) {
                     iReturnChunk++;
                     iReturnPos -= cLen;
                  }
               }
               cMatch++;
            } else {
               if ( cMatch != 0 ) {
                  iCurPos     = iReturnPos;
                  iCurChunk   = iReturnChunk - 1; // after the break this gets incremented again
                  cMatch      = 0;

                  break;
               }
            }

            if ( cMatch == iNeedleLen ) {
               aRange->start_ind = iReturnChunk;
               aRange->start_pos = iReturnPos;
               aRange->end_ind   = iCurChunk;
               aRange->end_pos   = iCurPos;

               startMinusOne( aRange );

               return true;
            }

            iCurPos++;
            if ( iCurPos == cLen ) {
               iCurPos = 0;
               break;
            }
         }

         iCurChunk++;
      }

   }

   return false;
}

bool TGFStringVector::pos_ansi( TGFStringVectorRange *aRange, const char *sNeedle ) {
    return pos( aRange, sNeedle, strlen( sNeedle ) );
}

unsigned long TGFStringVector::getLength( unsigned int iStartChunk ) {
   unsigned int iCurChunk  = iStartChunk;

   unsigned long iLength    = 0;
   unsigned int cChunkLen  = 0;

   unsigned int c = this->size();

   while ( iCurChunk < c ) {
      TGFString *sChunk = static_cast<TGFString *>( elementAt(iCurChunk) );
      if ( sChunk != NULL ) {
         cChunkLen = sChunk->getLength();

         iLength += cChunkLen;

         iCurChunk++;
      } else {
         break;
      }
   }

   return iLength;
}

TGFStringVectorRange *TGFStringVector::translatePosToChunkedIndex( unsigned long iPos ) {
   TGFStringVectorRange *range = new TGFStringVectorRange();

   unsigned int cChunkLen = 0;
   unsigned int iCurChunk = 0;
   unsigned long iLenTodo = iPos;

   while ( iLenTodo != 0 ) {
      TGFString *sChunk = static_cast<TGFString *>( elementAt(iCurChunk) );
      if ( sChunk != NULL ) {
         cChunkLen = sChunk->getLength();

         if ( cChunkLen > iLenTodo ) {
            range->start_ind = iCurChunk;
            range->start_pos = iLenTodo;

            break;
         }

         iLenTodo -= cChunkLen;
         iCurChunk++;
      } else {
         break;
      }
   }

   return range;
}

void TGFStringVector::startMinusOne( TGFStringVectorRange *range ) {
   if ( range->start_pos == 0 ) {
      range->start_ind--;

      TGFString *sChunk = static_cast<TGFString *>( elementAt(range->start_ind) );
      range->start_pos = sChunk->getLength() - 1;
   } else {
      range->start_pos--;
   }
}

void TGFStringVector::endMinusOne( TGFStringVectorRange *range ) {
   if ( range->end_pos == 0 ) {
      range->end_ind--;

      TGFString *sChunk = static_cast<TGFString *>( elementAt(range->end_ind) );
      range->end_pos = sChunk->getLength() - 1;
   } else {
      range->end_pos--;
   }
}
