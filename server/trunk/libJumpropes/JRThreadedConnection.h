
#ifndef _JRTHREADEDCONNECTION_H_
#define _JRTHREADEDCONNECTION_H_

#include <Groundfloor/Materials/GFThread.h>
#include "JRBaseSocket.h"

class TJRThreadedConnection: public TGFThread
{
    protected:
        TJRBaseSocket *socket;
        TGFString *sMessage;
    public:
        TJRThreadedConnection( TJRBaseSocket *aSocket );
        ~TJRThreadedConnection();

        TJRBaseSocket *setSocket( TJRBaseSocket *aSocket );
        TJRBaseSocket *getSocket();

        void execute();

        virtual void newMessageReceived( const TGFString *sMessage ) = 0;
};

#endif // _JRTHREADEDCONNECTION_H_
