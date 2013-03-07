
#ifndef __ITEMS_H__
#define __ITEMS_H__

#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Molecules/GFString.h>

class CItem: public TGFFreeable {
public:
   unsigned long id;
   TGFString name;
   unsigned int type;
   unsigned long stats_id;
   unsigned int charslot_id;
   bool equipable;
   TGFString description;
   unsigned int use_spell_id;
   unsigned int equip_spell_id;
};

class CDropPoolItem: public TGFFreeable {
public:
   unsigned long item_id;
   double chance;
};

#endif //__ITEMS_H__
