
#include "Items.h"

#include <MySQLBooks/MySQLSquirrel.h>

bool CItem::loadFromRecord(TRemoteSQL *qry) {
   TGFBFields flds;
   TGFBRecord rec;

   qry->fetchFields(&flds);
   qry->fetchRecord(&rec);

   this->id = rec.getValue(flds.getFieldIndex_ansi("id"))->asInteger();
   this->name.setValue( rec.getValue(flds.getFieldIndex_ansi("name"))->asString() );
   this->type = rec.getValue(flds.getFieldIndex_ansi("type"))->asInteger();
   this->stats_id = rec.getValue(flds.getFieldIndex_ansi("stats_id"))->asInteger();
   this->charslot_id = rec.getValue(flds.getFieldIndex_ansi("charslot_id"))->asInteger();
   this->equipable = (this->charslot_id > 0);
   this->description.setValue( rec.getValue(flds.getFieldIndex_ansi("description"))->asString() );
   this->use_spell_id = rec.getValue(flds.getFieldIndex_ansi("use_spell_id"))->asInteger();
   this->equip_spell_id = rec.getValue(flds.getFieldIndex_ansi("equip_spell_id"))->asInteger();

   return true;
}

bool CItem::loadFromDb(TMySQLSquirrelConnection *pConn, __int64 id) {
   bool b = false;

   TGFString sql("select * from item where id=:id");
   // todo: join with stats and save to vars
   
   TMySQLSquirrel qrySelect(pConn);
   qrySelect.setQuery(&sql);
   qrySelect.findOrAddParam("id")->setInt64(id);

   TSquirrelReturnData err;
   if (qrySelect.open(&err)) {
      TGFBFields flds;
      TGFBRecord rec;

      qrySelect.fetchFields(&flds);
      if (qrySelect.next()) {
         this->loadFromRecord(&qrySelect);

         b = true;
      }

      qrySelect.close();
   } else {
      printf("CItem::loadFromDb - %s\n", err.errorstring.getValue());
   }

   return b;
}
