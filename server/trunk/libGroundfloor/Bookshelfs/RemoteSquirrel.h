
#ifndef __RemoteSquirrel_H
#define __RemoteSquirrel_H

#include "AbstractSquirrel.h"
#include "../Materials/GFStringLookup.h"

/// specialized Squirrel implementation for Remote Databases
class TRemoteSquirrelConnection: public TAbstractSquirrelConnection {
   public:
      TRemoteSquirrelConnection();
      ~TRemoteSquirrelConnection();

      /// hostname (implementation should automatically resolve this into an IP if needed)
      TGFStringProperty host;
      /// port to connect to on the host
      TGFProperty<int> port;
      /// username to use when logging in to the server
      TGFStringProperty username;
      /// password to use
      TGFStringProperty password;
};

/// for Local Databases
class TLocalSquirrelConnection: public TAbstractSquirrelConnection {
   public:
      TLocalSquirrelConnection();
      ~TLocalSquirrelConnection();

      TGFStringProperty path;
};


/// specialized Squirrel implementation for Remote Databases
class TRemoteSQL: public TAbstractSquirrel {
    protected:
        TGFSimpleStringLookup params;
        TGFString sCurrentQuery;

        virtual void prepareQuery();

        /// replaces occurences of sToken (parameters) with the given value
        /** depending on the lastsettype of the value the function should escape the value according to the specific database's rules, setting bIncludeQuotation to false is not recommended, as it could negate portability */
        virtual void replaceInQuery( const char *sToken, TGFBValue *aValue, bool bIncludeQuotations = true ) = 0;
    public:
        TRemoteSQL( TRemoteSquirrelConnection *pConnection  );
        ~TRemoteSQL();

        /// sets the current query to sQuery
        virtual void setQuery( const TGFString *sQuery, bool bReplaceBackticksWithDoubleQuotes = true );

        /// returns ptr to current query
        virtual TGFString *getCurrentQuery();

        virtual TGFBValue *findOrAddParam( const char *sToken );
        virtual TGFBValue *findOrAddParam( const TGFString *sName );

        /// return (if possible) the last id that was inserted or updated by use of an autonumbering type or sequence
        /** this function should only return the id that was used last in this exact session/connection */
        virtual __int64 getLastID( const char *sTableName, TSquirrelReturnData *errData = NULL ) = 0;
};

#endif // __RemoteSquirrel_H

