
#ifndef __TELNETCONNECTION_H__
#define __TELNETCONNECTION_H__

#include <Jumpropes/JRThreadedConnection.h>
#include <Groundfloor/Molecules/GFString.h>

#include "../general/GameInterface.h"

#define c_run_walkforward 0x00000005
#define c_run_walkbackwards 0x00000006
#define c_run_walkleft 0x00000007
#define c_run_walkright 0x00000008

#define c_attack_start 0x10000009
#define c_attack_stop 0x0000000a

#define c_chat_say 0x30000010

#define c_interact_greet 0x20000020
#define c_interact_getquesttitles 0x20000021
#define c_interact_getquesttext 0x20000022


#define c_response_lastactioninfo 0x10010000
#define c_response_roominfo 0x30020000
#define c_response_asciimap 0x10030000

#define c_response_npcnames 0x30110000
#define c_response_dialog 0x20120000

#define c_response_questtitle 0x30130000
#define c_response_questtext 0x30140000

//#define c_response_completedquest 0x3015000


/*
#define c_response_ 0x10100000
#define c_response_ 0x10200000
#define c_response_ 0x10300000
#define c_response_ 0x10400000
*/

#define c_response_chatmessage 0x30100000


class CTelnetConnection: public TJRThreadedConnection {
protected:
   bool bBinaryMode;

   TGFString buffer;

   CGameInterface gameintf;

   void inform_currentroom();
   void inform_lastaction();
   void inform_map();
   void inform_questtitle(DWORD32 iQuestId, TGFString *s);
   void inform_questtext(DWORD32 iQuestId, TGFString *s);

   //void inform_currentplayerstats();

   bool decodeNextBinMessageInBuffer(DWORD32 *command, DWORD32 *intparam1, DWORD32 *intparam2, TGFString *s);
public:
   CTelnetConnection( TJRBaseSocket *aSocket );
   ~CTelnetConnection();

   bool inBinaryMode();

   void newMessageReceived( const TGFString *sMessage );

   void send( const TGFString *s );
   void sendBin(DWORD32 command, DWORD32 intparam1, DWORD32 intparam2, TGFString *s);

   bool matchWithCharacterRef( void *ref );

   void append_nickname(TGFString *s);
};

#endif // __TELNETCONNECTION_H__
