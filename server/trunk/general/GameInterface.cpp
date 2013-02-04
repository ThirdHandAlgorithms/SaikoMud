
#include "GameInterface.h"


#include "../Globals.h"
#include <Groundfloor/Materials/GFGarbageCollector.h>


CGameInterface::CGameInterface() : TGFFreeable() {
	this->loggedInAccount = NULL;
	this->loggedInCharacter = NULL;
   
   this->sLastactionInfo.setValue_ansi("");
}

CGameInterface::~CGameInterface() {
   if (this->loggedInCharacter != NULL) {
      Global_World()->unloadCharacter(this->loggedInCharacter);
   }
}

void CGameInterface::DoChecks() {
   if (this->loggedInAccount == NULL) {
      this->sLastactionInfo.setValue_ansi("You're not logged in.");

      throw;
   }

   if (this->loggedInCharacter == NULL) {
      this->sLastactionInfo.setValue_ansi("You're not in character.");

      throw; 
   }
}


// character identity
bool CGameInterface::Login(TGFString *username, TGFString *password) {
   CAccount *acc = new CAccount( Global_DBConnection() );
   if ( acc->login(username, password) ) {
      this->loggedInAccount = acc;

      unsigned long cid = acc->getMainCharId();
      if (cid != 0) {
         this->loggedInCharacter = new CCharacter( Global_DBConnection(), cid, false );
         Global_World()->generateUniqueWorldId(this->loggedInCharacter);
         this->nickname.set( this->loggedInCharacter->name.get() );
      }

      this->sLastactionInfo.setValue_ansi("Logged in.");
   } else {
      delete acc;

      this->sLastactionInfo.setValue_ansi("No hablo whatever you're saying.");
   }

	return false;
}

void CGameInterface::Logout() {
   if (this->loggedInCharacter != NULL) {
      delete this->loggedInCharacter;

      this->loggedInCharacter = NULL;
   }

   if (this->loggedInAccount != NULL) {
      delete this->loggedInAccount;

      this->loggedInAccount = NULL;
   }
}

bool CGameInterface::IsLoggedIn() {
   return (this->loggedInAccount != NULL);
}

bool CGameInterface::MatchWithCharacterRef( void *ref ) {
   return ( this->loggedInCharacter == ref );
}

// world
void CGameInterface::inform_walkimpossible() {
   this->sLastactionInfo.setValue_ansi("You can't WALK this way.");
}

void CGameInterface::ReloadWorld() {
   this->DoChecks();

   Global_World()->reloadFromDatabase(Global_DBConnection());
}

void CGameInterface::GetTinyMap(TGFString *s) {
   this->DoChecks();

   Global_World()->echoAsciiMap(s, this->loggedInCharacter->x.get(), this->loggedInCharacter->y.get(), 5);
}

BYTE CGameInterface::GetRoomInfo(TGFString *s) {
   this->DoChecks();

   long x = this->loggedInCharacter->x.get();
   long y = this->loggedInCharacter->y.get();

   BYTE envtype = 0;

   CRoom *room = Global_World()->getRoom(x,y);
   if ( room != NULL ) {
      s->setValue(room->description.link());

      envtype = room->envtype.get();
   } else {
      // todo: error! room doesn't exist!

      s->setValue_ansi("Error!");

      if ( (x == 0) && (y == 0) ) {
         printf("Error: (0, 0) doesn't exist!\n");

         // todo: now what...
      } else {
         this->run_teleport(0,0);
      }
   }

   return envtype;
}

void CGameInterface::GetLastActionInfo(TGFString *s) {
   s->setValue(&sLastactionInfo);
}

// character actions
bool CGameInterface::run_walkforward() {
   this->DoChecks();

   long x = this->loggedInCharacter->x.get();
   long y = this->loggedInCharacter->y.get();

   CRoom *room = Global_World()->getRoom(x,y+1);
   if ( room != NULL ) {
      this->loggedInCharacter->y.set( y + 1 );
      Global_CharacterUpdate()->schedule(this->loggedInCharacter);

      return true;
   }

   return false;
}

