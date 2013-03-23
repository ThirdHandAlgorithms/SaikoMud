
#include "TelnetServe.h"

#include "TelnetConnection.h"
#include <Groundfloor/Materials/GFGarbageCollector.h>

CTelnetServe::CTelnetServe() : TJRThreadedServer() {
   this->startListening(23123);
}

void CTelnetServe::newClientConnection( TJRBaseSocket *aClient ) {
   CTelnetConnection *c = new CTelnetConnection( aClient );
   pool.addElement( c );
}

CTelnetServe::~CTelnetServe() {
}

void CTelnetServe::cleanup() {
   if ( poollock.lockWhenAvailable() ) {
      unsigned int c = pool.size();
      for ( unsigned int i = 0; i < c; i++ ) {
         CTelnetConnection *conn = static_cast<CTelnetConnection *>( pool.elementAt(i) );
         if ( conn != NULL ) {
            if ( !conn->isRunning() ) {
               pool.removeElement(conn);

               // delete after 10 seconds (some object might still be having this connection around)
               GFDisposable(conn,10);
            };
         }
      }

      poollock.unlock();
   }
}

CTelnetConnection *CTelnetServe::getClientFromPool( void *ref ) {
   if ( poollock.lockWhenAvailable() ) {
      unsigned int c = pool.size();
      for ( unsigned int i = 0; i < c; i++ ) {
         CTelnetConnection *conn = static_cast<CTelnetConnection *>( pool.elementAt(i) );
         if ( conn != NULL ) {
            if ( conn->matchWithCharacterRef(ref) ) {
               if ( conn->isRunning() ) {
                  poollock.unlock();
                  return conn;
               }
            }
         }
      }

      poollock.unlock();
   }

   return NULL;
}
