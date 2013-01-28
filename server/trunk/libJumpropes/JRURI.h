
#ifndef __JRURI_H
#define __JRURI_H

#include <Groundfloor/Atoms/GFDefines.h>

#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Molecules/GFString.h>
#include <Groundfloor/Molecules/GFProperty.h>

#include "JRNetAddress.h"
#include "JRLookupCache.h"

class TJRURI: public TGFFreeable {
   protected:
      TJRLookupBase *pLookupBase;
      void useDefaultPort();
   public:
      TJRURI( TJRLookupBase *pLookupBase );
      ~TJRURI();

      void parseUri( const TGFString *sUri, bool bClearVars = true );
      void parseUri( const char *sUri, bool bClearVars = true );
      TGFString *getUri();

      TGFString protocol;
      TJRLookupObject host;
      TGFProperty<int> port;
      TGFString path;
      TGFString query;

      TGFString user;
      TGFString pass;
};


#endif // __JRURI_H
