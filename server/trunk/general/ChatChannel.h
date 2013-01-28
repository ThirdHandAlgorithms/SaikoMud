
#ifndef __CHATCHANNEL_H__
#define __CHATCHANNEL_H__

#include <Jumpropes/JRBaseSocket.h>
#include <Jumpropes/JRThreadedServer.h>

#include <Groundfloor/Molecules/GFString.h>
#include <Groundfloor/Materials/GFFifoVector.h>

#include "TelnetConnection.h"

class CChatChannel: public TGFThread {
protected:
   TGFVector clients;
   TGFFifoVector msgqueue;

   DWORD32 channelnr;

public:
   CChatChannel();
   ~CChatChannel();

   void addClient( CTelnetConnection *pClient );
   void delClient( CTelnetConnection *pClient );

   void messageToAll( CTelnetConnection *pFrom, const TGFString *sMsg );
   void messageToAll_ansi( CTelnetConnection *pFrom, const char *sMsg );

   void execute();
};

bool initGlobalChatChannel();
void finiGlobalChatChannel();

void enterGlobalChatChannel(CTelnetConnection *pClient );
void leaveGlobalChatChannel(CTelnetConnection *pClient );
void sendToGlobalChatChannel(CTelnetConnection *pFrom, const char *sMsg);
void sendToGlobalChatChannel(CTelnetConnection *pFrom, const TGFString *sMsg);

#endif // __CHATCHANNEL_H__
