
#include "GFBFields.h"
#include "GFBField.h"

TGFBFields::TGFBFields() : TGFVector() {
   autoClear = true;
}

TGFBFields::~TGFBFields() {
}

int TGFBFields::getFieldIndex( const char *sName, unsigned int len ) {
   unsigned int c = size();
   for ( unsigned int i = 0; i < c; i++ ) {
      TGFBField *obj = static_cast<TGFBField *>( elementAt( i ) );

      if ( obj != 0 ) {
         if ( obj->name.match( sName, len ) ) {
            return i;
         }
      }
   }

   return -1;
}

int TGFBFields::getFieldIndex_ansi( const char *sName ) {
    return getFieldIndex( sName, strlen(sName) );
}

int TGFBFields::getFieldIndex( const TGFString *sName ) {
   unsigned int c = size();
   for ( unsigned int i = 0; i < c; i++ ) {
      TGFBField *obj = static_cast<TGFBField *>( elementAt( i ) );

      if ( obj != 0 ) {
         if ( obj->name.match( sName ) ) {
            return i;
         }
      }
   }

   return -1;
}

TGFBField *TGFBFields::getField( unsigned int iIndex ) {
   return static_cast<TGFBField *>( elementAt(iIndex) );
}

TGFBField *TGFBFields::newField() {
   TGFBField *field = new TGFBField();
   addElement( field );
   return field;
}

TGFBField *TGFBFields::newField( char *n, int dt ) {
   TGFBField *field = new TGFBField( n, dt );
   addElement( field );
   return field;
}
