
#ifndef __JRLookupCache_H
#define __JRLookupCache_H

class TJRLookupBase;
class TJRLookupCache;

#include "JRDefines.h"
#include "JRFunctions.h"
#include "JRNetAddress.h"
#include <Groundfloor/Atoms/GFLockable.h>

class TJRLookupBase: public TGFFreeable {
   protected:
      TGFString sNonValidHostName;
   public:
      TJRLookupBase();
      ~TJRLookupBase();

      virtual TJRLookupObject *lookupHost( const TGFString *sHost );
      virtual bool lookupHost( TJRLookupObject *obj );

      virtual bool equalsNonValidHostResponse( const TJRLookupObject *obj );
      virtual TJRLookupObject *getNonValidHostResponseObject();
};

class TJRLookupCache: public TJRLookupBase {
   protected:
      TGFLockable lock;
      TGFVector list;
      TJRLookupObject *findObj( const TGFString *sHost );
   public:
      TJRLookupCache( bool bInitNonvalidhostAdress = true );
      ~TJRLookupCache();

      void clear( bool bInitNonvalidhostAdress = true );

      TJRLookupObject *lookupHost( const TGFString *sHost );
      bool lookupHost( TJRLookupObject *obj );

      void addObject( const TJRLookupObject *obj );
};


#endif // __JRLookupCache_H
