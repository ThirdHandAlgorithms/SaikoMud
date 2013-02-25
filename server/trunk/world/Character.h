
#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include <MySQLBooks/MySQLSquirrel.h>
#include <Groundfloor/Molecules/GFProperty.h>

#include <Groundfloor/Molecules/GFVector.h>
#include <Groundfloor/Molecules/GFStringVector.h>

#include "Quest.h"
#include "../combat/Combat.h"

class CCharacter: public CCombatant {
protected:
   TMySQLSquirrelConnection *conn;
   TGFVector quests;

   void load();

   void calculateStats();

public:
   DWORD32 WorldId;
 
   TGFProperty<unsigned long> id;
   TGFLockableProperty<unsigned long> xp;
   TGFLockableProperty<unsigned long> money;
   TGFLockableProperty<long> x;
   TGFLockableProperty<long> y;

   TGFProperty<bool> isNPC;

   TGFStringProperty greeting;

   CCharacter( TMySQLSquirrelConnection *pConn, unsigned long id, bool isNPC );
   CCharacter( TMySQLSquirrelConnection *pConn, TMySQLSquirrel *pQuery );
   ~CCharacter();

   void _addQuest(CQuest *q);

   int getQuests(CCharacter *cFor, TGFVector *vQuests);

   void save();

   bool hasDoneQuest(long iQuestId);   // quests player has completed (real player, not npc)
};

#endif //__CHARACTER_H__
