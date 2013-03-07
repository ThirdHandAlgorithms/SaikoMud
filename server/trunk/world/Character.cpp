
#include "Character.h"
#include "../world/quest.h"

#include "../world/Items.h"

CCharacter::CCharacter( TMySQLSquirrelConnection *pConn, unsigned long id ) : CCombatant() {
   this->WorldId = 0;

   this->conn = pConn;
   this->id = id;

   this->isNPC = false;

   this->timeofdeath = 0;

   this->greeting.set("");

   this->load();
}

CCharacter::CCharacter( TMySQLSquirrelConnection *pConn, TMySQLSquirrel *pQuery ): CCombatant() {
   this->WorldId = 0;

   this->conn = pConn;
   this->isNPC = false;
   
   this->timeofdeath = 0;

   this->loadFromRecord(pQuery);
}

CCharacter::~CCharacter() {
}

void CCharacter::loadFromRecord(TMySQLSquirrel *pQuery) {
   TGFBRecord rec;
   TGFBFields flds;

   pQuery->fetchFields(&flds);
   pQuery->fetchRecord(&rec);

   this->id = rec.getValue(flds.getFieldIndex_ansi("id"))->asInteger();

   bool bIsNPC = (rec.getValue(flds.getFieldIndex_ansi("account_id"))->asInteger() == 0);
   this->isNPC = bIsNPC;

   this->faction = rec.getValue(flds.getFieldIndex_ansi("faction_id"))->asInteger();

   this->name.internalSetCopy( rec.getValue(flds.getFieldIndex_ansi("name"))->asString() );
   this->level.set( rec.getValue(flds.getFieldIndex_ansi("level"))->asInteger() );
   this->xp.set( rec.getValue(flds.getFieldIndex_ansi("totalxp"))->asInteger() );
   this->money.set( rec.getValue(flds.getFieldIndex_ansi("money"))->asInteger() );
   this->x.set( rec.getValue(flds.getFieldIndex_ansi("x"))->asInteger() );
   this->y.set( rec.getValue(flds.getFieldIndex_ansi("y"))->asInteger() );
   this->currenthealthpool.set( rec.getValue(flds.getFieldIndex_ansi("hp"))->asInteger() );

   this->greeting.internalSetCopy( rec.getValue(flds.getFieldIndex_ansi("greeting"))->asString() );
}

void CCharacter::calculateStats() {
   // reset stats
   this->currentstats.strength.set(0);
   this->currentstats.energy.set(0);
   this->currentstats.protection.set(0);

   // load gear
   TGFString sql(
   "SELECT stats.strength, stats.energy, stats.protection \
   FROM `charslots` \
   left outer join item on (item.id=charslots.item_id) \
   left outer join stats on (item.stats_id=stats.id) \
   where charslots.char_id=:id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("id")->setInteger(this->id);
   
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      TGFBFields flds;
      qry.fetchFields(&flds);
      while ( qry.next() ) {
         TGFBRecord rec;
         qry.fetchRecord(&rec);

         this->currentstats.strength.add(rec.getValue(0)->asInteger());
         this->currentstats.energy.add(rec.getValue(1)->asInteger());
         this->currentstats.protection.add(rec.getValue(2)->asInteger());
      }

      qry.close();
   } else {
      printf("CCharacter::calculateStats(): %s\n", err.errorstring.getValue());
   }

   // apply stat modifiers
}

void CCharacter::load() {
   TGFString sql("select * from `char` where id=:id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("id")->setInteger(this->id);
   
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      if ( qry.next() ) {
         this->loadFromRecord(&qry);
      }

      qry.close();
   } else {
      printf("CCharacter::load(): %s\n", err.errorstring.getValue());
   }
}

void CCharacter::loadBagslots() {
}

bool CCharacter::addToBags(unsigned long iItemId) {
   printf("CCharacter::addToBags(%d) TODO", iItemId);


   // returns false when bags are full
   return false;
}

void CCharacter::loadActionSlots() {
}

