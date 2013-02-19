
#ifndef __QUEST_H__
#define __QUEST_H__

#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Molecules/GFString.h>

class CQuest: public TGFFreeable {
protected:

public:
   unsigned long id;
   TGFString title;
   TGFString story;
   unsigned long prereq_quest_id;
   long rewards_xp;
   bool autocomplete;
   
   CQuest();
   ~CQuest();
};

#endif //__QUEST_H__
