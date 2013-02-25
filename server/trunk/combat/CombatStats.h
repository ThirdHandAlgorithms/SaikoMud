
#ifndef __COMBATSTATS_H__
#define __COMBATSTATS_H__

#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Molecules/GFProperty.h>
#include <MySQLBooks/MySQLSquirrel.h>


class CBaseCombatStats: public TGFFreeable {
public:
   CBaseCombatStats();
   ~CBaseCombatStats();

   TGFProperty<int> strength;
   TGFProperty<int> energy;
   TGFProperty<int> protection;

};

class CCombatStats: public CBaseCombatStats {
protected:
   TMySQLSquirrelConnection *conn;
   unsigned long id;

   void load();

public:
   CCombatStats( TMySQLSquirrelConnection *pConn, unsigned long id );
   CCombatStats();
   ~CCombatStats();

   void save();
};

#endif //__COMBATSTATS_H__
