#include "Combat.h"

#include "../Globals.h"

CCombatant::CCombatant() : TGFFreeable() {
   combattimer = NULL;
   swingslices = 0;
   cooldownslices = 0;
   combat = NULL;
   maintarget = NULL;

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

void CCombatant::resetCooldown() {
   cooldownslices = 0;
}

void CCombatant::enterCombat(CCombat *c) {
   this->combat = c;

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

void CCombatant::setLevel( unsigned int l ) {
   level.internalSet( l );
   maxhealthpool.set( l * 100 );
}

void CCombatant::onTimer( TGFFreeable *obj ) {
   if ( sliceslock.lockWhenAvailable() ) {
      cooldownslices += 100;
      swingslices += 100;

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
   int lvldiff = this->level.get() - target->level.get();
   return ( roll >= (50 + lvldiff) );
}

bool CCombatant::rollCrit( CCombatant *target ) {
   int roll = combat->getDiceRoll();
   int lvldiff = this->level.get() - target->level.get();
   return ( roll >= (30 + lvldiff) );
}

int CCombatant::rollAutoattackDamage() {
   int basedamage = 25 + max(0, currentstats.strength.get() >> 2);

   int roll = combat->getDiceRoll();
   int deviation = roll % (int)floor(basedamage * 0.1);

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

void CCombatant::doAutoAttack() {
   if ( sliceslock.lockWhenAvailable() ) {
      if ( swingslices >= autoattackswingtime ) {
         resetSwing();
      }

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

      sliceslock.unlock();
   }
}

int CCombatant::affectWithDamage( int combatevent, int amount ) {
   int aftermitigation = 0;

   if ( this->currenthealthpool.get() > 0 ) {
      int protection = this->currentstats.protection.get();
   
      // 200 protection max per level
      int protectioncap = this->level.get() * 200;
      protection = min(protection, protectioncap);
   
      // cap equals 80% mitigation
      double protectionperc = 0.80 / protectioncap * protection;

      int curhp = this->currenthealthpool.get();
      aftermitigation = min( floor(amount * (1 - protectionperc)), curhp );

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

   unsigned int c = combatants.size();
   for ( unsigned int i = 0; i < c; i++ ) {
      CCombatant *obj = static_cast<CCombatant *>( combatants.elementAt(i) );
      if ( obj != NULL ) {
         if ( obj->isAlive() ) {
            alive++;
            obj->combatcycle();
         }
      }
   }

   if ( alive <= 1 ) {
      this->stop();

      unsigned int c = combatants.size();
      for ( unsigned int i = 0; i < c; i++ ) {
         CCombatant *obj = static_cast<CCombatant *>( combatants.elementAt(i) );
         if ( obj != NULL ) {
            this->leaveCombat(obj);
         }
      }
   }
}

void CCombat::joinCombat( CCombatant *c ) {
   if ( combatants.findElement(c) == -1 ) {
      combatants.insertSomewhere( c );

      CTelnetConnection *hook = Global_Server()->getClientFromPool(c);
      if ( hook ) {
         reinterpret_cast<CChatChannel *>(combatlog)->addClient(hook);
      }

      c->enterCombat(this);
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
   long r = rand() + rand() - rand() + rand() - rand() + rand();

   return (r % 100);
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

   if ( !target->isAlive() ) {
      if ( combatlog != NULL ) {
         combatmsg.setValue_ansi("COMBATEVENT - %b has died");
         combatmsg.replace_ansi("%b", target->name.get() );

         reinterpret_cast<CChatChannel *>(combatlog)->messageToAll( NULL, &combatmsg );
      }
   }
}

