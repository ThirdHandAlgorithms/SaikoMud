
#include "Account.h"

CAccount::CAccount( TMySQLSquirrelConnection *pConn ) : TGFFreeable() {
   this->conn = pConn;
   this->loggedin = false;
   this->id = 0;
   this->isAdmin = false;
}

CAccount::~CAccount() {
   this->logout();
}

void CAccount::logout() {
   this->loggedin = false;
}

bool CAccount::login( TGFString *sUser, TGFString *sPass ) {
   this->logout();

   TGFString sql("select id, is_admin from account where login=:user and password=SHA1(:pass)");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("user")->setString(sUser);
   qry.findOrAddParam("pass")->setString(sPass);

   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      if ( qry.next() ) {
         TGFBRecord rec;
         qry.fetchRecord(&rec);
         this->id = rec.getValue(0)->asInteger();
         this->isAdmin = (rec.getValue(1)->asInteger() == 1);
         this->loggedin = true;
      }
      qry.close();
   } else {
      printf("CAccount::login(): %s\n", err.errorstring.getValue() );
   }

   return this->loggedin;
}

unsigned long CAccount::getMainCharId() {
   unsigned long charid = 0;

   TGFString sql("select id from `char` where account_id=:account_id order by id asc limit 0,1");
   TMySQLSquirrel qry(this->conn);
   qry.setQuery(&sql);
   qry.findOrAddParam("account_id")->setInteger(this->id);
   TSquirrelReturnData err;
   if ( qry.open(&err) ) {
      while ( qry.next() ) {
         TGFBRecord rec;
         qry.fetchRecord(&rec);
         charid = rec.getValue(0)->asInteger();
      }

      qry.close();
   } else {
      printf("CAccount::getMainCharId(): %s\n", err.errorstring.getValue() );
   }

   return charid;
}
