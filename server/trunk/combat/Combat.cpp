#include "Combat.h"

#include "../Globals.h"

#include <math.h>

#ifndef min
   #define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
   #define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#include <Groundfloor/Materials/GFFunctions.h>

CCombatant::CCombatant() : TGFFreeable() {
   combattimer = NULL;
   swingslices = 0;
   cooldownslices = -1;
   combat = NULL;
   maintarget = NULL;
   spellcastslices = 0;
   iscasting = false;
   castingspell = NULL;
   spelltarget = NULL;

   // barehanded default swingtimer
   autoattackswingtime = 3000;

   name.internalSet("RandomCombatant");

   level.setNotify( GFCreateNotify(unsigned int,CCombatant,this,&CCombatant::setLevel) );
   level.set( 1 );
}

CCombatant::~CCombatant() {
   delete combattimer;
}

void CCombatant::setTarget( CCombatant *t ) {
   maintarget = t;
}

void CCombatant::resetSwing() {
   swingslices = 0;
}

void CCombatant::resetSpellcast() {
   spellcastslices = 0;
}

void CCombatant::resetCooldown() {
   cooldownslices = 0;
}

bool CCombatant::startCasting(const CSpell *spell, CCombatant *target) {
   if ( sliceslock.lockWhenAvailable() ) {
      this->castingspell = spell;
      this->spelltarget = target;

      this->spellcastslices = spell->casttime;
      this->cooldownslices = -1;

      this->resetSwing();

      this->iscasting = true;

      sliceslock.unlock();
   }

   return true;
}

void CCombatant::enterCombat(CCombat *c, bool bOffsetHalf) {
   this->combat = c;

   if (bOffsetHalf) {
      swingslices = (autoattackswingtime / 2) * -1;
   }

   combattimer = new TGFTimer();
   combattimer->setInterval(100);

   combattimer->onTimerEvent += GFCreateNotify(TGFFreeable *,CCombatant,this,&CCombatant::onTimer);
   combattimer->start();
}

bool CCombatant::isInCombat() {
   return (combattimer != NULL);
}

bool CCombatant::isAlive() {
   return ( this->currenthealthpool.get() != 0 );
}

bool CCombatant::isTargetAlive() {
   if ( this->maintarget != NULL ) {
      return this->maintarget->isAlive();
   }

   return false;
}

void CCombatant::setLevel( unsigned int l ) {
   level.internalSet( l );
   // max hp = 100 base + 10hp per level
   maxhealthpool.set( 100 + (l - 1) * 10 );
}

void CCombatant::onTimer( TGFFreeable *obj ) {
   if ( sliceslock.lockWhenAvailable() ) {
      long slice = 100;
      swingslices += slice;

      if (iscasting) {
         spellcastslices -= slice;
      } else if (cooldownslices >= 0) {
         cooldownslices -= slice;
      }

      if (swingslices == 0) {
         swingslices = 1;
      }

      sliceslock.unlock();
   }
}

void CCombatant::leaveCombat() {
   this->combat = NULL;

   delete combattimer;
   combattimer = NULL;

   resetSwing();
   resetCooldown();
}

void CCombatant::combatcycle() {
   doAutoAttack();
}

bool CCombatant::rollHit( CCombatant *target ) {
   int roll = combat->getDiceRoll();
   int lvldiff = target->level.get() - this->level.get();
   return ( roll >= (30 + (lvldiff * 10)) );
}

bool CCombatant::rollCrit( CCombatant *target ) {
   int roll = combat->getDiceRoll();
   int lvldiff = target->level.get() - this->level.get();
   return ( roll >= (40 + (lvldiff * 5)) );
}

int CCombatant::rollAutoattackDamage() {
   int basedamage = 25 + max(0, currentstats.strength.get() >> 2);

   int roll = combat->getDiceRoll();
   int deviation = roll % (int)floor(basedamage * 0.1);

   return basedamage + deviation;
}

int CCombatant::rollSpellDamage() {
   int basedamage = castingspell->basedamage;

   int roll = combat->getDiceRoll();
   int deviation = roll % (int)floor(basedamage * 0.1);

   // -100 + -10 = -110
   // 100 + 10 = 110
   return basedamage + deviation;
}

void CCombatant::interact_autoattack() {
   bool isHit = this->rollHit(maintarget);
   if ( isHit ) {
      bool isCrit = this->rollCrit(maintarget);

      if ( isCrit ) {
         combat->doEvent( COMBATSOURCE_AUTOATTACK, COMBATEVENT_CRIT, this, maintarget, floor(this->rollAutoattackDamage() * 1.5) );
      } else {
         combat->doEvent( COMBATSOURCE_AUTOATTACK, COMBATEVENT_HIT, this, maintarget, this->rollAutoattackDamage() );
      }
   } else {
      combat->doEvent( COMBATSOURCE_AUTOATTACK, COMBATEVENT_MISS, this, maintarget, 0 );
   }
}

