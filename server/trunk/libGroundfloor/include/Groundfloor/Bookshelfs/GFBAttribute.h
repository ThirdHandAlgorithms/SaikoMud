
#ifndef __GFBAttribute_H
#define __GFBAttribute_H

class TGFBAttribute;

#include "../Atoms/GFFreeable.h"
#include "../Molecules/GFString.h"
#include "GFBValue.h"

/// attribute as used in TGFBNode, following a key=value format.
class TGFBAttribute: public TGFFreeable {
   public:
      TGFBAttribute();
      TGFBAttribute( const TGFBAttribute *attrib );
      TGFBAttribute( const char *sKey );
      TGFBAttribute( const char *sKey, const char *sValue );
      TGFBAttribute( const char *sKey, const TGFString *sValue );
      TGFBAttribute( const char *sKey, bool bValue );
      TGFBAttribute( const char *sKey, long iValue );
      TGFBAttribute( const char *sKey, double iValue, short iDecimal );
      ~TGFBAttribute();

      TGFString key;
      TGFBValue value;
      bool hideFromXML;
};

#endif // __GFBAttribute_H
