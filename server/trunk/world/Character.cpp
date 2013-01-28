
#include "Character.h"
#include "../world/quest.h"

CCharacter::CCharacter( TMySQLSquirrelConnection *pConn, unsigned long id, bool isNPC ) : CCombatant() {
   this->conn = pConn;
   this->id = id;

   this->isNPC.internalSet(isNPC);

   this->load();

   if (isNPC) {
      this->loadQuests();
   }
}

CCharacter::~CCharacter() {
}

void CCharacter::loadQuests() {
   this->quests.clear();

   TGFString sql("select `quest`.* from `npc_quest` left outer join `quest` on (`quest`.`id`=`npc_quest`.`quest_id`) where char_id=:char_id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("char_id")->setInteger(this->id);
   
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

         this->quests.addElement(quest);
      }

      qry.close();
   } else {
      printf("CCharacter::loadQuests(): %s\n", err.errorstring.getValue());
   }
}

void CCharacter::load() {
   TGFString sql("select * from `char` where id=:id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("id")->setInteger(id);
   
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
      }

      qry.close();
   } else {
      printf("CCharacter::load(): %s\n", err.errorstring.getValue());
   }
}

void CCharacter::save() {
   TGFString sql("update `char` set totalxp=:totalxp, level=:level, money=:money, x=:x, y=:y where id=:id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("id")->setInteger(this->id);
   qry.findOrAddParam("totalxp")->setInteger(this->xp.get());
   qry.findOrAddParam("level")->setInteger(this->level.get());
   qry.findOrAddParam("money")->setInteger(this->money.get());
   qry.findOrAddParam("x")->setInteger(this->x.get());
   qry.findOrAddParam("y")->setInteger(this->y.get());
   qry.findOrAddParam("hp")->setInteger(this->currenthealthpool.get());
   
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      qry.close();
   } else {
      printf("CCharacter::save(): %s\n", err.errorstring.getValue());
   }
}

//------------------------


