
#include "World.h"

#include "Character.h"
#include <Groundfloor/Materials/GFGarbageCollector.h>

#include "../Globals.h"

#include <Groundfloor/Materials/GFFunctions.h>

#include <vector>

CWorld::CWorld() : TGFFreeable() {
   worldids.autoClear = false;
   worldids.resizeVector(1024);

   itemcache.autoClear = true;
}

CWorld::~CWorld() {
}

CCombat *CWorld::getCombat( long x, long y ) {
   unsigned long vpos = (y + y_fix) * w + x + x_fix;

   CCombat *obj = static_cast<CCombat *>( combats.elementAt(vpos) );
   if ( obj != NULL ) {
      if ( !obj->isRunning() ) {
         this->endCombat(obj);

         obj = NULL;
      }
   }

   if ( obj == NULL ) {
      obj = new CCombat();
      combats.insertAt(vpos,obj);
   }

   return obj;
}

void CWorld::endCombat( CCombat *c ) {
   combats.removeElement(c);
   
   delete c;
}

void CWorld::reloadFromDatabase( TMySQLSquirrelConnection *pConn ) {
   rooms.clear();

   this->conn = pConn;

   TMySQLSquirrel qry( pConn );
   TGFBRecord rec;

   TGFString sql("select min(x), min(y), max(x), max(y), count(x) from grid");
   qry.setQuery(&sql);
   if ( qry.open() ) {
      qry.next();

      qry.fetchRecord(&rec);

      x_min = rec.getValue(0)->asInteger();
      y_min = rec.getValue(1)->asInteger();
      x_max = rec.getValue(2)->asInteger();
      y_max = rec.getValue(3)->asInteger();
      reccount = rec.getValue(4)->asInteger();

      qry.close();
   }

   h = y_max - y_min + 1;
   w = x_max - x_min + 1;

   x_fix = x_min * -1;
   y_fix = y_min * -1;

   rooms.resizeVector( w * h );
   rooms.setElementCount( w * h );

   combats.resizeVector( w * h );
   combats.setElementCount( w * h );

   sql.setValue_ansi("select * from grid order by y asc, x asc");
   qry.setQuery(&sql);
   TSquirrelReturnData errData;
   if ( qry.open(&errData) ) {
      TGFBFields flds;
      qry.fetchFields(&flds);

      int xind = flds.getFieldIndex_ansi("x");
      int yind = flds.getFieldIndex_ansi("y");
      int descind = flds.getFieldIndex_ansi("desc");
      int envtypeind = flds.getFieldIndex_ansi("envtype");
      int activeind = flds.getFieldIndex_ansi("active");

      while ( qry.next() ) {
         qry.fetchRecord(&rec);

         CRoom *room = new CRoom();

         // this was previously a function within CRoom, inlining for speed
         room->traversable = rec.getValue(activeind)->asBoolean();
         room->x = rec.getValue(xind)->asInteger();
         room->y = rec.getValue(yind)->asInteger();

         room->envtype = static_cast<BYTE>(rec.getValue(envtypeind)->asInteger());

         room->description.setValue( rec.getValue(descind)->asString() );
         // end

         long x = room->x;
         long y = room->y;

         rooms.insertAt( (y + y_fix) * w + x + x_fix, room );
      }

      qry.close();
   } else {
      printf("Error %d: %s\n", errData.errorcode, errData.errorstring.getValue());
   }

   this->reloadNpcs();

   // todo: bugfix

   // todo: reload quests etc
}

void CWorld::reloadNpcs() {
   npcs.clear();

   TMySQLSquirrel qry( this->conn );
   TGFBRecord rec;

   CNPCharacter *c;

   TSquirrelReturnData errData;

   TGFString sqlPrepareDBSync("update `char` set current_worldid=NULL where account_id=0");
   qry.setQuery(&sqlPrepareDBSync);
   if ( !qry.open(&errData) ) {
      printf("Error %d: %s\n", errData.errorcode, errData.errorstring.getValue());
   }

   TGFString sql("select * from `char` where account_id=0 order by id asc");
   qry.setQuery(&sql);
   if ( qry.open(&errData) ) {
      while ( qry.next() ) {
         qry.fetchRecord(&rec);

         c = new CNPCharacter(this->conn, &qry);
         generateUniqueWorldId(c);
         this->loadNeededQuests(c);
         this->npcs.addElement(c);
      }

      qry.close();
   } else {
      printf("Error %d: %s\n", errData.errorcode, errData.errorstring.getValue());
   }

   for (unsigned int j = 0; j < this->npcs.size(); j++) {
      c = static_cast<CNPCharacter *>(this->npcs.elementAt(j));
      Global_CharacterUpdate()->schedule(c);
   }
}

