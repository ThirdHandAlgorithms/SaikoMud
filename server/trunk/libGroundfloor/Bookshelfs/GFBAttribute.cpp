
#include "GFBAttribute.h"

TGFBAttribute::TGFBAttribute() : TGFFreeable() {
   hideFromXML = false;
}

TGFBAttribute::TGFBAttribute( const TGFBAttribute *attrib ) : TGFFreeable() {
   hideFromXML = attrib->hideFromXML;
   key.setValue( &attrib->key );
   value.setValue( &attrib->value );
}

TGFBAttribute::TGFBAttribute( const char *sKey ) : TGFFreeable() {
   hideFromXML = false;
   key.setValue_ansi( sKey );
}

TGFBAttribute::TGFBAttribute( const char *sKey, const char *sValue ) : TGFFreeable() {
   hideFromXML = false;
   key.setValue_ansi( sKey );
   value.setString( sValue );
}

TGFBAttribute::TGFBAttribute( const char *sKey, const TGFString *sValue ) : TGFFreeable() {
   hideFromXML = false;
   key.setValue_ansi( sKey );
   value.setString( sValue );
}

TGFBAttribute::TGFBAttribute( const char *sKey, bool bValue ) : TGFFreeable() {
   hideFromXML = false;
   key.setValue_ansi( sKey );
   value.setBoolean( bValue );
}

TGFBAttribute::TGFBAttribute( const char *sKey, long iValue ) : TGFFreeable() {
   hideFromXML = false;
   key.setValue_ansi( sKey );
   value.setInteger( iValue );
}

TGFBAttribute::TGFBAttribute( const char *sKey, double iValue, short iDecimal ) : TGFFreeable() {
   hideFromXML = false;
   key.setValue_ansi( sKey );
   value.precision.set( iDecimal );
   value.setDouble( iValue );
}


TGFBAttribute::~TGFBAttribute() {
}
