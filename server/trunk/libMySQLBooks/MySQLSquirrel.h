
#ifndef __MySQLSquirrel_H
#define __MySQLSquirrel_H


#include <Groundfloor/Bookshelfs/RemoteSquirrel.h>
#include <Groundfloor/Bookshelfs/GFBTable.h>
#include <Groundfloor/Molecules/GFString.h>

#include <Groundfloor/Atoms/GFDefines.h>

#ifdef GF_OS_WIN32
   #include <winsock2.h>
#endif

#include <mysql/mysql.h>
#include <mysql/errmsg.h>

#include <Groundfloor/Materials/GFThread.h>
#include <Groundfloor/Atoms/GFLockable.h>

bool initMySQLBooks();
bool finiMySQLBooks();

class TMySQLSquirrelConnection: public TRemoteSquirrelConnection {
   protected:
      MYSQL aMysqlObj;
      MYSQL *pMysqlObj;

      bool bCompression;

      void debug( const char *sMsg );

      void startThreadNotify(TGFThread *aThread);
      void endThreadNotify(TGFThread *aThread);
   public:
      TMySQLSquirrelConnection();
      ~TMySQLSquirrelConnection();

      /// Select database to use. No default.
      void selectDatabase( const char *sDatabase );

      bool connect();
      bool disconnect();

      MYSQL *getMySQLObj();
};

class TMySQLSquirrel: public TRemoteSQL {
   protected:
      MYSQL_RES *pCurrentResult;
      MYSQL_ROW aCurrentRow;

      TGFString sCurrentQuery;
      TGFBFields aCurrentFields;

      unsigned long iCurrentRecord;
      unsigned long iRecordCount;
      bool bAnsiMode;

      void retreiveFields( TGFBFields *aFields ) const;

      void replaceInQuery( const char *sToken, TGFBValue *aValue, bool bIncludeQuotations = true );
   public:
      TMySQLSquirrel( TRemoteSquirrelConnection *pConnection );
      ~TMySQLSquirrel();

      /// Set the query to send to the server when calling open().
      void setQuery( const TGFString *sQuery, bool bReplaceBackticksWithDoubleQuotes = true );

      __int64 getLastID( const char *sTableName, TSquirrelReturnData *errData = NULL );

      unsigned int getRecordCount();

      // implementation
      bool open( TSquirrelReturnData *errData = NULL );
      bool close();

      bool isOpen();

      bool isFirst();
      bool isLast();

      bool next();

      bool fetchFields( TGFBFields *aFields );
      bool fetchRecord( TGFBRecord *aRecord );
};

#endif // __MySQLSquirrel_H

