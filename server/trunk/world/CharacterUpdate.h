
#ifndef __CHARACTERUPDATE_H__
#define __CHARACTERUPDATE_H__

#include <Groundfloor/Materials/GFFifoVector.h>
#include <Groundfloor/Materials/GFThread.h>
#include "Character.h"

class CCharacterUpdate: public TGFThread {
protected:
   TGFFifoVector queue;
   TGFFifoVector queueBagSave;
   TGFFifoVector queueSpellSave;

public:
   CCharacterUpdate();
   ~CCharacterUpdate();

   void execute();

   void schedule( CCharacter *pChar );
   void scheduleBagSave( CCharacter *pChar );
   void scheduleSpellSave( CCharacter *pChar );

   void scheduleFullSave( CCharacter *pChar );
};

#endif //__CHARACTERUPDATE_H__
