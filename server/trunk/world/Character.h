
#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include <MySQLBooks/MySQLSquirrel.h>
#include <Groundfloor/Molecules/GFProperty.h>

#include <Groundfloor/Molecules/GFVector.h>
#include <Groundfloor/Molecules/GFStringVector.h>

#include <Groundfloor/Atoms/GFLockable.h>

#include "Quest.h"
#include "../combat/Combat.h"

#include "Items.h"

#include <vector>

class CCharacter: public CCombatant {
protected:
   TMySQLSquirrelConnection *conn;

   void load();

   void calculateStats();

   TGFLockable baglock;
   std::vector<unsigned long> bagslots;

   TGFLockable spellslock;
   std::vector<unsigned long> spells;

   void loadSpells();
   void loadBagslots();

   bool updateItemIntoSlot(int iSlotId, __int64 iItemId);

   void loadFromRecord(TMySQLSquirrel *pQuery);
public:
   unsigned long id;
   unsigned long respawntime;
   __int64 timeofdeath;
   bool isNPC;
   int faction;

   TGFLockableProperty<unsigned long> xp;
   TGFLockableProperty<unsigned long> money;
   TGFLockableProperty<long> x;
   TGFLockableProperty<long> y;
   TGFLockableProperty<unsigned int> maxbagslots;

   TGFStringProperty greeting;

   CCharacter(TMySQLSquirrelConnection *pConn, unsigned long id);
   CCharacter(TMySQLSquirrelConnection *pConn, TMySQLSquirrel *pQuery);
   ~CCharacter();

   void save();
   void saveBagslots();
   void saveSpells();

   bool hasItemInBags(unsigned long iItemId, unsigned int iAmount);
   bool addToBags(unsigned long iItemId);
   bool takeFromBags(unsigned long iItemId);

   bool equipItem(const CItem *item);

   std::vector<unsigned long> getBagSlots() const;
   std::vector<unsigned long> getSpells() const;

   void setSpell(unsigned long iSpellId, int iHotkey);
   bool canPerformSpell(unsigned iSpellId);

   // player specific functions
   bool pickupQuest(long iQuestId);

   bool hasDoneQuest(long iQuestId);   // quests player has completed (real player, not npc)
   bool isOnQuest(long iQuestId);

   bool completeQuest(long iQuestId, long iEarnXp, unsigned long iEarnItem, unsigned long iEarnSpell);
   
   int getItemsInSlots(TGFVector *v);  // must free items in v
};

class CNPCharacter: public CCharacter {
protected:
   TGFVector quests;
   TGFVector droppool;

   void loadDroppool();
public:
   CNPCharacter(TMySQLSquirrelConnection *pConn, unsigned long id);
   CNPCharacter(TMySQLSquirrelConnection *pConn, TMySQLSquirrel *pQuery);
   ~CNPCharacter();

   // npc specific functions
   void _addQuest(CQuest *q);
   int getQuests(CCharacter *cFor, TGFVector *vQuests);

   unsigned long getRandomItemDrop();
   std::vector<unsigned long> getPossibleQuestDrops();
};

#endif //__CHARACTER_H__
