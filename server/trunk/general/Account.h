
#ifndef __ACCOUNT_H__
#define __ACCOUNT_H__

#include <Groundfloor/Atoms/GFFreeable.h>
#include <MySQLBooks/MySQLSquirrel.h>

class CAccount: public TGFFreeable {
protected:
   TMySQLSquirrelConnection *conn;
   bool loggedin;

   unsigned long id;

public:
   CAccount( TMySQLSquirrelConnection *pConn );
   ~CAccount();

   bool login( TGFString *sUser, TGFString *sPass );
   void logout();

   unsigned long getMainCharId();
};

#endif // __ACCOUNT_H__
