
#include "World.h"

#include "Character.h"
#include <Groundfloor/Materials/GFGarbageCollector.h>

#include "../Globals.h"

#include <Groundfloor/Materials/GFFunctions.h>

#include <vector>

CWorld::CWorld() : TGFFreeable() {
   worldids.autoClear = false;
   worldids.resizeVector(1024);
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

void CWorld::echoAsciiMap( TGFString *s, long x, long y, unsigned int radius ) {
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
               } else {
                  line.append_ansi("@");
               }
            } else {
               line.append_ansi("@");
            }
         } else {
            CRoom *room = this->getRoom(j,i);
            if ( room != NULL ) {
               if (this->hasNPCsAt(j,i)) {
                  line.append(static_cast<char>((65 + room->envtype)));
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

CCharacter *CWorld::getCharacter(DWORD32 id) {
   return static_cast<CCharacter *>(worldids.elementAt(id));
}

DWORD32 CWorld::generateUniqueWorldId(CCharacter *c) {
   DWORD32 r;

   worldidlock.lock();
   try  {

      r = (rand() % 0xffff) + ((DWORD32)c % 0x0fff) + 1;

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

CQuest *CWorld::getQuest(DWORD32 id) {
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
   long xp = 0;

   TGFString sql("insert into `questhistory` ( char_id, quest_id, dt_completed) values (:char_id,:quest_id,:dt_completed)");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("char_id")->setInteger(cFor->id);
   qry.findOrAddParam("quest_id")->setInteger(q->id);
   qry.findOrAddParam("dt_completed")->setInt64(GFGetTimestamp());
   if (qry.open()) {
      qry.close();

      xp = q->rewards_xp;
      cFor->xp.lockedAdd(xp);
      
      Global_CharacterUpdate()->schedule(cFor);

      // xp earned message before sql update :S - oh well...
      CTelnetConnection *tc = Global_Server()->getClientFromPool(cFor);
      if (tc != NULL) {
         tc->inform_earnxp(xp, cFor->xp.get());
      }
   } else {
      // do update?
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

      unsigned long drop_item_id = npc->getRandomItemDrop();
      if (drop_item_id != 0) {
         cKilledBy->addToBags(drop_item_id);
      }

      std::vector<unsigned long> questdrops = npc->getPossibleQuestDrops();
      for (int i = 0; i < questdrops.size(); i++) {
         cKilledBy->addToBags(questdrops[i]);
      }
   }

   // schedule teleportation to nearest spawnpoint
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

