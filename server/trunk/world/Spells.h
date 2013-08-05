
#ifndef __SPELLS_H__
#define __SPELLS_H__

#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Molecules/GFProperty.h>
#include <MySQLBooks/MySQLSquirrel.h>

class CSpell: public TGFFreeable {
protected:
   unsigned long id;

   void loadFromRecord(TMySQLSquirrel *pQuery);
public:
   TGFString name;

   int cooldown;   // minimum time between spells (in ms)
   int casttime;   // time it takes for the spell to actually hit the target (in ms)
   int basedamage;   // negative damage heals "any" target
   //int manarequired;  // we don't have mana yet
   
   CSpell(TMySQLSquirrelConnection *pConn, unsigned long id);
   CSpell(TMySQLSquirrelConnection *pConn, TMySQLSquirrel *pQuery);
   ~CSpell();
};


#endif // __SPELLS_H__
