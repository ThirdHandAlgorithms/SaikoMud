
#include "../world/Quest.h"

CQuest::CQuest() : TGFFreeable() {
   this->id = 0;
   this->prereq_quest_id = 0;
}

CQuest::~CQuest() {

}

