
#ifndef __SPELLS_H__
#define __SPELLS_H__

#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Molecules/GFProperty.h>

class CSpell: public TGFFreeable {
public:
   TGFStringProperty name;
   TGFProperty<int> spell_id;

   CSpell();
   ~CSpell();

};


#endif // __SPELLS_H__
