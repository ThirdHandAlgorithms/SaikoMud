
#include "CharacterUpdate.h"


#include "../Globals.h"

CCharacterUpdate::CCharacterUpdate() : TGFThread() {
   this->start();
}
CCharacterUpdate::~CCharacterUpdate() {
   this->stopAndWait();
}

void CCharacterUpdate::execute() {
   CCharacter *c = static_cast<CCharacter *>( queue.pop() );
   if ( c != NULL ) {
      c->save();
   }
}

void CCharacterUpdate::schedule( CCharacter *pChar ) {
   if ( queue.findElement(pChar) == -1 ) {
      queue.push(pChar);
   }
}

