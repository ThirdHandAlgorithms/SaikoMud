
#include "CombatStats.h"

CBaseCombatStats::CBaseCombatStats() : TGFFreeable() {
}

CBaseCombatStats::~CBaseCombatStats() {
}

// -----------------------------------------

CCombatStats::CCombatStats(TMySQLSquirrelConnection *pConn, unsigned long id) : CBaseCombatStats() {
   this->conn = pConn;
   this->id = id;

   this->load();
}

CCombatStats::CCombatStats() : CBaseCombatStats() {
   this->conn = NULL;
   this->id = 0;
}

CCombatStats::~CCombatStats() {
}

void CCombatStats::load() {
   TGFString sql("select * from `stats` where id=:id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("id")->setInteger(id);
   
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      TGFBFields flds;
      qry.fetchFields(&flds);
      if ( qry.next() ) {
         TGFBRecord rec;
         qry.fetchRecord(&rec);

         this->strength.set( rec.getValue(flds.getFieldIndex_ansi("strength"))->asInteger() );
         this->energy.set( rec.getValue(flds.getFieldIndex_ansi("energy"))->asInteger() );
         this->protection.set( rec.getValue(flds.getFieldIndex_ansi("protection"))->asInteger() );
      }

      qry.close();
   } else {
      printf("CCombatStats::load(): %s\n", err.errorstring.getValue());
   }
}

void CCombatStats::save() {
   TGFString sql("update `stats` set strength=:strength, energy=:energy, protection=:protection where id=:id");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("id")->setInteger(this->id);
   qry.findOrAddParam("strength")->setInteger(this->strength.get());
   qry.findOrAddParam("energy")->setInteger(this->energy.get());
   qry.findOrAddParam("protection")->setInteger(this->protection.get());
   
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      qry.close();
   } else {
      printf("CCombatStats::save(): %s\n", err.errorstring.getValue());
   }
}
