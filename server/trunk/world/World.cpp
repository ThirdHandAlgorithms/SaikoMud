
#include "World.h"

#include "Character.h"
#include <Groundfloor/Materials/GFGarbageCollector.h>

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

   TGFString sql("select min(x), min(y), max(x), max(y), count(x) from grid where active=1");
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

   sql.setValue_ansi("select * from grid where active=1 order by y asc, x asc");
   qry.setQuery(&sql);
   TSquirrelReturnData errData;
   if ( qry.open(&errData) ) {
      TGFBFields flds;
      qry.fetchFields(&flds);

      int xind = flds.getFieldIndex_ansi("x");
      int yind = flds.getFieldIndex_ansi("y");
      int descind = flds.getFieldIndex_ansi("desc");
      int envtypeind = flds.getFieldIndex_ansi("envtype");

      while ( qry.next() ) {
         qry.fetchRecord(&rec);

         CRoom *room = new CRoom();

         // this was previously a function within CRoom, inlining for speed
         room->x.internalSet( rec.getValue(xind)->asInteger() );
         room->y.internalSet( rec.getValue(yind)->asInteger() );

         room->envtype.internalSet( static_cast<BYTE>(rec.getValue(envtypeind)->asInteger()) );

         room->description.internalSetCopy( rec.getValue(descind)->asString() );
         // end

         long x = room->x.get();
         long y = room->y.get();

         rooms.insertAt( (y + y_fix) * w + x + x_fix, room );
      }

      qry.close();
   } else {
      printf("Error %d: %s\n", errData.errorcode, errData.errorstring.getValue());
   }

   this->reloadNpcs();
}

void CWorld::reloadNpcs() {
   npcs.clear();

   TMySQLSquirrel qry( this->conn );
   TGFBRecord rec;

   CCharacter *c;

   TSquirrelReturnData errData;

   TGFString sql("select * from `char` where account_id=0");
   qry.setQuery(&sql);
   if ( qry.open(&errData) ) {
      while ( qry.next() ) {
         qry.fetchRecord(&rec);

         c = new CCharacter(this->conn, &qry);
         generateUniqueWorldId(c);
         this->loadNeededQuests(c);
         this->npcs.addElement(c);
      }

      qry.close();
   } else {
      printf("Error %d: %s\n", errData.errorcode, errData.errorstring.getValue());
   }
}

CCharacter *CWorld::getNpcByName(TGFString *s) {
   CCharacter *obj;
   unsigned long c = this->npcs.size();
   for (unsigned long i = 0; i < c; i++) {
      obj = static_cast<CCharacter *>( this->npcs.elementAt(i) );
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
         if ((obj->x.get() == x) && (obj->y.get() == y)) {
            return true;
         }
      }
   }

   return false;
}

void CWorld::preloadInteriors( long x, long y ) {
   CRoom *r;

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
               if (this->hasNPCsAt(i,j)) {
                  line.append(static_cast<char>(97 + room->envtype.get()));
               } else {
                  line.append_ansi("@");
               }
            } else {
               line.append_ansi("@");
            }
         } else {
            CRoom *room = this->getRoom(j,i);
            if ( room != NULL ) {
               if (this->hasNPCsAt(i,j)) {
                  line.append(static_cast<char>((65 + room->envtype.get())));
               } else {
                  line.append(static_cast<char>((48 + room->envtype.get())));
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

      if (!c->isNPC.get()) {
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

void CWorld::loadNeededQuests(CCharacter *cNpc) {
   TGFString sql("select `quest`.* from `npc_quest` left outer join `quest` on (`quest`.`id`=`npc_quest`.`quest_id`) where char_id=:char_id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("char_id")->setInteger(cNpc->id.get());
   
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      TGFBFields flds;
      qry.fetchFields(&flds);

      int fld_id = flds.getFieldIndex_ansi("id");
      int fld_title = flds.getFieldIndex_ansi("title");
      int fld_story = flds.getFieldIndex_ansi("story");
      int fld_prereq = flds.getFieldIndex_ansi("prereq_quest_id");

      while ( qry.next() ) {
         TGFBRecord rec;
         qry.fetchRecord(&rec);

         CQuest *quest = new CQuest();
         quest->id = rec.getValue(fld_id)->asInteger();
         quest->prereq_quest_id = rec.getValue(fld_prereq)->asInteger();
         quest->title.setValue( rec.getValue(fld_title)->asString() );
         quest->story.setValue( rec.getValue(fld_story)->asString() );

         if (this->quests.size() < quest->id) {
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

bool CWorld::getQuestStory(long iQuestId, CCharacter *cFor, TGFString *sStory) {
   CQuest *q = this->getQuest(iQuestId);
   if (q != NULL) {
      if (q->id == iQuestId) {
         sStory->setValue(&(q->story));

         if (cFor != NULL) {
            sStory->replace_ansi("{player}", cFor->name.get());
            
            TGFBValue v;
            v.setInteger( cFor->level.get() );
            sStory->replace_ansi("{lvl}", v.asString()->getValue());
         }

         return true;
      }
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

   s.append_ansi("NPC ID (world-id): %d\n");

   CCharacter *c = this->getNpcByName(GFDisposableStr("npc_mister_a"));

   printf(
      s.getValue(),
      rooms.size(),
      characters.size(),
      npcs.size(),
      //worldids.size() - 1,
      quests.size() - 1,
      c->WorldId
      //combats.size()
   );
}
