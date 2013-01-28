
#ifndef __JRHttpConnection_H
#define __JRHttpConnection_H

#include <Groundfloor/Atoms/GFLockable.h>
#include <Groundfloor/Molecules/GFStringVector.h>
#include "../JRThreadedConnection.h"

class TJRHttpConnection: public TJRThreadedConnection {
   protected:
   public:
      TJRHttpConnection( TJRBaseSocket *aSocket );
      ~TJRHttpConnection();

      TGFLockable bufferlock;
      TGFStringVector receivedData;

      void newMessageReceived( const TGFString *sMessage );
};


#endif // __JRHttpConnection_H
