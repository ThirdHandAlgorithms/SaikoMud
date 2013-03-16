
#ifndef __TELNETCONNECTION_H__
#define __TELNETCONNECTION_H__

#include <Jumpropes/JRThreadedConnection.h>
#include <Groundfloor/Molecules/GFString.h>

#include "../general/GameInterface.h"

#define c_run_walkforward 0x00000005
#define c_run_walkbackwards 0x00000006
#define c_run_walkleft 0x00000007
#define c_run_walkright 0x00000008

#define c_attack_start 0x20000009
#define c_attack_stop 0x0000000a

#define c_chat_say 0x30000010

#define c_interact_greet 0x20000020
#define c_interact_getquesttitles 0x20000021
#define c_interact_getquesttext 0x20000022

#define c_radar_getnearbynpcs 0x00000101
#define c_radar_getnearbyplayers 0x00000102

#define c_self_getallstats 0x00000201

#define c_info_getiteminfo 0x20000301
#define c_info_getitemstats 0x20000302

#define c_response_lastactioninfo 0x30010000
#define c_response_roominfo 0x30020000
#define c_response_asciimap 0x10030000

#define c_event_earnsxp 0x20040001
#define c_event_combatmsg 0x70040002

#define c_event_statinfo_level 0x20040101
#define c_event_statinfo_totalxp 0x20040102
#define c_event_statinfo_totalhp 0x20040103
#define c_event_statinfo_hp 0x20040104
#define c_event_statinfo_strength 0x20040105
#define c_event_statinfo_energy 0x20040106
#define c_event_statinfo_protection 0x20040107


#define c_response_npcinfo 0x30110000
#define c_response_dialog 0x30120000

#define c_response_questtitle 0x30130000
#define c_response_questtext 0x30140000

//#define c_response_completedquest 0x3015000

#define c_response_iteminfo 0x70300001
#define c_response_itemstats 0x70300002

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
   void inform_questtitle(uint32_t iQuestId, TGFString *s);
   void inform_questtext(uint32_t iQuestId, TGFString *s, long rewards_xp);
   void inform_npcinfo(uint32_t iWorldId, TGFString *s);
   void inform_npcdialog(uint32_t iWorldId, TGFString *s);
   bool inform_iteminfo(uint32_t iItemId);
   bool inform_itemstats(uint32_t iItemId);

   //void inform_currentplayerstats();

   bool decodeNextBinMessageInBuffer(uint32_t *command, uint32_t *intparam1, uint32_t *intparam2, TGFString *s, uint32_t *intparam3, uint32_t *intparam4);
public:
   CTelnetConnection( TJRBaseSocket *aSocket );
   ~CTelnetConnection();

   bool inBinaryMode();

   void newMessageReceived( const TGFString *sMessage );

   void send( const TGFString *s );
   void sendBin(uint32_t command, uint32_t intparam1, uint32_t intparam2, TGFString *s, uint32_t intparam3 = 0, uint32_t intparam4 = 0);

   bool matchWithCharacterRef( void *ref );

   void append_nickname(TGFString *s);

   void inform_earnxp(long xp, long totalxp);
   void inform_combatevent(uint32_t iSourceWorldId, uint32_t iTargetWorldId, int eventtype, int amount, TGFString *combatmsg);
   void informAboutAllStats(CCharacter *cAbout);

   void inform_map();
};

#endif // __TELNETCONNECTION_H__
