
#include "GFBField.h"


TGFBField::TGFBField() : TGFFreeable() {
   name.setValue( "", 0 );
   datatype.set( -1 );
   maxlength.set( -1 );
   flags.set( 0 );
}

TGFBField::TGFBField( char *n, int dt ) : TGFFreeable() {
   name.setValue_ansi( n );
   datatype.set( dt );
   maxlength.set( -1 );
   flags.set( 0 );
}

TGFBField::~TGFBField() {
}

