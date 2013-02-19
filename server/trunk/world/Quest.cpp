
#include "../world/Quest.h"

CQuest::CQuest() : TGFFreeable() {
   this->id = 0;
   this->prereq_quest_id = 0;
   this->rewards_xp = 0;
   this->autocomplete = 0;
}

CQuest::~CQuest() {

}

