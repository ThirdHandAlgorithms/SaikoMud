#pragma once

#include <Groundfloor/Materials/GFThread.h>
#include <Groundfloor/Molecules/GFVector.h>
#include "Character.h"

class CRespawnThread: public TGFThread {
protected:
   unsigned int cursor;

   TGFVector characters;
   TGFLockable lock;

public:
   CRespawnThread();
   ~CRespawnThread();

   void add(CCharacter *c);

   void execute();
};