void CCombatant::interact_spellcast() {
   bool isHit = true;
   
   // always a hit when spell does 0 damage or heals
   if (castingspell->basedamage > 0) {
      isHit = this->rollHit(spelltarget);
   }

   if ( isHit ) {
      bool isCrit = false;

      // can't crit if the spell does 0 damage
      if (castingspell->basedamage != 0) {
         isCrit = this->rollCrit(maintarget);
      }

      if ( isCrit ) {
         if (castingspell->basedamage < 0) {
            combat->doEvent( COMBATSOURCE_SPELL, COMBATEVENT_HEALCRIT, this, spelltarget, this->rollSpellDamage() * -1.0 );
         } else {
            combat->doEvent( COMBATSOURCE_SPELL, COMBATEVENT_CRIT, this, spelltarget, floor(this->rollSpellDamage() * 1.5) );
         }
      } else {
         if (castingspell->basedamage < 0) {
            combat->doEvent( COMBATSOURCE_SPELL, COMBATEVENT_HEAL, this, spelltarget, this->rollSpellDamage() * -1.0 );
         } else if (castingspell->basedamage == 0) {
            combat->doEvent( COMBATSOURCE_SPELL, COMBATEVENT_AFFECT, this, spelltarget, 0 ); // todo: affect with what???
         } else {
            combat->doEvent( COMBATSOURCE_SPELL, COMBATEVENT_HIT, this, spelltarget, this->rollSpellDamage() );
         }
      }
   } else {
      combat->doEvent( COMBATSOURCE_SPELL, COMBATEVENT_MISS, this, spelltarget, 0 );
   }
}

bool CCombatant::castSpell(const CSpell *spell, CCombatant *target) {
   if ((spell == NULL) || (target == NULL)) {
      return false;
   }

   // check for spell cooldown, can only cast if you're trying to cast within 0.5 (?) seconds of when the cooldown is done
   if (iscasting) {
      return false;
   }

   if (cooldownslices > 500) {
      return false;
   }

   // wait until 0-0.5 remaining time and schedule? to cast
   if (cooldownslices > 0) {
      GFMillisleep(cooldownslices);
   }

   // start casting
   return startCasting(spell, target);
}

void CCombatant::doAutoAttack() {
   if ( sliceslock.lockWhenAvailable() ) {
      // you're either autoattacking or using a spell, not both
      if (iscasting) {
         if (spellcastslices <= 0) {
            interact_spellcast();

            iscasting = false;

            cooldownslices = castingspell->cooldown;
         }
      } else if ( swingslices >= autoattackswingtime ) {
         resetSwing();
         
         if ( swingslices == 0 ) {
            if ( (maintarget != NULL) && (combat != NULL) ) {
               if ( maintarget->isAlive() ) {
                  interact_autoattack();
                  swingslices++;
               } else {
                  combattimer->stop();
               }
            }
         }
      }

      sliceslock.unlock();
   }
}

int CCombatant::affectWithDamage( int combatevent, int amount ) {
   int aftermitigation = 0;

   if ( this->currenthealthpool.get() > 0 ) {
      int protection = this->currentstats.protection.get();
   
      // 100 protection max per level
      int protectioncap = this->level.get() * 100;
      protection = min(protection, protectioncap);
   
      // cap equals 80% mitigation
      double protectionperc = 0.80 / protectioncap * protection;

      // test max armor
      // protectionperc = 0.80;

      int curhp = this->currenthealthpool.get();
      aftermitigation = min( floor(amount * (1.0 - protectionperc)), curhp );

      this->currenthealthpool.set( curhp - aftermitigation );
   }

   return aftermitigation;
}

int CCombatant::affectWithHealing( int combatevent, int amount ) {
   int maxhp = this->maxhealthpool.get();
   int curhp = this->currenthealthpool.get();

   if ( curhp < maxhp ) {
      int newhp = min(maxhp,curhp + amount);

      int afteramount = newhp - curhp;

      this->currenthealthpool.set( newhp );

      return afteramount;
   }

   return 0;
}

void CCombatant::calculateStats() {
   int lvl = this->level.get();

   this->currentstats.energy.set(lvl);
   this->currentstats.strength.set(lvl);
   this->currentstats.protection.set(lvl);
}

CBaseCombatStats *CCombatant::getCurrentStats() {
   this->calculateStats();

   return &(this->currentstats);
}

// ------------------------------------------------------------------------------
#include "../general/ChatChannel.h"


CCombat::CCombat() : TGFThread() {
   combatants.autoClear = false;

   combatlog = new CChatChannel();

   this->setInterval(10);
}

CCombat::~CCombat() {
   this->stopAndWait();

   delete combatlog;
}

void CCombat::execute() {
   unsigned int alive = 0;

   unsigned int targetsdead = 0;

   unsigned int c = combatants.size();
   for ( unsigned int i = 0; i < c; i++ ) {
      CCombatant *obj = static_cast<CCombatant *>( combatants.elementAt(i) );
      if ( obj != NULL ) {
         if ( obj->isAlive() ) {
            alive++;
            if (obj->isTargetAlive()) {
               obj->combatcycle();
            } else {
               targetsdead++;
            }
         }
      }
   }

   // stop combat when everyone's target is dead, or when only 1 or 0 people are still alive
   if ( (alive <= 1) || (alive <= targetsdead) ) {
      this->stop();

      printf("stopping combat\n");

      unsigned int c = combatants.size();
      for ( unsigned int i = 0; i < c; i++ ) {
         CCombatant *obj = static_cast<CCombatant *>( combatants.elementAt(i) );
         if ( obj != NULL ) {
            printf("char %s leaving combat\n", obj->name.get());
            this->leaveCombat(obj);
         }
      }
   }
}

