
#include "GFBRecords.h"
#include "../Atoms/GFDefines.h"

TGFBRecords::TGFBRecords() : TGFVector() {
   autoClear = true;
}

TGFBRecords::~TGFBRecords() {
}

TGFBRecord *TGFBRecords::getRecord( unsigned int iIndex ) {
   return static_cast<TGFBRecord *>( elementAt( iIndex ) );
}

TGFBRecord *TGFBRecords::newRecord() {
   TGFBRecord *record = new TGFBRecord();
   addElement( record );

   return record;
}

TGFBRecord *TGFBRecords::findRecordByInteger( unsigned int iColumn, int iNeedle ) {
   TGFBRecord *record;

   unsigned int c = size();
   for ( unsigned int i = 0; i < c; i++ ) {
      record = static_cast<TGFBRecord *>( elementAt( i ) );

      if ( record->matchInteger( iColumn, iNeedle ) ) {
         return record;
      }
   }

   return NULL;
}