int CWorld::getNpcsInRoom_fromdb(long x, long y, TGFVector *v) {
   TMySQLSquirrel qry( this->conn );
   TGFBRecord rec;

   int cTotal = 0;

   TSquirrelReturnData errData;

   TGFString sql("select current_worldid from `char` where x=:x and y=:y and account_id=0 and current_worldid is not null");
   qry.setQuery(&sql);
   qry.findOrAddParam("x")->setInteger(x);
   qry.findOrAddParam("y")->setInteger(y);
   if ( qry.open(&errData) ) {
      while ( qry.next() ) {
         qry.fetchRecord(&rec);

         long worldid = rec.getValue(0)->asInteger();
         CCharacter *c = this->getCharacter(worldid);
         if (c != NULL) {
            v->addElement(c);
            cTotal++;
         }
      }

      qry.close();
   } else {
      printf("Error %d: %s\n", errData.errorcode, errData.errorstring.getValue());
   }

   return cTotal;
}

CNPCharacter *CWorld::getNpcByName(TGFString *s) {
   CNPCharacter *obj;
   unsigned long c = this->npcs.size();
   for (unsigned long i = 0; i < c; i++) {
      obj = static_cast<CNPCharacter *>( this->npcs.elementAt(i) );
      if (obj != NULL) {
         if (s->match(obj->name.link())) {
            return obj;
         }
      }
   }

   return NULL;
}

bool CWorld::hasNPCsAt(long x, long y) {
   CCharacter *obj;
   unsigned long c = this->npcs.size();
   for (unsigned long i = 0; i < c; i++) {
      obj = static_cast<CCharacter *>( this->npcs.elementAt(i) );
      if (obj != NULL) {
         if ((obj->x.get() == x) && (obj->y.get() == y) && (obj->currenthealthpool.get() > 0)) {
            return true;
         }
      }
   }

   return false;
}

bool CWorld::hasPlayersAt(long x, long y, CCharacter *oExcludeChar) {
   CCharacter *obj;
   unsigned long c = this->characters.size();
   for (unsigned long i = 0; i < c; i++) {
      obj = static_cast<CCharacter *>( this->characters.elementAt(i) );
      if ((obj != NULL) && (obj != oExcludeChar)) {
         if (obj->isAlive() && (obj->x.get() == x) && (obj->y.get() == y)) {
            return true;
         }
      }
   }

   return false;
}

int CWorld::getNearbyPlayers(long x, long y, unsigned int radius, TGFVector *v, CCharacter *oExcludeChar) {
   long size_w = (radius << 1) + 1;  // +1 for center
   long size_h = (radius << 1) + 1;
   long start_x = x - radius;
   long start_y = y - radius;
   long end_x = (start_x + size_w);
   long end_y = (start_y + size_h);

   int playercount = 0;

   CCharacter *obj;
   unsigned long c = this->characters.size();
   for (unsigned long i = 0; i < c; i++) {
      obj = static_cast<CCharacter *>( this->characters.elementAt(i) );
      if (obj != NULL) {
         if ((obj != oExcludeChar) && obj->isAlive() &&
             ((obj->x.get() >= start_x) && (obj->x.get() < end_x)) && 
             ((obj->y.get() >= start_y) && (obj->y.get() < end_y))) {
            v->addElement(obj);
            playercount++;
         }
      }
   }

   return playercount;
}


void CWorld::preloadInteriors( long x, long y ) {

}

CRoom *CWorld::getRoom( long x, long y ) {
   if (  (x < this->x_min) || (x > this->x_max) ||
         (y < this->y_min) || (y > this->y_max) ) {
      return NULL;
   }

   unsigned long vpos = (y + y_fix) * w + x + x_fix;

   return static_cast<CRoom *>( rooms.elementAt(vpos) );
}

