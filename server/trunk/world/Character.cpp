
#include "Character.h"
#include "../world/quest.h"

CCharacter::CCharacter( TMySQLSquirrelConnection *pConn, unsigned long id, bool isNPC ) : CCombatant() {
   WorldId = 0;

   this->conn = pConn;
   this->id.internalSet(id);

   this->isNPC.internalSet(isNPC);

   this->quests.autoClear = false;

   this->greeting.set("");

   this->load();
}

CCharacter::CCharacter( TMySQLSquirrelConnection *pConn, TMySQLSquirrel *pQuery ): CCombatant() {
   WorldId = 0;

   this->conn = pConn;
   
   this->quests.autoClear = false;

   TGFBRecord rec;
   TGFBFields flds;

   pQuery->fetchFields(&flds);
   pQuery->fetchRecord(&rec);

   this->id.internalSet( rec.getValue(flds.getFieldIndex_ansi("id"))->asInteger() );

   bool bIsNPC = (rec.getValue(flds.getFieldIndex_ansi("account_id"))->asInteger() == 0);
   this->isNPC.internalSet(bIsNPC);

   this->name.internalSetCopy( rec.getValue(flds.getFieldIndex_ansi("name"))->asString() );
   this->level.set( rec.getValue(flds.getFieldIndex_ansi("level"))->asInteger() );
   this->xp.set( rec.getValue(flds.getFieldIndex_ansi("totalxp"))->asInteger() );
   this->money.set( rec.getValue(flds.getFieldIndex_ansi("money"))->asInteger() );
   this->x.set( rec.getValue(flds.getFieldIndex_ansi("x"))->asInteger() );
   this->y.set( rec.getValue(flds.getFieldIndex_ansi("y"))->asInteger() );
   this->currenthealthpool.set( rec.getValue(flds.getFieldIndex_ansi("hp"))->asInteger() );

   this->greeting.internalSetCopy( rec.getValue(flds.getFieldIndex_ansi("greeting"))->asString() );
}

CCharacter::~CCharacter() {
}

void CCharacter::calculateStats() {
   // reset stats
   this->currentstats.strength.set(0);
   this->currentstats.energy.set(0);
   this->currentstats.protection.set(0);

   // load gear
   long iCharId = this->id.internalGet();

   TGFString sql(
   "SELECT stats.strength, stats.energy, stats.protection \
   FROM `charslots` \
   left outer join item on (item.id=charslots.item_id) \
   left outer join stats on (item.stats_id=stats.id) \
   where charslots.char_id=:id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("id")->setInteger(iCharId);
   
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

void CCharacter::_addQuest(CQuest *q) {
   if ( this->quests.findElement(q) == -1 ) {
      this->quests.addElement(q);
   }
}

void CCharacter::load() {
   long iCharId = this->id.internalGet();

   TGFString sql("select * from `char` where id=:id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("id")->setInteger(iCharId);
   
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      TGFBFields flds;
      qry.fetchFields(&flds);
      if ( qry.next() ) {
         TGFBRecord rec;
         qry.fetchRecord(&rec);

         this->name.internalSetCopy( rec.getValue(flds.getFieldIndex_ansi("name"))->asString() );
         this->level.set( rec.getValue(flds.getFieldIndex_ansi("level"))->asInteger() );
         this->xp.set( rec.getValue(flds.getFieldIndex_ansi("totalxp"))->asInteger() );
         this->money.set( rec.getValue(flds.getFieldIndex_ansi("money"))->asInteger() );
         this->x.set( rec.getValue(flds.getFieldIndex_ansi("x"))->asInteger() );
         this->y.set( rec.getValue(flds.getFieldIndex_ansi("y"))->asInteger() );
         this->currenthealthpool.set( rec.getValue(flds.getFieldIndex_ansi("hp"))->asInteger() );

         this->greeting.internalSetCopy( rec.getValue(flds.getFieldIndex_ansi("greeting"))->asString() );

         // wordt intern bijgehouden
         //this->WorldId = rec.getValue(flds.getFieldIndex_ansi("current_worldid"))->asInteger();
      }

      qry.close();
   } else {
      printf("CCharacter::load(): %s\n", err.errorstring.getValue());
   }
}

void CCharacter::save() {
   TGFString sql("update `char` set totalxp=:totalxp, level=:level, money=:money, x=:x, y=:y, current_worldid=:current_worldid where id=:id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("id")->setInteger(this->id.internalGet());
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

   TGFString sql("select * from `questhistory` where char_id=:char_id and quest_id=:quest_id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("char_id")->setInteger(this->id.internalGet());
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

int CCharacter::getQuests(CCharacter *cFor, TGFVector *vQuests) {
   int total = 0;
   bool bInclude = false;
   char crlf[] = GFWIN32NEXTLINE;

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

