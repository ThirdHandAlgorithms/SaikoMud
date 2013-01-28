
#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include <MySQLBooks/MySQLSquirrel.h>
#include <Groundfloor/Molecules/GFProperty.h>

#include "../combat/Combat.h"

class CCharacter: public CCombatant {
protected:
   TMySQLSquirrelConnection *conn;
   unsigned long id;
   TGFVector quests;

   void load();
   void loadQuests();   // load quests this character can give to players (only when accountid=0 (npc))
public:
   TGFProperty<unsigned int> xp;
   TGFProperty<unsigned int> money;
   TGFProperty<long> x;
   TGFProperty<long> y;

   TGFProperty<bool> isNPC;

   CCharacter( TMySQLSquirrelConnection *pConn, unsigned long id, bool isNPC );
   ~CCharacter();

   void save();

   bool hasDoneQuest(long iQuestId);   // quests player has completed (real player, not npc)
};

#endif //__CHARACTER_H__