void CWorld::echoAsciiMap( TGFString *s, long x, long y, unsigned int radius, bool bShowCharacterPos, CCharacter *self ) {
   long size_w = (radius << 1) + 1;  // +1 for center
   long size_h = (radius << 1) + 1;
   long start_x = x - radius;
   long start_y = y - radius;

   this->preloadInteriors(x,y);

   s->setSize(size_w*size_h + size_h*2);
   TGFString line;
   line.setSize(size_w + 2);

   // following is really slow, but the maths of this is way easier
   for ( long i = start_y; i < (start_y + size_h); i++ ) {
      line.setLength(0);
      for ( long j = start_x; j < (start_x + size_w); j++ ) {
         if ( (x == j) && (y == i) ) {
            CRoom *room = this->getRoom(j,i);
            if ( room != NULL ) {
               if (this->hasNPCsAt(j,i)) {
                  line.append(static_cast<char>(97 + room->envtype));
               } else if (this->hasPlayersAt(j, i, self)) {
                  line.append(static_cast<char>(97 + room->envtype));
               } else if (bShowCharacterPos) {
                  line.append_ansi("@");
               }
            } else if (bShowCharacterPos) {
               line.append_ansi("@");
            }
         } else {
            CRoom *room = this->getRoom(j,i);
            if ( room != NULL ) {
               if (this->hasNPCsAt(j,i)) {
                  line.append(static_cast<char>((65 + room->envtype)));
               } else if (this->hasPlayersAt(j, i, self)) {
                  line.append(static_cast<char>(65 + room->envtype));
               } else {
                  line.append(static_cast<char>((48 + room->envtype)));
               }
            } else {
               line.append_ansi(" ");
            }
         }
      }

      line.append(13);
      line.append(10);
      s->prepend(&line);
   }
}

CCharacter *CWorld::getCharacter(uint32_t iWorldId) {
   return static_cast<CCharacter *>(worldids.elementAt(iWorldId));
}

CItem *CWorld::getItem(uint32_t iItemId) {
   // todo: what to do when item id exceeds 32bits uint
   CItem *item = static_cast<CItem *>(itemcache.elementAt(iItemId));

   if (item == NULL) {
      item = new CItem();
      item->loadFromDb(this->conn, iItemId);

      if (itemcache.size() < iItemId) {
         itemcache.resizeVector(iItemId+1);
         itemcache.setElementCount(iItemId+1);
      }
      itemcache.replaceElement(iItemId, item);
   }

   return item;
}

CBaseCombatStats *CWorld::getItemStats(uint32_t iItemId) {
   CItem *item = this->getItem(iItemId);
   if (item != NULL) {
      return new CCombatStats( this->conn, item->stats_id );
   }
   return NULL;
}

uint32_t CWorld::generateUniqueWorldId(CCharacter *c) {
   uint32_t r;

   worldidlock.lock();
   try  {

      r = (rand() % 0xffff) + 1;

      TGFFreeable *obj = NULL;

      while ( (obj = worldids.elementAt(r)) != NULL ) {
         r = (rand() % 0xffff) + 1;
      }


      if (worldids.size() < r) {
         worldids.resizeVector(r+1);
         worldids.setElementCount(r+1);
      }
      worldids.replaceElement(r, c);

      c->WorldId = r;

      if (!c->isNPC) {
         this->characters.addElement(c);
      }
   } catch (...) {
      worldidlock.unlock();
      throw;
   }
   worldidlock.unlock();

   return r;
}

void CWorld::unloadCharacter(CCharacter *c) {
   if (c->WorldId != 0) {
      worldidlock.lock();
      try  {
         this->characters.removeElement(c);
         this->characters.compress();

         worldids.replaceElement(c->WorldId, NULL);
      } catch (...) {
         worldidlock.unlock();
         throw;
      }
      worldidlock.unlock();
   }
}

CQuest *CWorld::getQuest(uint32_t id) {
   return static_cast<CQuest *>( this->quests.elementAt(id) );
}

