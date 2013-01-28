
#ifndef _JRTHREADEDSERVER_H_
#define _JRTHREADEDSERVER_H_

#include <Groundfloor/Materials/GFThread.h>
#include "JRServerSocket.h"

class TJRThreadedServer: public TGFThread
{
    protected:
        TJRServerSocket *socket;
        bool bSocketAssigned;
        
        virtual void newClientConnection( TJRBaseSocket *aClient ) = 0;

    public:
        TJRThreadedServer();
        TJRThreadedServer( TJRServerSocket *aSocket );
        ~TJRThreadedServer();

        void startListening( int iPort, int iMax );
        void startListening( int iPort );

        TJRServerSocket *getServerSocket() { return socket; };

        void execute();
};

#endif // _JRTHREADEDSERVER_H_
