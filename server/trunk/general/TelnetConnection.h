
#ifndef __TELNETCONNECTION_H__
#define __TELNETCONNECTION_H__

#include <Jumpropes/JRThreadedConnection.h>
#include <Groundfloor/Molecules/GFString.h>

#include "../general/GameInterface.h"

#define c_run_walkforward 0x00000005
#define c_run_walkbackwards 0x00000006
#define c_run_walkleft 0x00000007
#define c_run_walkright 0x00000008

#define c_check_walkforward 0x00000015
#define c_check_walkbackwards 0x00000016
#define c_check_walkright 0x00000017
#define c_check_walkleft 0x00000018

#define c_attack_start 0x20000009
#define c_attack_stop 0x0000000a

#define c_cast_spell 0x2000000b

#define c_chat_say 0x30000010

#define c_interact_greet 0x20000020
#define c_interact_getquesttitles 0x20000021
#define c_interact_getquesttext 0x20000022
#define c_interact_getquestitemsrequired 0x20000023

#define c_radar_getnearbynpcs 0x00000101
#define c_radar_getnearbyplayers 0x00000102
#define c_radar_getmap 0x00000109

#define c_self_getallstats 0x00000201
#define c_self_getspells 0x00000202
#define c_info_getspellinfo 0x20000203

#define c_info_getiteminfo 0x20000301
#define c_info_getitemstats 0x20000302

#define c_info_getgearslots 0x30400001
#define c_info_equipitem 0x60400002     // 6 means... 4+2+0... --> intcommand + bExtIntParamCommand ... 
#define c_info_dequipitem 0x60400003

#define c_self_getbagslots 0x00000411
//#define c_info_deletefrombagslots 0x30410002

#define c_response_lastactioninfo 0x30010000
#define c_response_roominfo 0x30020000
#define c_response_asciimap 0x70030000

#define c_event_earnsxp 0x20040001
#define c_event_combatmsg 0x70040002

#define c_event_statinfo_level 0x20040101
#define c_event_statinfo_totalxp 0x20040102
#define c_event_statinfo_totalhp 0x20040103
#define c_event_statinfo_hp 0x20040104
#define c_event_statinfo_strength 0x20040105
#define c_event_statinfo_energy 0x20040106
#define c_event_statinfo_protection 0x20040107


#define c_response_npcinfo 0x70110000
#define c_response_dialog 0x30120000

#define c_response_questtitle 0x30130000
#define c_response_questtext 0x30140000
#define c_response_questitemrequired 0x30150000

//#define c_response_completedquest 0x3015000

#define c_response_iteminfo 0x70300001
#define c_response_itemstats 0x70300002
#define c_response_spellinfo 0x70300003

#define c_response_gearslots 0x80400001
#define c_response_bagslots 0x80410001
#define c_response_spells 0x80420001

#define c_response_playerinfo 0x70210001

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

   __int64 iLastTimeSentMap;

   TGFString buffer;

   CGameInterface gameintf;

   void inform_currentroom();
   void inform_lastaction();
   void inform_questtitle(uint32_t iQuestId, TGFString *s, bool bCanComplete);
   void inform_questtext(uint32_t iQuestId, TGFString *s, long rewards_xp);
   void inform_questitemrequired(uint32_t iQuestId, uint32_t item_id, uint32_t iNumberRequired, TGFString *sItemName);
   void inform_npcinfo(uint32_t iWorldId, TGFString *s, uint32_t x, uint32_t y);
   void inform_npcdialog(uint32_t iWorldId, TGFString *s);
   bool inform_iteminfo(uint32_t iItemId);
   bool inform_itemstats(uint32_t iItemId);
   bool inform_spellinfo(uint32_t iSpellId);

   bool inform_gearslots(uint32_t iWorldId);

   bool inform_equipeditem(uint32_t iSlotId);

   void inform_playerinfo(CCharacter *c);

   //void inform_currentplayerstats();

   bool decodeNextBinMessageInBuffer(uint32_t *command, uint32_t *intparam1, uint32_t *intparam2, TGFString *s, uint32_t *intparam3, uint32_t *intparam4);

   void send( const TGFString *s );
   void sendBin(uint32_t command, uint32_t intparam1, uint32_t intparam2, TGFString *s, uint32_t intparam3 = 0, uint32_t intparam4 = 0);
   void sendBin2(uint32_t command, std::vector<uint32_t> *intarray, TGFStringVector *strarray);
public:
   CTelnetConnection( TJRBaseSocket *aSocket );
   ~CTelnetConnection();

   bool inBinaryMode();

   void newMessageReceived( const TGFString *sMessage );

   bool matchWithCharacterRef( void *ref );

   void append_nickname(TGFString *s);

   void inform_earnxp(long xp, long totalxp);
   void inform_combatevent(uint32_t iSourceWorldId, uint32_t iTargetWorldId, int eventtype, int amount, TGFString *combatmsg);
   void informAboutAllStats(CCharacter *cAbout);
   bool inform_self_bagslots();
   bool inform_playerspells(uint32_t iWorldId);

   void sendChatMessage(uint32_t iChannelNr, TGFString *sMsg);

   void inform_map(uint32_t extrainfo, uint32_t extrasize = 0);
};

#endif // __TELNETCONNECTION_H__
