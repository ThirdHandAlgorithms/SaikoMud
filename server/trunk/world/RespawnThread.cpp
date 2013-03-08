#include "RespawnThread.h"

#include <Groundfloor\Materials\GFFunctions.h>
#include "..\Globals.h"

CRespawnThread::CRespawnThread() : TGFThread() {
   cursor = 0;

   this->setInterval(1000, true);
}


CRespawnThread::~CRespawnThread() {

}

void CRespawnThread::execute() {
   lock.lockWhenAvailable();
   
   if (cursor < characters.size()) {
      CCharacter *c = static_cast<CCharacter *>(characters.elementAt(cursor));
      if (c != NULL) {
         __int64 tNow = GFGetTimestamp();
         if ((tNow - c->timeofdeath) >= c->respawntime) {
            Global_World()->onRespawnTimerCharacter(c);

            characters.removeElement(cursor);
         }
      }

      cursor++;
   } else {
      cursor = 0;
      characters.compress();
   }

   lock.unlock();
}

void CRespawnThread::add(CCharacter *c) {
   lock.lockWhenAvailable();
   
   characters.addElement(c);

   lock.unlock();
}
