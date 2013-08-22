
#include "../world/Quest.h"

#include <MySQLBooks/MySQLSquirrel.h>

#include "../Globals.h"

CQuest::CQuest() : TGFFreeable() {
   this->id = 0;
   this->prereq_quest_id = 0;
   this->rewards_xp = 0;
   this->autocomplete = 0;
   this->rewards_item = 0;
   this->rewards_spell = 0;
}

CQuest::~CQuest() {

}


CQuestItemRequired make_itemreq(uint32_t item_id, uint32_t amountrequired) {
    CQuestItemRequired req = {item_id, amountrequired};
    return req;
}

std::vector<CQuestItemRequired> CQuest::getRequiredItems() const {
   std::vector<CQuestItemRequired> arr;

   TGFString sql("select item_id, amountrequired from quest_item where quest_id=:quest_id");

   TMySQLSquirrel qry(Global_DBConnection());
   qry.setQuery(&sql);
   qry.findOrAddParam("quest_id")->setInteger(this->id);

   TSquirrelReturnData err;
   if (qry.open(&err)) {
      //TGFBFields flds;
      TGFBRecord rec;
      //qry.fetchFields(&flds);
      
      if (qry.next()){
         qry.fetchRecord(&rec);
         __int64 itemid = rec.getValue(0)->asInt64();
         unsigned long amountrequired = rec.getValue(1)->asInteger();

         arr.push_back( make_itemreq(itemid, amountrequired) );
      }
      qry.close();
   } else {
      printf("CQuest::getRequiredItems() - %s\n", err.errorstring.getValue());
   }

   return arr;
}
