
#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include <MySQLBooks/MySQLSquirrel.h>
#include <Groundfloor/Molecules/GFProperty.h>

#include <Groundfloor/Molecules/GFVector.h>
#include <Groundfloor/Molecules/GFStringVector.h>

#include "Quest.h"
#include "../combat/Combat.h"

#include <vector>

class CCharacter: public CCombatant {
protected:
   TMySQLSquirrelConnection *conn;

   void load();

   void calculateStats();

   TGFVector bagslots;

   TGFVector actionslots;

   void loadBagslots();
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

   TGFStringProperty greeting;

   CCharacter(TMySQLSquirrelConnection *pConn, unsigned long id);
   CCharacter(TMySQLSquirrelConnection *pConn, TMySQLSquirrel *pQuery);
   ~CCharacter();

   void save();

   bool addToBags(unsigned long iItemId);

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
