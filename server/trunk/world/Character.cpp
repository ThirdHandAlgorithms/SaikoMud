
#include "Character.h"
#include "../world/Quest.h"

#include "../world/Items.h"

#include <Groundfloor/Materials/GFFunctions.h>

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
   this->respawntime = 0;

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

   this->respawntime = rec.getValue(flds.getFieldIndex_ansi("respawntime"))->asInteger();
   if (!this->isNPC)  {
      this->respawntime = 5;
   }

   this->maxbagslots.set( rec.getValue(flds.getFieldIndex_ansi("maxbagslots"))->asInteger() );

   this->loadBagslots();
   this->loadSpells();

   this->setSpell(1, 5);
}

void CCharacter::calculateStats() {
   // reset stats
   int lvl = this->level.get();

   this->currentstats.strength.set(lvl);
   this->currentstats.energy.set(lvl);
   this->currentstats.protection.set(lvl);

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

int CCharacter::getItemsInSlots(TGFVector *v) {
   int c = 0;

   v->clear();
   v->resizeVector(5);
   v->setElementCount(5);

   TGFString sql(
      "SELECT item.* \
      FROM charslots \
      LEFT OUTER JOIN item ON (charslots.item_id=item.id) \
      WHERE charslots.char_id=:charid \
      ORDER BY item.charslot_id ASC");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("charid")->setInteger(this->id);
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      TGFBRecord rec;
      while ( qry.next() ) {
         // todo: get item from Global_World() caching functions
         CItem *item = new CItem();
         item->loadFromRecord(&qry);

         if (item->charslot_id > 0) {
            // index 0..4 equal to slot id -1
            v->replaceElement(item->charslot_id - 1, item);
         } else {
            printf("CCharacter::getItemsInSlots(): '%s' equiped item '%d' that can't be equiped", this->name.get(), item->id);

            delete item;
         }

         c++;
      }
   } else {
      printf("CCharacter::getItemsInSlots(): %s\n", err.errorstring.getValue());
   }

   return c;
}

bool CCharacter::updateItemIntoSlot(int iSlotId, __int64 iItemId) {
   bool bDoUpdate = false;

   TGFString sql(
      "INSERT INTO charslots ( char_id, slot_id, item_id) VALUES (:char_id,:slot_id,:item_id)"
   );

   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("item_id")->setInteger(iItemId);
   qry.findOrAddParam("char_id")->setInteger(this->id);
   qry.findOrAddParam("slot_id")->setInteger(iSlotId);
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      qry.close();

      return true;
   } else {
      if (err.errorstring.pos_ansi(0, "Duplicate entry") != -1) {
         bDoUpdate = true;
      } else {
         printf("CCharacter::updateItemIntoSlot(): %s (%s)\n", err.errorstring.getValue(), sql.getValue());
      }
   }

   if (bDoUpdate) {   // affected doesn't work for Update queries???
      sql.setValue_ansi("UPDATE charslots SET item_id=:item_id WHERE char_id=:char_id AND slot_id=:slot_id");
      qry.setQuery(&sql);
         
      qry.findOrAddParam("item_id")->setInteger(iItemId);
      qry.findOrAddParam("char_id")->setInteger(this->id);
      qry.findOrAddParam("slot_id")->setInteger(iSlotId);

      if ( qry.open(&err) ) {
         qry.close();

         return true;
      } else {
         printf("CCharacter::updateItemIntoSlot(): %s (%s)\n", err.errorstring.getValue(), sql.getValue());
      }
   }

   return false;
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

void CCharacter::loadSpells() {
   if (spellslock.lockWhenAvailable()) {
      spells.clear();

      TGFString sql("select spell_id, hotkey from `char_spell` where char_id=:id order by hotkey asc");
      TMySQLSquirrel qry(this->conn);
      qry.setQuery(&sql);
      qry.findOrAddParam("id")->setInteger(this->id);
   
      TSquirrelReturnData err;
      if ( qry.open(&err) ) {
         TGFBRecord rec;
         while ( qry.next() ) {
            qry.fetchRecord(&rec);

            int c = rec.getValue(1)->asInteger();
            while (spells.size() < c) {
               // not a spell, but hack to match hotkey position
               spells.push_back(0);
            }

            // add spell_id
            spells.push_back( rec.getValue(0)->asInteger() );
         }

         qry.close();
      } else {
         printf("CCharacter::loadSpells(): %s\n", err.errorstring.getValue());
      }

      spellslock.unlock();
   }
}

