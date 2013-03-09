
#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include <MySQLBooks/MySQLSquirrel.h>
#include <Groundfloor/Molecules/GFProperty.h>

#include <Groundfloor/Molecules/GFVector.h>
#include <Groundfloor/Molecules/GFStringVector.h>

#include <Groundfloor/Atoms/GFLockable.h>

#include "Quest.h"
#include "../combat/Combat.h"

#include <vector>

class CCharacter: public CCombatant {
protected:
   TMySQLSquirrelConnection *conn;

   void load();

   void calculateStats();

   TGFLockable baglock;

   std::vector<unsigned long> bagslots;
   TGFVector actionslots;

   void loadBagslots();
   void saveBagslots();
   void loadActionSlots();

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

   bool addToBags(unsigned long iItemId);
   bool takeFromBags(unsigned long iItemId);

   // player specific functions
   bool hasDoneQuest(long iQuestId);   // quests player has completed (real player, not npc)
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
