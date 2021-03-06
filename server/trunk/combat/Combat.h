
#ifndef __COMBAT_H__
#define __COMBAT_H__

#include <stdint.h>

#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Atoms/GFLockable.h>
#include <Groundfloor/Molecules/GFVector.h>
#include <Groundfloor/Molecules/GFProperty.h>
#include <Groundfloor/Materials/GFThread.h>
#include <Groundfloor/Materials/GFCallbackThread.h>

#include "../combat/CombatStats.h"
#include "../world/Spells.h"

class CCombat;

#define COMBATEVENT_MISS 1
#define COMBATEVENT_HIT 2
#define COMBATEVENT_CRIT 3

#define COMBATEVENT_HEAL 5
#define COMBATEVENT_HEALCRIT 6

#define COMBATEVENT_AFFECT 8

#define COMBATEVENT_DEATH 9


#define COMBATSOURCE_AUTOATTACK 0
#define COMBATSOURCE_SPELL 1


class CCombatant: public TGFFreeable {
protected:
   CBaseCombatStats currentstats;

   TGFTimer *combattimer;

   CCombat *combat;
   CCombatant *maintarget;

   CCombatant *spelltarget;

   bool iscasting;
   const CSpell *castingspell;

   TGFLockable sliceslock;
   long cooldownslices;
   long swingslices;
   long spellcastslices;

   long autoattackswingtime;

   void resetSwing();
   void resetCooldown();
   void resetSpellcast();

   bool startCasting(const CSpell *spell, CCombatant *target);

   void doAutoAttack();

   bool rollHit( CCombatant *target );
   bool rollCrit( CCombatant *target );
   int rollAutoattackDamage();
   int rollSpellDamage();

   void interact_autoattack();
   void interact_spellcast();

   virtual void calculateStats();
public:
   uint32_t WorldId;

   TGFStringProperty name;
   TGFProperty<unsigned int> level;
   TGFProperty<unsigned int> maxhealthpool;
   TGFProperty<unsigned int> currenthealthpool;

   CCombatant();
   ~CCombatant();

   void setTarget( CCombatant *t );

   bool isInCombat();
   bool isAlive();

   bool isTargetAlive();

   void setLevel( unsigned int l );
   void onTimer( TGFFreeable *obj );

   void combatcycle();

   bool castSpell(const CSpell *spell, CCombatant *target);

   int affectWithDamage( int combatevent, int amount );
   int affectWithHealing( int combatevent, int amount );

   void enterCombat( CCombat *c, bool bOffsetHalf = false);
   void leaveCombat();

   CBaseCombatStats *getCurrentStats();
};


class CCombat: public TGFThread {
protected:
   TGFVector combatants;

   TGFFreeable *combatlog;
public:
   CCombat();
   ~CCombat();

   void joinCombat( CCombatant *c, bool bOffsetHalf = false );
   bool isPartOfCombat( CCombatant *c );
   void leaveCombat( CCombatant *c );

   int getDiceRoll();

   void doEvent( int sourcetype, int eventtype, CCombatant *source, CCombatant *target, int amount );

   void execute();
};

#endif //__COMBAT_H__