void CWorld::loadNeededQuests(CNPCharacter *cNpc) {
   TGFString sql("select `quest`.* from `npc_quest` left outer join `quest` on (`quest`.`id`=`npc_quest`.`quest_id`) where char_id=:char_id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("char_id")->setInteger(cNpc->id);
   
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      TGFBFields flds;
      qry.fetchFields(&flds);

      int fld_id = flds.getFieldIndex_ansi("id");
      int fld_title = flds.getFieldIndex_ansi("title");
      int fld_story = flds.getFieldIndex_ansi("story");
      int fld_prereq = flds.getFieldIndex_ansi("prereq_quest_id");
      int fld_rewardsxp = flds.getFieldIndex_ansi("rewards_xp");
      int fld_autocomplete = flds.getFieldIndex_ansi("autocomplete");

      while ( qry.next() ) {
         TGFBRecord rec;
         qry.fetchRecord(&rec);

         CQuest *quest = new CQuest();
         quest->id = rec.getValue(fld_id)->asInteger();
         quest->prereq_quest_id = rec.getValue(fld_prereq)->asInteger();
         quest->title.setValue( rec.getValue(fld_title)->asString() );
         quest->story.setValue( rec.getValue(fld_story)->asString() );
         quest->rewards_xp = rec.getValue(fld_rewardsxp)->asInteger();
         quest->autocomplete = (rec.getValue(fld_autocomplete)->asInteger() == 1);

         if (this->quests.size() <= quest->id) {
            this->quests.resizeVector(quest->id + 1);
            this->quests.setElementCount(quest->id + 1);
         }
         this->quests.replaceElement(quest->id, quest);

         cNpc->_addQuest(quest);
      }

      qry.close();
   } else {
      printf("CWorld::loadNeededQuests(): %s\n", err.errorstring.getValue());
   }
}

long CWorld::completeQuest(CQuest *q, CCharacter *cFor) {
   long xp = q->rewards_xp;

   if (cFor->completeQuest(q->id, xp)) {
      Global_CharacterUpdate()->schedule(cFor);

      // xp earned message before sql update :S - oh well...
      CTelnetConnection *tc = Global_Server()->getClientFromPool(cFor);
      if (tc != NULL) {
         // inform player
         tc->inform_earnxp(xp, cFor->xp.get());
      }
   } else {
      // some error happened.. pretend we didn't notice for the time being..
   }

   return xp;
}

void CWorld::onRespawnTimerCharacter(TGFFreeable *c) {
   CCharacter *cFor = reinterpret_cast<CCharacter *>(c);

   cFor->timeofdeath = 0;

   if (!cFor->isNPC) {
      long x = 0, y = 0;
	   cFor->x.set(x);
	   cFor->y.set(y);
      cFor->currenthealthpool.set(cFor->maxhealthpool.get());
	   Global_CharacterUpdate()->schedule(cFor);

      CTelnetConnection *tc = Global_Server()->getClientFromPool(cFor);
      if (tc != NULL) {
         tc->inform_map();
         tc->informAboutAllStats(cFor);
      }
   } else {
      // is npc

      CNPCharacter *npc = reinterpret_cast<CNPCharacter *>(cFor);

      cFor->currenthealthpool.set(cFor->maxhealthpool.get());
	   Global_CharacterUpdate()->schedule(cFor);

   }
}

