
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

   c = static_cast<CCharacter *>( queueBagSave.pop() );
   if ( c != NULL ) {
      c->saveBagslots();
   }

   c = static_cast<CCharacter *>( queueSpellSave.pop() );
   if ( c != NULL ) {
      c->saveSpells();
   }
}

void CCharacterUpdate::scheduleFullSave( CCharacter *pChar ) {
   this->schedule(pChar);
   this->scheduleBagSave(pChar);
   this->scheduleSpellSave(pChar);
}

void CCharacterUpdate::schedule( CCharacter *pChar ) {
   if ( queue.findElement(pChar) == -1 ) {
      queue.push(pChar);
   }
}

void CCharacterUpdate::scheduleBagSave( CCharacter *pChar ) {
   if ( queueBagSave.findElement(pChar) == -1 ) {
      queueBagSave.push(pChar);
   }
}

void CCharacterUpdate::scheduleSpellSave( CCharacter *pChar ) {
   if ( queueSpellSave.findElement(pChar) == -1 ) {
      queueSpellSave.push(pChar);
   }
}
