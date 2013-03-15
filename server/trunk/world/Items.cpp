
#include "Items.h"

#include <MySQLBooks/MySQLSquirrel.h>

bool CItem::loadFromDb(TMySQLSquirrelConnection *pConn, unsigned long id) {
   bool b = false;

   TGFString sql("select * from item where id=:id");
   // todo: join with stats and save to vars
   
   TMySQLSquirrel qrySelect(pConn);
   qrySelect.setQuery(&sql);

   TSquirrelReturnData err;
   if (qrySelect.open(&err)) {
      TGFBFields flds;
      TGFBRecord rec;

      qrySelect.fetchFields(&flds);

      if (qrySelect.fetchRecord(&rec)) {
         this->id = id;
         this->name.setValue( rec.getValue(flds.getFieldIndex_ansi("name"))->asString() );
         this->type = rec.getValue(flds.getFieldIndex_ansi("type"))->asInteger();
         this->stats_id = rec.getValue(flds.getFieldIndex_ansi("stats_id"))->asInteger();
         this->charslot_id = rec.getValue(flds.getFieldIndex_ansi("type"))->asInteger();
         this->equipable = rec.getValue(flds.getFieldIndex_ansi("equipable"))->asBoolean();
         this->description.setValue( rec.getValue(flds.getFieldIndex_ansi("description"))->asString() );
         this->use_spell_id = rec.getValue(flds.getFieldIndex_ansi("use_spell_id"))->asInteger();
         this->equip_spell_id = rec.getValue(flds.getFieldIndex_ansi("equip_spell_id"))->asInteger();

         b = true;
      }

      qrySelect.close();
   } else {
      printf("CItem::loadFromDb - %s\n", err.errorstring.getValue());
   }

   return b;
}
