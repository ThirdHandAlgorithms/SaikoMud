
#include "JRLookupCache.h"

TJRLookupBase::TJRLookupBase() : TGFFreeable() {
   sNonValidHostName.setValue_ansi( C_NONVALIDHOSTNAME );
}

TJRLookupBase::~TJRLookupBase() {
}


TJRLookupObject *TJRLookupBase::lookupHost( const TGFString *sHost ) {
   TJRLookupObject *obj = new TJRLookupObject();

   obj->name.setValue( sHost );
   if ( JRresolveAll( obj ) ) {
      return obj;
   }

   delete obj;
   return NULL;
}

bool TJRLookupBase::lookupHost( TJRLookupObject *obj ) {
   return JRresolveAll( obj );
}

bool TJRLookupBase::equalsNonValidHostResponse( const TJRLookupObject *obj ) {
   TJRLookupObject *nonValid = lookupHost( &sNonValidHostName );
   return obj->firstAddressEqualsOneOf( nonValid, true );
}

TJRLookupObject *TJRLookupBase::getNonValidHostResponseObject() {
    return lookupHost( &sNonValidHostName );
}

//------------------------------------------------------------------------------

TJRLookupCache::TJRLookupCache( bool bInitNonvalidhostAdress ) : TJRLookupBase() {
   list.autoClear = true;

   if ( bInitNonvalidhostAdress ) {
      lookupHost( &sNonValidHostName );
   }
}

TJRLookupCache::~TJRLookupCache() {
}

TJRLookupObject *TJRLookupCache::findObj( const TGFString *sHost ) {
   lock.lockWhenAvailable( GFLOCK_INFINITEWAIT );

   unsigned int c = list.size();
   for ( unsigned int i = 0; i < c; i++ ) {
      TJRLookupObject *obj = static_cast<TJRLookupObject *>( list.elementAt(i) );

      if ( sHost->match( &obj->name ) ) {
         lock.unlock();
         return obj;
      }
   }

   lock.unlock();

   return NULL;
}

TJRLookupObject *TJRLookupCache::lookupHost( const TGFString *sHost ) {
   TJRLookupObject *obj = findObj( sHost );

   if ( obj == NULL ) {
      obj = TJRLookupBase::lookupHost( sHost );
      if ( obj != NULL ) {
         lock.lockWhenAvailable( GFLOCK_INFINITEWAIT );
         list.addElement( obj );
         lock.unlock();
      }
   }

   return obj;
}

bool TJRLookupCache::lookupHost( TJRLookupObject *obj ) {
   TJRLookupObject *objCached = findObj( &obj->name );

   if ( objCached == NULL ) {
      if ( TJRLookupBase::lookupHost( obj ) ) {
         addObject( obj );
      }
   } else {
      obj->setValue( objCached );
      return true;
   }

   return false;
}


void TJRLookupCache::clear( bool bInitNonvalidhostAdress ) {
   list.clear();

   if ( bInitNonvalidhostAdress ) {
      lookupHost( &sNonValidHostName );
   }
}

void TJRLookupCache::addObject( const TJRLookupObject *obj ) {
   TJRLookupObject *objCached = new TJRLookupObject();
   objCached->setValue( obj );

   lock.lockWhenAvailable( GFLOCK_INFINITEWAIT );
   list.addElement( objCached );
   lock.unlock();
}
