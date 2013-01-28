
#include "GFBRecord.h"

TGFBRecord::TGFBRecord() : TGFVector() {
   autoClear = true;
}

TGFBRecord::~TGFBRecord() {
}

TGFBValue *TGFBRecord::newValue() {
   TGFBValue *val = new TGFBValue();

   addElement( val );

   return val;
}

TGFBValue *TGFBRecord::getValue( unsigned int iIndex ) {
   return static_cast<TGFBValue *>( elementAt( iIndex ) );
}

void TGFBRecord::setValueCount( unsigned int iCount ) {
   for ( unsigned int i = 0; i < iCount; i++ ) {
      if ( elementAt(i) == 0 ) {
         insertAt( i, new TGFBValue() );
      }
   }

   if ( autoClear ) {
      for ( unsigned int i = iCount; i < size(); i++ ) {
         if ( elementAt(i) == 0 ) {
            delete removeElement(i);
         }
      }
   }
}

bool TGFBRecord::matchInteger( unsigned int iColumn, int iNeedle ) {
   TGFBValue *val = static_cast<TGFBValue *>( elementAt( iColumn ) );

   return ( val->asInteger() == iNeedle);
}

