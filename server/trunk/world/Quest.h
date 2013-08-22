
#ifndef __QUEST_H__
#define __QUEST_H__

#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Molecules/GFString.h>

#include <stdint.h>
#include <vector>

struct CQuestItemRequired {
   uint32_t item_id;
   uint32_t amountrequired;
};

class CQuest: public TGFFreeable {
protected:

public:
   unsigned long id;
   TGFString title;
   TGFString story;
   unsigned long prereq_quest_id;
   long rewards_xp;
   bool autocomplete;
   unsigned long rewards_item;
   unsigned long rewards_spell;
   
   CQuest();
   ~CQuest();

   std::vector<CQuestItemRequired> getRequiredItems() const;
};

#endif //__QUEST_H__
