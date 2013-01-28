
#include "RemoteSquirrel.h"


TRemoteSQL::TRemoteSQL( TRemoteSquirrelConnection *pConnection  ) : TAbstractSquirrel( pConnection ) {
   params.resizeVector(10);
}

TRemoteSQL::~TRemoteSQL() {
}

void TRemoteSQL::prepareQuery() {
   TGFString n;

   unsigned int c = params.size();
   for ( unsigned int i = 0; i < c; i++ ) {
      TGFSimpleStringLookupObject *obj = static_cast<TGFSimpleStringLookupObject *>( params.elementAt(i) );

      n.setValue(":",1);
      n.append( &obj->name );
      this->replaceInQuery( n.getValue(), static_cast<TGFBValue *>(obj->object) );
   }
}

TGFBValue *TRemoteSQL::findOrAddParam( const TGFString *sName ) {
   TGFBValue *p = static_cast<TGFBValue *>( params.getObjectByString( sName ) );
   if ( p == NULL ) {
      p = new TGFBValue();
      // sorted insert, so we dont have to sort the parameters afterwards
      params.insertObjectSortedByLength( sName, p, false );
   }

   return p;
}

TGFBValue *TRemoteSQL::findOrAddParam( const char *sName ) {
   TGFBValue *p = static_cast<TGFBValue *>( params.getObjectByString( sName ) );
   if ( p == NULL ) {
      p = new TGFBValue();
      // sorted insert, so we dont have to sort the parameters afterwards
      params.insertObjectSortedByLength( sName, p, false );
   }

   return p;
}

void TRemoteSQL::setQuery( const TGFString *sQuery, bool bReplaceBackticksWithDoubleQuotes ) {
    sCurrentQuery.setValue( sQuery );
    if ( bReplaceBackticksWithDoubleQuotes ) {
        // \"ID\" is a ANSI-SQL way to make sure the interpreter knows ID is an identifier,
        //  it is however the most annoying character to use in strings and backticks `ID` are way easier
        //  MySQL uses backticks by default, so a MySQL implementation doesn't need this replace command unless in ANSI mode
        sCurrentQuery.replace_ansi( "`", "\"" );
    }
}

TGFString *TRemoteSQL::getCurrentQuery() {
    return &sCurrentQuery;
}

// ------------

TRemoteSquirrelConnection::TRemoteSquirrelConnection() : TAbstractSquirrelConnection() {
}

TRemoteSquirrelConnection::~TRemoteSquirrelConnection() {
}

TLocalSquirrelConnection::TLocalSquirrelConnection() : TAbstractSquirrelConnection() {
}

TLocalSquirrelConnection::~TLocalSquirrelConnection() {
}
