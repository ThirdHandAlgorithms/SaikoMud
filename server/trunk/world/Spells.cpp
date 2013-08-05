
#include "Spells.h"


CSpell::CSpell(TMySQLSquirrelConnection *pConn, TMySQLSquirrel *pQuery) : TGFFreeable() {
   this->loadFromRecord(pQuery);
}

CSpell::CSpell(TMySQLSquirrelConnection *pConn, unsigned long id) : TGFFreeable() {
   this->id = id;

   if (pConn != NULL) {
      TMySQLSquirrel aQuery(pConn);
      TGFString sQry("select * from spell where id=:id");
      aQuery.setQuery(&sQry);
      aQuery.findOrAddParam("id")->setInteger(id);
      TSquirrelReturnData errData;
      if (aQuery.open(&errData)) {
         if (aQuery.next()) {
            this->loadFromRecord(&aQuery);
         }

         aQuery.close();
      } else {
         printf("CSpell(): %s\n", errData.errorstring);
      }
   }
}

CSpell::~CSpell() {

}

void CSpell::loadFromRecord(TMySQLSquirrel *pQuery) {
   TGFBRecord rec;
   TGFBFields flds;

   pQuery->fetchFields(&flds);
   pQuery->fetchRecord(&rec);

   this->id = rec.getValue(flds.getFieldIndex_ansi("id"))->asInteger();
   this->name.setValue(rec.getValue(flds.getFieldIndex_ansi("name"))->asString());

   this->cooldown = rec.getValue(flds.getFieldIndex_ansi("cooldown"))->asInteger();
   this->casttime = rec.getValue(flds.getFieldIndex_ansi("casttime"))->asInteger();
   this->basedamage = rec.getValue(flds.getFieldIndex_ansi("basedamage"))->asInteger();
}

