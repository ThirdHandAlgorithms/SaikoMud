
#ifndef __GameInterface_H__
#define __GameInterface_H__


#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Molecules/GFString.h>
#include "../general/Account.h"
#include "../world/Character.h"
#include "../world/Items.h"


class CGameInterface: public TGFFreeable {
protected:
   CCharacter *loggedInCharacter;
   CAccount *loggedInAccount;

   TGFString sLastactionInfo;

	void inform_walkimpossible();

   void DoChecks();

public:
   TGFStringProperty nickname; 

   CGameInterface();
   ~CGameInterface();

   // character identity
   bool Login(TGFString *username, TGFString *password);
   void Logout();
   bool MatchWithCharacterRef(void *ref);

   bool IsLoggedIn();

   bool IsAdmin();

   // world
   void ReloadWorld();
   void GetTinyMap(TGFString *s, uint32_t *iCurrentSelfX, uint32_t *iCurrentSelfY);

   uint32_t GetLastActionInfo(TGFString *s);
   BYTE GetRoomInfo(TGFString *s);

   bool canCompleteQuest(CQuest *q);

   // character actions
   bool run_walkforward();
   bool run_walkbackwards();
   bool run_walkleft();
   bool run_walkright();

   bool attack_start(uint32_t iWorldId);
   //bool attack_stop();
   // ...

   int radar_getNearbyNpcs(TGFVector *v);
   int radar_getNearbyPlayers(TGFVector *v);

   bool interact_greet(uint32_t iCharId, TGFString *sGreeting);
   int interact_getQuests(uint32_t iCharId, TGFVector *v);
   bool interact_getQuestText(uint32_t iQuestId, TGFString *s, long *rewards_xp);

   bool inform_SelfAboutAllStats();

   int getOwnBagSlots(TGFVector *v);

   // special GM actions
   bool run_teleport( long x, long y );

   bool admin_teleport_player( uint32_t iWorldId, long x, long y );

	// 
	void StartCombatDummy();
};


#endif // __GameInterface_H__
