
#ifndef __ITEMS_H__
#define __ITEMS_H__

#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Molecules/GFString.h>

#include <MySQLBooks/MySQLSquirrel.h>

class CItem: public TGFFreeable {
public:
   __int64 id;
   TGFString name;
   unsigned int type;
   unsigned long stats_id;
   unsigned int charslot_id;
   bool equipable;
   TGFString description;
   unsigned int use_spell_id;
   unsigned int equip_spell_id;

   bool loadFromRecord(TRemoteSQL *qry);
   bool loadFromDb(TMySQLSquirrelConnection *pConn, __int64 id);
};

class CDropPoolItem: public TGFFreeable {
public:
   __int64 item_id;
   double chance;
};

#endif //__ITEMS_H__
