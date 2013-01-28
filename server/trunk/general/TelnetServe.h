
#ifndef __TELNETSERVE_H__
#define __TELNETSERVE_H__

#include <Jumpropes/JRBaseSocket.h>
#include <Jumpropes/JRThreadedServer.h>
#include <Groundfloor/Molecules/GFVector.h>
#include <Groundfloor/Atoms/GFLockable.h>

#include "TelnetConnection.h"

class CTelnetServe: public TJRThreadedServer {
protected:
   TGFVector pool;
   TGFLockable poollock;

   void newClientConnection( TJRBaseSocket *aClient );
public:
   CTelnetServe();
   ~CTelnetServe();

   void cleanup();

   CTelnetConnection *getClientFromPool( void *ref );
};


#endif // __TELNETSERVE_H__
