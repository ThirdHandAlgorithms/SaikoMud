
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

      throw 1;
   }

   if (this->loggedInAccount->isAdmin) {
      return;
   }

   if (this->loggedInCharacter == NULL) {
      this->sLastactionInfo.setValue_ansi("You're not in character.");

      throw 2; 
   }

   this->sLastactionInfo.setValue_ansi("");
}

bool CGameInterface::IsAdmin() {
   if (this->loggedInAccount != NULL) {
      return this->loggedInAccount->isAdmin;
   }

   return false;
}

// character identity
bool CGameInterface::Login(TGFString *username, TGFString *password) {
   CAccount *acc = new CAccount( Global_DBConnection() );
   if ( acc->login(username, password) ) {
      this->loggedInAccount = acc;

      unsigned long cid = acc->getMainCharId();
      if (cid != 0) {
         this->loggedInCharacter = new CCharacter( Global_DBConnection(), cid );
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

void CGameInterface::GetTinyMap(TGFString *s, uint32_t *iCurrentSelfX, uint32_t *iCurrentSelfY, uint32_t extra) {
   this->DoChecks();

   if (this->loggedInCharacter == NULL) {
      Global_World()->echoAsciiMap(s, 0, 0, 20, false);
   } else {
      long realx = this->loggedInCharacter->x.get();
      long realy = this->loggedInCharacter->y.get();

      *iCurrentSelfX = (uint32_t)((int)realx);
      *iCurrentSelfY = (uint32_t)((int)realy);

      Global_World()->echoAsciiMap(s, realx, realy, 5 + extra, true, this->loggedInCharacter);
   }
}

BYTE CGameInterface::GetRoomInfo(TGFString *s) {
   this->DoChecks();

   long x = 0, y = 0;
   if (this->loggedInCharacter != NULL) {
      x = this->loggedInCharacter->x.get();
      y = this->loggedInCharacter->y.get();
   }

   BYTE envtype = 0;

   CRoom *room = Global_World()->getRoom(x,y);
   if ( room != NULL ) {
      s->setValue(&(room->description));

      envtype = room->envtype;
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

uint32_t CGameInterface::GetLastActionInfo(TGFString *s) {
   s->setValue(&sLastactionInfo);

   if (this->loggedInCharacter != NULL) {
      return this->loggedInCharacter->WorldId;
   }

   return 0;
}

bool CGameInterface::canCompleteQuest(const CQuest *q) {
   this->DoChecks();

   if (this->loggedInCharacter != NULL) {
      std::vector<CQuestItemRequired> reqs = q->getRequiredItems();
      
      bool b = true;

      for(std::vector<CQuestItemRequired>::iterator it = reqs.begin(); it != reqs.end(); ++it) {
         b &= this->loggedInCharacter->hasItemInBags(it->item_id, it->amountrequired);

         if (!b) {
            break;
         }
      }

      return b;
   }

   return false;
}

bool CGameInterface::completeQuest(const CQuest *q) {
   this->DoChecks();

   if (this->loggedInCharacter != NULL) {
      return (Global_World()->completeQuest(q, this->loggedInCharacter) != 0);
   }

   return false;
}


bool CGameInterface::check_walkforward() {
   this->DoChecks();

   long x = this->loggedInCharacter->x.get();
   long y = this->loggedInCharacter->y.get();

   CRoom *room = Global_World()->getRoom(x,y+1);
   if ( room != NULL ) {
      return (room->traversable);
   }

   return false;
}
bool CGameInterface::check_walkbackwards() {
   this->DoChecks();

   long x = this->loggedInCharacter->x.get();
   long y = this->loggedInCharacter->y.get();

   CRoom *room = Global_World()->getRoom(x,y-1);
   if ( room != NULL ) {
      return (room->traversable);
   }

   return false;
}
bool CGameInterface::check_walkleft() {
   this->DoChecks();

   long x = this->loggedInCharacter->x.get();
   long y = this->loggedInCharacter->y.get();

   CRoom *room = Global_World()->getRoom(x-1,y);
   if ( room != NULL ) {
      return (room->traversable);
   }

   return false;
}
bool CGameInterface::check_walkright() {
   this->DoChecks();

   long x = this->loggedInCharacter->x.get();
   long y = this->loggedInCharacter->y.get();

   CRoom *room = Global_World()->getRoom(x+1,y);
   if ( room != NULL ) {
      return (room->traversable);
   }

   return false;
}

// character actions
bool CGameInterface::run_walkforward() {
   this->DoChecks();

   long x = this->loggedInCharacter->x.get();
   long y = this->loggedInCharacter->y.get();

   CRoom *room = Global_World()->getRoom(x,y+1);
   if ( room != NULL ) {
      if (room->traversable) {
         this->loggedInCharacter->y.set( y + 1 );
         Global_CharacterUpdate()->schedule(this->loggedInCharacter);

         return true;
      }
   }

   return false;
}

bool CGameInterface::run_walkbackwards() {
   this->DoChecks();

   long x = this->loggedInCharacter->x.get();
   long y = this->loggedInCharacter->y.get();

   CRoom *room = Global_World()->getRoom(x,y-1);
   if ( room != NULL ) {
      if (room->traversable) {
         this->loggedInCharacter->y.set( y - 1 );
         Global_CharacterUpdate()->schedule(this->loggedInCharacter);

         return true;
      }
   }

   return false;
}

bool CGameInterface::run_walkleft() {
   this->DoChecks();

   long x = this->loggedInCharacter->x.get();
   long y = this->loggedInCharacter->y.get();

   CRoom *room = Global_World()->getRoom(x-1,y);
   if ( room != NULL ) {
      if (room->traversable) {
         this->loggedInCharacter->x.set( x - 1 );
         Global_CharacterUpdate()->schedule(this->loggedInCharacter);

         return true;
      }
   }

	return false;
}
bool CGameInterface::run_walkright() {
   this->DoChecks();

   long x = this->loggedInCharacter->x.get();
   long y = this->loggedInCharacter->y.get();

   CRoom *room = Global_World()->getRoom(x+1,y);
   if ( room != NULL ) {
      if (room->traversable) {
         this->loggedInCharacter->x.set( x + 1 );
         Global_CharacterUpdate()->schedule(this->loggedInCharacter);

         return true;
      }
   }

	return false;
}

bool CGameInterface::equip_itemfrombags(uint32_t iItemId) {
   this->DoChecks();

   CItem *item = Global_World()->getItem(iItemId);
   if (item != NULL) {
      bool b =  this->loggedInCharacter->equipItem(item);
      if (b) {
         Global_CharacterUpdate()->scheduleBagSave(this->loggedInCharacter);
      }
      return b;
   }

   return false;
}

bool CGameInterface::dequip_item(uint32_t iItemId) {
   // todo
   return false;
}

uint32_t CGameInterface::getPlayerWorldId() {
   this->DoChecks();

   if (this->loggedInCharacter != NULL) {
      return this->loggedInCharacter->WorldId;
   }

   return 0;
}

// special GM actions
bool CGameInterface::run_teleport( long x, long y ) {
   this->DoChecks();

   this->sLastactionInfo.setValue_ansi("You're being teleported.");

   CRoom *room = Global_World()->getRoom(x,y);
   if ( room != NULL ) {
      if (room->traversable) {
	      this->loggedInCharacter->x.set(x);
	      this->loggedInCharacter->y.set(y);
	      Global_CharacterUpdate()->schedule(this->loggedInCharacter);

         return false;
      }
   }

	return false;
}

bool CGameInterface::admin_teleport_player( uint32_t iWorldId, long x, long y ) {
   this->DoChecks();

   this->sLastactionInfo.setValue_ansi("You're being teleported.");

   CRoom *room = Global_World()->getRoom(x,y);
   CCharacter *player = Global_World()->getCharacter(iWorldId);
   if ((room != NULL) && (player != NULL)) {
      if (room->traversable) {
	      player->x.set(x);
	      player->y.set(y);
	      Global_CharacterUpdate()->schedule(player);

         return false;
      }
   }

	return false;
}

bool CGameInterface::attack_start(uint32_t iWorldId) {
   this->DoChecks();

   CCharacter *cTarget = Global_World()->getCharacter(iWorldId);
   if (cTarget == NULL) {
      this->sLastactionInfo.setValue_ansi("Error");
   } else {
      if (cTarget->isAlive()) {
         CCombat *combat = Global_World()->getCombat(this->loggedInCharacter->x.get(),this->loggedInCharacter->y.get());

         // start attack by targetting and starting combat
         this->loggedInCharacter->setTarget(cTarget);
         combat->joinCombat(this->loggedInCharacter);

         // target targets and hits back in return
         cTarget->setTarget(this->loggedInCharacter);
         combat->joinCombat(cTarget, true);

         combat->start();

         return true;
      } else {
         this->sLastactionInfo.setValue_ansi("Target is already dead");
      }
   }

   return false;
}

bool CGameInterface::cast_spell(uint32_t iWorldId, uint32_t iSpellId) {
   this->DoChecks();

   CCharacter *cTarget = Global_World()->getCharacter(iWorldId);
   if (cTarget == NULL) {
      this->sLastactionInfo.setValue_ansi("Error");
   } else {
      // TODO: check that when spell hits -> combat is started (opposed to attack_start() where combat starts directly)

      // TODO: put spell in indexed cachearray
      CSpell *tmp = new CSpell(Global_DBConnection(), iSpellId);

      return this->loggedInCharacter->castSpell(tmp, cTarget);
   }

   return false;
}

bool CGameInterface::interact_greet(uint32_t iCharId, TGFString *sGreeting) {
   this->DoChecks();

   CCharacter *cTarget = Global_World()->getCharacter(iCharId);
   if (cTarget == NULL) {
      this->sLastactionInfo.setValue_ansi("Error");
   } else {
      if (cTarget->isNPC) {
         return Global_World()->getGreeting(iCharId, this->loggedInCharacter, sGreeting);
      } else {
         this->sLastactionInfo.setValue_ansi("Error");
      }
   }

   return false;
}

int CGameInterface::interact_getQuests(uint32_t iCharId, TGFVector *v) {
   this->DoChecks();

   CCharacter *cTarget = Global_World()->getCharacter(iCharId);
   if (cTarget == NULL) {
      this->sLastactionInfo.setValue_ansi("Error");

      return -1;
   } else {
      if (cTarget->isNPC) {
         return reinterpret_cast<CNPCharacter *>(cTarget)->getQuests(this->loggedInCharacter, v);
      } else {
         this->sLastactionInfo.setValue_ansi("Error");
         
         return -1;
      }
   }   

   return 0;
}

bool CGameInterface::interact_getQuestText(uint32_t iQuestId, TGFString *s, long *rewards_xp) {
   this->DoChecks();

   return Global_World()->getQuestStory(iQuestId, this->loggedInCharacter, s, rewards_xp);
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

int CGameInterface::radar_getNearbyNpcs(TGFVector *v) {
   this->DoChecks();

   if (this->loggedInCharacter != NULL) {
      long x = this->loggedInCharacter->x.get();
      long y = this->loggedInCharacter->y.get();

      return Global_World()->getNpcsInRoom_fromdb(x, y, v);
   }  else {
      return 0;
   }
}

int CGameInterface::radar_getNearbyPlayers(TGFVector *v) {
   this->DoChecks();

   if (this->loggedInCharacter != NULL) {
      long x = this->loggedInCharacter->x.get();
      long y = this->loggedInCharacter->y.get();

      return Global_World()->getNearbyPlayers(x, y, 5, v, this->loggedInCharacter);
   } else {
      return 0;
   }
}


bool CGameInterface::inform_SelfAboutAllStats() {
   this->DoChecks();

   if (this->loggedInCharacter != NULL) {
      Global_World()->informAboutAllStats(this->loggedInCharacter, this->loggedInCharacter);
   }

   return true;
}

int CGameInterface::getOwnBagSlots(TGFVector *v) {
   this->DoChecks();

   v->autoClear = false;
   if (this->loggedInCharacter != NULL) {
      std::vector<unsigned long> bagslots = this->loggedInCharacter->getBagSlots();
      
      CWorld *world = Global_World();

      for (std::vector<unsigned long>::iterator it = bagslots.begin(); it != bagslots.end(); ++it) {
         CItem *item = world->getItem( *it );
         
         v->addElement(item);
      }

      return v->size();
   }

   return -1;
}