void CCombat::joinCombat( CCombatant *c, bool bOffsetHalf ) {
   if ( combatants.findElement(c) == -1 ) {
      combatants.insertSomewhere( c );

      CTelnetConnection *hook = Global_Server()->getClientFromPool(c);
      if ( hook ) {
         reinterpret_cast<CChatChannel *>(combatlog)->addClient(hook);
      }

      c->enterCombat(this, bOffsetHalf);
   }
}

bool CCombat::isPartOfCombat( CCombatant *c ) {
   return (combatants.findElement(c) >= 0);
}

void CCombat::leaveCombat( CCombatant *c ) {
   c->leaveCombat();

   combatants.removeElement( c );

   CTelnetConnection *hook = Global_Server()->getClientFromPool(c);
   if ( hook ) {
      reinterpret_cast<CChatChannel *>(combatlog)->delClient(hook);
   }
}

int CCombat::getDiceRoll() {
   long r = abs(rand() + rand() - rand() + rand() - rand() + rand());

   return (r % 101);
}

void CCombat::doEvent( int sourcetype, int eventtype, CCombatant *source, CCombatant *target, int amount ) {
   int resamount = 0;

   TGFString combatmsg;

   switch ( eventtype ) {
   case COMBATEVENT_HEAL:
      resamount = target->affectWithHealing( eventtype, amount );
      combatmsg.setValue_ansi("COMBATEVENT - %a healed %b with %c for %d (%e)");
      break;
   case COMBATEVENT_HEALCRIT:
      resamount = target->affectWithHealing( eventtype, amount );
      combatmsg.setValue_ansi("COMBATEVENT - %a crithealed %b with %c for %d (%e)");
      break;
   case COMBATEVENT_HIT:
      resamount = target->affectWithDamage( eventtype, amount );
      combatmsg.setValue_ansi("COMBATEVENT - %a hit %b with %c for %d (%e)");
      break;
   case COMBATEVENT_CRIT:
      resamount = target->affectWithDamage( eventtype, amount );
      combatmsg.setValue_ansi("COMBATEVENT - %a crit %b with %c for %d (%e)");
      break;
   case COMBATEVENT_MISS:
      combatmsg.setValue_ansi("COMBATEVENT - %a missed %b with %c");
      break;
   default:
      combatmsg.setValue_ansi("COMBATEVENT - Unknown event");
      break;
   }

   if ( sourcetype == COMBATSOURCE_AUTOATTACK ) {
      combatmsg.replace_ansi("%c", "AUTOATTACK" );
   } else if ( sourcetype == COMBATSOURCE_SPELL ) {
      // TODO: spellname?
      combatmsg.replace_ansi("%c", "SPELL" );
   } else {
      combatmsg.replace_ansi("%c", "UNKNOWN" );
   }

   if ( combatlog != NULL ) {
      TGFBValue v;

      combatmsg.replace_ansi("%a", source->name.get() );
      combatmsg.replace_ansi("%b", target->name.get() );
      v.setInteger(resamount);
      combatmsg.replace_ansi("%d", v.asString()->getValue() );
      v.setInteger(amount);
      combatmsg.replace_ansi("%e", v.asString()->getValue() );

      reinterpret_cast<CChatChannel *>(combatlog)->messageToAll( NULL, &combatmsg );
   }

   CTelnetConnection *tcsource, *tctarget;

   tcsource = Global_Server()->getClientFromPool(target);
   if (tcsource != NULL) {
      tcsource->inform_combatevent(source->WorldId, target->WorldId, eventtype, amount, &combatmsg);
   }
   tctarget = Global_Server()->getClientFromPool(source);
   if (tctarget != NULL) {
      tctarget->inform_combatevent(source->WorldId, target->WorldId, eventtype, amount, &combatmsg);
   }

   if ( !target->isAlive() ) {
      if ( combatlog != NULL ) {
         combatmsg.setValue_ansi("COMBATEVENT - %b has died");
         combatmsg.replace_ansi("%b", target->name.get() );

         reinterpret_cast<CChatChannel *>(combatlog)->messageToAll( NULL, &combatmsg );
      }

      if (tcsource != NULL) {
         tcsource->inform_combatevent(source->WorldId, target->WorldId, COMBATEVENT_DEATH, 0, &combatmsg);
      }
      if (tctarget != NULL) {
         tctarget->inform_combatevent(source->WorldId, target->WorldId, COMBATEVENT_DEATH, 0, &combatmsg);
      }

      this->leaveCombat(target);

      if (target->WorldId != 0) {
         Global_World()->handleDeath( reinterpret_cast<CCharacter *>(target), reinterpret_cast<CCharacter *>(source) );
      }
   }
}