void CWorld::handleDeath(CCharacter *cFor, CCharacter *cKilledBy) {
   // lose all xp (of current level...)
   long xp = -1 * cFor->xp.get();
   cFor->xp.lockedAdd(xp);
   Global_CharacterUpdate()->schedule(cFor);

   CTelnetConnection *tc = Global_Server()->getClientFromPool(cFor);
   if (tc != NULL) {
      tc->inform_earnxp(xp, cFor->xp.get());
   }

   if (cFor->isNPC && !cKilledBy->isNPC) {
      CNPCharacter *npc = static_cast<CNPCharacter *>(cFor);
      
      bool bNotEnoughBagspace = false;
      bool bEarnedItems = false;

      // earn xp for killing npc
      long earnedxp = npc->level.get() * 10;
      cKilledBy->xp.lockedAdd(earnedxp);

      // earn random item dropped by npc
      unsigned long drop_item_id = npc->getRandomItemDrop();
      if (drop_item_id != 0) {
         bEarnedItems = true;

         if (!cKilledBy->addToBags(drop_item_id)) {
            bNotEnoughBagspace = true;
         }
      }

      // earn fixed quest items that are dropped by npc (if you have the quest)
      std::vector<unsigned long> questdrops = npc->getPossibleQuestDrops();
      for (int i = 0; i < questdrops.size(); i++) {
         bool bEarnedThisItem = false;

         CItem *item = this->getItem(questdrops[i]);
         if (item != NULL) {

            // .. get list of quests that need this item
            // .. check if player is one of these quests
            // .. put item in bag if player has one of the quest
            
            // .. if the item is not part of a quest (but was returned because it has 100% dropchance) -> still add to bag as long as there is space  <-- not covered by query

            /// use select join, otherwise this will be too slow

            //this->IsItemUsedInQuestThatPlayerPickedUp()
            TGFString sql(
               "select quest_item.amountrequired, quest_item.quest_id, questhistory.dt_pickedup, questhistory.dt_completed \
               from quest_item \
               left outer join questhistory on (questhistory.quest_id=quest_item.quest_id and questhistory.char_id=:char_id) \
               where quest_item.item_id=:item_id"
            );

            TMySQLSquirrel qry(Global_DBConnection());
            qry.setQuery(&sql);

            qry.findOrAddParam("char_id")->setInteger(cKilledBy->id);
            qry.findOrAddParam("item_id")->setInteger(item->id);

            TSquirrelReturnData err;
            if (qry.open(&err) ) {
               TGFBFields flds;
               TGFBRecord rec;

               qry.fetchFields(&flds);

               int c = 0;

               // can return multiple quest's that need this item
               while (qry.next()) {
                  c++;

                  qry.fetchRecord(&rec);

                  __int64 ts = rec.getValue(flds.getFieldIndex_ansi("dt_pickedup"))->asTimestamp();      // does this return 0 when it's value is NULL?
                  if (ts > 0) {
                     ts = rec.getValue(flds.getFieldIndex_ansi("dt_completed"))->asTimestamp();
                     if (ts == 0) {
                        bEarnedThisItem = true;
                        break;
                     }
                  }

               }

               if (c == 0) {
                  // no quests uses this item, so you can just have it
                  bEarnedThisItem = true;
               }
            }
         }

         if ( bEarnedThisItem ) {
            bEarnedItems = true;
            if ( !cKilledBy->addToBags(questdrops[i]) ) {
               bNotEnoughBagspace = true;
            }
         }
      }

      Global_CharacterUpdate()->schedule(cKilledBy);
      
      if (bEarnedItems) {
         Global_CharacterUpdate()->scheduleBagSave(cKilledBy);

         // todo: signal for new items
      }

      if (bNotEnoughBagspace) {
         printf("TODO: Not enough bagspace\n");

         tc = Global_Server()->getClientFromPool(cKilledBy);
         if (tc != NULL) {
            // ...
         }
      }
   }

   // schedule respawn
   cFor->timeofdeath = GFGetTimestamp();
   Global_RespawnThread()->add(cFor);
}

void CWorld::informAboutAllStats(CCharacter *cFor, CCharacter *cAbout) {
   CTelnetConnection *tc = Global_Server()->getClientFromPool(cFor);
   if (tc != NULL) {
      tc->informAboutAllStats(cAbout);
   }
}

bool CWorld::getQuestStory(long iQuestId, CCharacter *cFor, TGFString *sStory, long *rewards_xp) {
   CQuest *q = this->getQuest(iQuestId);
   if (q != NULL) {
      if (q->id == iQuestId) {
         cFor->pickupQuest(iQuestId);

         sStory->setValue(&(q->story));

         *rewards_xp = q->rewards_xp;

         if (cFor != NULL) {
            sStory->replace_ansi("{player}", cFor->name.get());
            
            TGFBValue v;
            v.setInteger( cFor->level.get() );
            sStory->replace_ansi("{lvl}", v.asString()->getValue());

            if (q->autocomplete == 1) {
               *rewards_xp = this->completeQuest(q, cFor);
            }
         }

         return true;
      }
   }

   return false;
}

bool CWorld::getGreeting(long iCharId, CCharacter *cFor, TGFString *sGreeting) {
   CCharacter *c = this->getCharacter(iCharId);
   if (c != NULL) {
      sGreeting->setValue(c->greeting.link());

      if (cFor != NULL) {
         sGreeting->replace_ansi("{player}", cFor->name.get());
            
         TGFBValue v;
         v.setInteger( cFor->level.get() );
         sGreeting->replace_ansi("{lvl}", v.asString()->getValue());
      }

      return true;
   }

   return false;
}

void CWorld::printf_world_stats(bool preloadThings) {

   if (preloadThings) {
      // do stuff...
   }

   TGFString s;

   s.append_ansi("Number of mapped rooms: %d\n");
   s.append_ansi("Number of players     : %d\n");
   s.append_ansi("Number of NPCs        : %d\n");
   //s.append_ansi("Number of world-ids   : %d\n");
   s.append_ansi("Number of Quests      : %d\n");
   //s.append_ansi("Number of Combats     : %d\n");

   //s.append_ansi("NPC ID (world-id): %d\n");

   printf(
      s.getValue(),
      rooms.size(),
      characters.size(),
      npcs.size(),
      //worldids.size() - 1,
      quests.size() - 1
      //combats.size()
   );
}

