
#ifndef __GameInterface_H__
#define __GameInterface_H__


#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Molecules/GFString.h>
#include "../general/Account.h"
#include "../world/Character.h"


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

   // world
   void ReloadWorld();
   void GetTinyMap(TGFString *s);

   void GetLastActionInfo(TGFString *s);
   BYTE GetRoomInfo(TGFString *s);

   // character actions
   bool run_walkforward();
   bool run_walkbackwards();
   bool run_walkleft();
   bool run_walkright();

   bool attack_start(TGFString *sMobIdentifier);
   //bool attack_stop();
   // ...

   // special GM actions
   bool run_teleport( long x, long y );


	// 
	void StartCombatDummy();
};


#endif // __GameInterface_H__