bool CGameInterface::run_walkbackwards() {
   this->DoChecks();

   long x = this->loggedInCharacter->x.get();
   long y = this->loggedInCharacter->y.get();

   CRoom *room = Global_World()->getRoom(x,y-1);
   if ( room != NULL ) {
      this->loggedInCharacter->y.set( y - 1 );
      Global_CharacterUpdate()->schedule(this->loggedInCharacter);

      return true;
   }

   return false;
}

bool CGameInterface::run_walkleft() {
   this->DoChecks();

   long x = this->loggedInCharacter->x.get();
   long y = this->loggedInCharacter->y.get();

   CRoom *room = Global_World()->getRoom(x-1,y);
   if ( room != NULL ) {
      this->loggedInCharacter->x.set( x - 1 );
      Global_CharacterUpdate()->schedule(this->loggedInCharacter);

      return true;
   }

	return false;
}
bool CGameInterface::run_walkright() {
   this->DoChecks();

   long x = this->loggedInCharacter->x.get();
   long y = this->loggedInCharacter->y.get();

   CRoom *room = Global_World()->getRoom(x+1,y);
   if ( room != NULL ) {
      this->loggedInCharacter->x.set( x + 1 );
      Global_CharacterUpdate()->schedule(this->loggedInCharacter);

      return true;
   }

	return false;
}

// special GM actions
bool CGameInterface::run_teleport( long x, long y ) {
   this->DoChecks();

   this->sLastactionInfo.setValue_ansi("You're being teleported.");

	this->loggedInCharacter->x.set(x);
	this->loggedInCharacter->y.set(y);
	Global_CharacterUpdate()->schedule(this->loggedInCharacter);

	return true;
}

bool CGameInterface::attack_start(DWORD32 iCharId) {
   this->DoChecks();

   CCharacter *cTarget = Global_World()->getCharacter(iCharId);
   if (cTarget == NULL) {
      this->sLastactionInfo.setValue_ansi("Error");
   } else {
      if (cTarget->isAlive()) {
         CCombat *combat = Global_World()->getCombat(this->loggedInCharacter->x.get(),this->loggedInCharacter->y.get());
         combat->joinCombat(cTarget);
         this->loggedInCharacter->setTarget(cTarget);
         combat->joinCombat(this->loggedInCharacter);
         combat->start();

         return true;
      } else {
         this->sLastactionInfo.setValue_ansi("Target is already dead");
      }
   }

   return false;
}

bool CGameInterface::interact_greet(DWORD32 iCharId) {
   this->DoChecks();

   return false;
}

int CGameInterface::interact_getQuests(DWORD32 iCharId, TGFVector *v) {
   this->DoChecks();

   CCharacter *cTarget = Global_World()->getCharacter(iCharId);
   if (cTarget == NULL) {
      this->sLastactionInfo.setValue_ansi("Error");

      return -1;
   } else {
      if (cTarget->isNPC.get()) {
         return cTarget->getQuests(this->loggedInCharacter, v);
      } else {
         this->sLastactionInfo.setValue_ansi("Error");
         
         return -1;
      }
   }   

   return 0;
}

bool CGameInterface::interact_getQuestText(DWORD32 iQuestId, TGFString *s) {
   this->DoChecks();

   return Global_World()->getQuestStory(iQuestId, this->loggedInCharacter, s);
}

void CGameInterface::StartCombatDummy() {
   this->DoChecks();

   CCombat *combat = Global_World()->getCombat(this->loggedInCharacter->x.get(),this->loggedInCharacter->y.get());

   CCombatant *dummy = new CCombatant();
   dummy->currenthealthpool.set( dummy->maxhealthpool.get() );
   GFDisposable(dummy,GC_LIVEFOREVER);
   combat->joinCombat(dummy);
   dummy->setTarget(this->loggedInCharacter);
   this->loggedInCharacter->setTarget(dummy);
   combat->joinCombat(this->loggedInCharacter);
   combat->start();
}