void CCharacter::setSpell(unsigned long iSpellId, int iHotkey) {
   if (spellslock.lockWhenAvailable()) {
      while (spells.size() < iHotkey + 1) {
         spells.push_back(0);
      }

      int i = 0;
      for (std::vector<unsigned long>::iterator it = spells.begin(); it != spells.end(); ++it) {
         if (iHotkey == i) {
            *it = iSpellId;
         } else {
            if (iSpellId == *it) {
               *it = 0;
            }
         }
         i++;
      }

      // todo: save to database

      spellslock.unlock();
   }
}

void CCharacter::loadBagslots() {
   if (baglock.lockWhenAvailable()) {
      bagslots.clear();

      TGFString sql("select item_id, stacksize from `bagslot` where char_id=:id");
      TMySQLSquirrel qry(this->conn);
      qry.setQuery(&sql);
      qry.findOrAddParam("id")->setInteger(this->id);
   
      TSquirrelReturnData err;
      if ( qry.open(&err) ) {
         TGFBRecord rec;
         if ( qry.next() ) {
            qry.fetchRecord(&rec);
            
            long c = rec.getValue(1)->asInteger();

            for (long i = 0; i < c; i++) {
               //if (bagslots.size() < this->maxbagslots.get()) { // should be correct in database.....
               bagslots.push_back( rec.getValue(0)->asInteger() );
               //}
            }
         }

         qry.close();
      } else {
         printf("CCharacter::loadBagslots(): %s\n", err.errorstring.getValue());
      }

      baglock.unlock();
   }
}

void CCharacter::saveBagslots() {
   if (baglock.lockWhenAvailable()) {
      TGFString sql1("delete from bagslot where char_id=:char_id");     // <-- causes that we always start with an insert that succeeds if we have multiple items of the same item_id
      TMySQLSquirrel qry(this->conn);
      qry.setQuery(&sql1);
      qry.findOrAddParam("char_id")->setInteger(this->id);
   
      TSquirrelReturnData err;
      if ( qry.open(&err) ) {
         qry.close();
      } else {
         printf("CCharacter::saveBagslots() - delete: %s\n", err.errorstring.getValue());
      }

      TGFString sql2("insert into bagslot ( char_id, item_id, stacksize) values (:char_id,:item_id, 1)");

      unsigned int items_added = 0;

      for (std::vector<unsigned long>::iterator it = bagslots.begin(); it != bagslots.end(); ++it) {
         if (items_added >= this->maxbagslots.get()) {
            // something went wrong with some other piece of code.......
            break;
         }

         qry.setQuery(&sql2);
         qry.findOrAddParam("char_id")->setInteger(this->id);
         qry.findOrAddParam("item_id")->setInteger(*it);

         if (qry.open(&err)) {
            qry.close();
         } else {
            // if duplicate, increase stacksize <- only for DB, still multiple items in bags, so maxbagslots applies
            if (err.errorstring.pos_ansi("Duplicate entry") != -1) {
               TGFString sql3("update bagslot set stacksize=stacksize+1 where char_id=:char_id and item_id=:item_id");
               qry.setQuery(&sql3);
               qry.findOrAddParam("char_id")->setInteger(this->id);
               qry.findOrAddParam("item_id")->setInteger(*it);
               TSquirrelReturnData err2;
               if (qry.open(&err2)) {
                  qry.close();
               } else {
                  printf("CCharacter::saveBagslots() - update: %s\n", err2.errorstring.getValue());
               }
            } else {
               printf("CCharacter::saveBagslots() - insert: %s\n", err.errorstring.getValue());
            }
         }

         items_added++;
      }

      baglock.unlock();
   }
}

bool CCharacter::hasItemInBags(unsigned long iItemId, unsigned int iAmount) {
   bool iCount = 0;

   if (baglock.lockWhenAvailable()) {
      for (std::vector<unsigned long>::iterator it = bagslots.begin(); it != bagslots.end(); ++it) {
          if (*it == iItemId) {
             iCount++;
          }
      }

      baglock.unlock();
   }

   return (iCount == iAmount);
}