void CCharacter::save() {
   TGFString sql("update `char` set totalxp=:totalxp, level=:level, money=:money, x=:x, y=:y, current_worldid=:current_worldid where id=:id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("id")->setInteger(this->id);
   qry.findOrAddParam("totalxp")->setInteger(this->xp.get());
   qry.findOrAddParam("level")->setInteger(this->level.get());
   qry.findOrAddParam("money")->setInteger(this->money.get());
   qry.findOrAddParam("x")->setInteger(this->x.get());
   qry.findOrAddParam("y")->setInteger(this->y.get());
   qry.findOrAddParam("hp")->setInteger(this->currenthealthpool.get());
   qry.findOrAddParam("current_worldid")->setInteger(this->WorldId);

   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      qry.close();
   } else {
      printf("CCharacter::save(): %s\n", err.errorstring.getValue());
   }
}

bool CCharacter::hasDoneQuest(long iQuestId) {
   bool bDoneQuest = false;

   TGFString sql("select * from `questhistory` where char_id=:char_id and quest_id=:quest_id and dt_completed!=0");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("char_id")->setInteger(this->id);
   qry.findOrAddParam("quest_id")->setInteger(iQuestId);
   
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      //TGFBFields flds;
      //qry.fetchFields(&flds);

      //int fld_pickedup = flds.getFieldIndex_ansi("dt_pickedup");
      //int fld_completed = flds.getFieldIndex_ansi("dt_completed");

      if ( qry.next() ) {
         //TGFBRecord rec;
         //qry.fetchRecord(&rec);

         bDoneQuest = true;
      }

      qry.close();
   } else {
      printf("CCharacter::hasDoneQuest(): %s\n", err.errorstring.getValue());
   }

   return bDoneQuest;
}

// ----------------------------------------------------------------

CNPCharacter::CNPCharacter(TMySQLSquirrelConnection *pConn, unsigned long id) : CCharacter(pConn, id) {
   this->quests.autoClear = false;
   this->isNPC = true;

   this->loadDroppool();
}

CNPCharacter::CNPCharacter(TMySQLSquirrelConnection *pConn, TMySQLSquirrel *pQuery) : CCharacter(pConn, pQuery) {
   this->quests.autoClear = false;
   this->isNPC = true;

   this->loadDroppool();
}

CNPCharacter::~CNPCharacter() {
}


void CNPCharacter::_addQuest(CQuest *q) {
   if ( this->quests.findElement(q) == -1 ) {
      this->quests.addElement(q);
   }
}

void CNPCharacter::loadDroppool() {
   TGFString sql("select * from `droppool` where char_id=:char_id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("char_id")->setInteger(this->id);
   
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      TGFBFields flds;
      qry.fetchFields(&flds);

      int chanceind = flds.getFieldIndex_ansi("chance");
      int itemidind = flds.getFieldIndex_ansi("item_id");

      while (qry.next()) {
         TGFBRecord rec;
         qry.fetchRecord(&rec);

         CDropPoolItem *item = new CDropPoolItem();
         item->chance = rec.getValue(chanceind)->asDouble();
         item->item_id = rec.getValue(itemidind)->asInteger();

         this->droppool.addElement(item);
      }
      qry.close();
   }
}

unsigned long CNPCharacter::getRandomItemDrop() {
   double r = (rand() % 10000) / 100.0;

   double chanceinc = 0;

   int c = this->droppool.size();
   for (int i = 0; i < c; i++) {
      CDropPoolItem *item = static_cast<CDropPoolItem *>(this->droppool.elementAt(i));
      if (item->chance < 100) {
         if ((r >= chanceinc) && (r < chanceinc + item->chance)) {
            return item->item_id;
         }

         chanceinc += item->chance;
      }
   }

   return 0;
}

std::vector<unsigned long> CNPCharacter::getPossibleQuestDrops() {
   std::vector<unsigned long> v;

   double chanceinc = 0;

   int c = this->droppool.size();
   for (int i = 0; i < c; i++) {
      CDropPoolItem *item = static_cast<CDropPoolItem *>(this->droppool.elementAt(i));
      if (item->chance >= 100) {
         v.push_back(item->item_id);
      }
   }

   return v;
}

int CNPCharacter::getQuests(CCharacter *cFor, TGFVector *vQuests) {
   int total = 0;
   bool bInclude = false;

   TGFBValue v;

   int c = this->quests.size();
   for (int i = 0; i < c; i++) {
      bInclude = true;

      CQuest *q = static_cast<CQuest *>(this->quests.elementAt(i));
      if (cFor != NULL) {
         bInclude = !cFor->hasDoneQuest(q->id);
      }

      if (bInclude) {
         vQuests->addElement(q);

         total++;
      }
   }

   return total;
}