bool CCharacter::addToBags(unsigned long iItemId) {
   bool b = false;

   if (baglock.lockWhenAvailable()) {
      if (bagslots.size() < this->maxbagslots.get()) {
         bagslots.push_back(iItemId);
         b = true;
      }

      baglock.unlock();
   }

   // returns false when bags are full
   return b;
}

bool CCharacter::takeFromBags(unsigned long iItemId) {
   bool b = false;   // return false when item is not in bags

   if (baglock.lockWhenAvailable()) {

      for (std::vector<unsigned long>::iterator it = bagslots.begin(); it != bagslots.end(); ++it) {
          if (*it == iItemId) {
             bagslots.erase(it);

             b = true;

             break;
          }
      }

      baglock.unlock();
   }

   return b;
}

bool CCharacter::equipItem(const CItem *item) {
   bool b = false;   // return false when item is not in bags

   __int64 iItemId = item->id;
   TGFVector v;

   // determine destination slot
   unsigned int iSlotId = item->charslot_id;
   if (iSlotId > 0) {
      // all protected by appropriate mutexes
      if (baglock.lockWhenAvailable()) {

         for (std::vector<unsigned long>::iterator it = bagslots.begin(); it != bagslots.end(); ++it) {
            // check if item is in bags
             if (*it == iItemId) {
                // check if slot is occupied
                int c = this->getItemsInSlots(&v);

                CItem *slotitem = static_cast<CItem *>(v.elementAt(iSlotId - 1));
                if (slotitem == NULL) {
                   if (updateItemIntoSlot(iSlotId, iItemId)) {
                      // if not, place item in slot, remove from bags
                      bagslots.erase(it);

                      b = true;
                   }
                } else {
                   if (updateItemIntoSlot(iSlotId, iItemId)) {
                      // if it is, replace bag slot with item in item slot
                      *it = slotitem->id;

                      b = true;
                   }
                }

                break;
             }
         }

         // save to db
         this->saveBagslots();

         baglock.unlock();
      }
   }

   return b;
}

std::vector<unsigned long> CCharacter::getSpells() const {
   return this->spells;
}

std::vector<unsigned long> CCharacter::getBagSlots() const {
   return this->bagslots;
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

   TGFString sql("select dt_completed from `questhistory` where char_id=:char_id and quest_id=:quest_id and dt_completed!=0");
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

bool CCharacter::isOnQuest(long iQuestId) {
   bool bIsOnQuest = false;

   TGFString sql("select dt_pickedup from `questhistory` where char_id=:char_id and quest_id=:quest_id and dt_completed=0");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("char_id")->setInteger(this->id);
   qry.findOrAddParam("quest_id")->setInteger(iQuestId);
   
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      if ( qry.next() ) {
         bIsOnQuest = true;
      }

      qry.close();
   } else {
      printf("CCharacter::isOnQuest(): %s\n", err.errorstring.getValue());
   }

   return bIsOnQuest;
}

bool CCharacter::pickupQuest(long iQuestId) {
   // should give duplicate error if quest was already picked up somehow...
   TGFString sql("insert into `questhistory` ( char_id, quest_id, dt_pickedup) values (:char_id,:quest_id,:dt_pickedup)");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("char_id")->setInteger(this->id);
   qry.findOrAddParam("quest_id")->setInteger(iQuestId);
   qry.findOrAddParam("dt_pickedup")->setInt64( GFGetTimestamp() );
   
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      qry.close();
   } else {
      if (err.errorstring.pos_ansi("Duplicate entry") == -1) {
         printf("CCharacter::pickupQuest(): %s\n", err.errorstring.getValue());
      }

      return false;
   }

   return true;
}

bool CCharacter::completeQuest(long iQuestId, long iEarnXp) {
   TGFString sql("update `questhistory` set dt_completed=:dt_completed where char_id=:char_id and quest_id=:quest_id and dt_completed=0");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("char_id")->setInteger(this->id);
   qry.findOrAddParam("quest_id")->setInteger(iQuestId);
   qry.findOrAddParam("dt_completed")->setInt64(GFGetTimestamp());

   TSquirrelReturnData err;
   if (qry.open(&err)) {
      qry.close();

      if (err.affected == 1) {   // this way we don't have to lock the function with a mutex and do selects..
         this->xp.lockedAdd(iEarnXp);

         return true;
      }
   }

   return false;
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

