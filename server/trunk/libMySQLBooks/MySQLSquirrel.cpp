
#include "MySQLSquirrel.h"
#include "MySquirrelUtils.h"

#include <Groundfloor/Atoms/GFDefines.h>
#include <Groundfloor/Materials/GFFunctions.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "MySquirrelUtils.h"

#include <Groundfloor/Materials/GFThread.h>
#include <Groundfloor/Molecules/GFCallbacks.h>

bool initMySQLBooks() {
   return !mysql_library_init( 0, 0, 0 );
}
bool finiMySQLBooks() {
   mysql_library_end();

   return true;
}


TMySQLSquirrelConnection::TMySQLSquirrelConnection() : TRemoteSquirrelConnection() {
   pMysqlObj      = &aMysqlObj;
   bCompression   = true;

   host.set( "localhost" );
   port.set( 3306 );
   username.set( "root" );
   password.set( "" );

   addThreadStartNotify( GFCreateNotify(TGFThread *, TMySQLSquirrelConnection, this, &TMySQLSquirrelConnection::startThreadNotify) );
   addThreadEndNotify( GFCreateNotify(TGFThread *, TMySQLSquirrelConnection, this, &TMySQLSquirrelConnection::endThreadNotify) );
}

TMySQLSquirrelConnection::~TMySQLSquirrelConnection() {
}

void TMySQLSquirrelConnection::startThreadNotify(TGFThread *aThread) {
   mysql_thread_init();
}

void TMySQLSquirrelConnection::endThreadNotify(TGFThread *aThread) {
   mysql_thread_end();
}


bool TMySQLSquirrelConnection::connect() {
   mysql_init( pMysqlObj );

   mysql_options( pMysqlObj, MYSQL_READ_DEFAULT_GROUP, "MySQLSquirrel" );

   unsigned long iFlags = 0;
   if ( bCompression ) {
      iFlags |= CLIENT_COMPRESS;
   }

   mysql_options( pMysqlObj, MYSQL_SET_CHARSET_NAME, "utf8" );

   if ( !mysql_real_connect( pMysqlObj, host.get(), username.get(), password.get(), NULL, port.get(), NULL, iFlags ) ) {
       debug( mysql_error( pMysqlObj ) );
   } else {
      bConnected = true;
   }

   return bConnected;
}

bool TMySQLSquirrelConnection::disconnect() {
   if ( bConnected ) {
      mysql_close( pMysqlObj );
   }

   return true;
}

void TMySQLSquirrelConnection::selectDatabase( const char *sDatabase ) {
   if ( bConnected ) {
      mysql_select_db( pMysqlObj, sDatabase );

      if ( mysql_errno( pMysqlObj ) > 0 ) {
         debug( mysql_error( pMysqlObj ) );
      }
   }
}

void TMySQLSquirrelConnection::debug( const char *sMsg ) {
   printf( "%s\n", sMsg );
}

MYSQL *TMySQLSquirrelConnection::getMySQLObj() {
   return pMysqlObj;
}

//------------------------------------------------------------------------------

TMySQLSquirrel::TMySQLSquirrel( TRemoteSquirrelConnection *pConnection ) : TRemoteSQL( pConnection ) {
   bAnsiMode      = false;

   pCurrentResult = NULL;
   iCurrentRecord = 0;
   iRecordCount   = 0;

   sCurrentQuery.setValue( "", 0 );
}

TMySQLSquirrel::~TMySQLSquirrel() {
}

unsigned int TMySQLSquirrel::getRecordCount() {
   return iRecordCount;
}

void TMySQLSquirrel::retreiveFields( TGFBFields *aFields ) {
   MYSQL_FIELD *f;

   aFields->clear();

   unsigned int fieldcount = mysql_num_fields( pCurrentResult );

   aFields->resizeVector( fieldcount );

   for ( unsigned int i = 0; i < fieldcount; i++ ) {
      f = mysql_fetch_field_direct( pCurrentResult, i );

      TGFBField *field = aFields->newField();

      field->name.setValue( f->name, strlen(f->name) );
      field->maxlength.set( f->length );

      switch ( f->type ) {
         case MYSQL_TYPE_TINY:
         case MYSQL_TYPE_SHORT:
         case MYSQL_TYPE_LONG:
         case MYSQL_TYPE_INT24:
         case MYSQL_TYPE_LONGLONG:
         case MYSQL_TYPE_BIT:
            field->datatype.set( FLDTYPE_INT );
            break;
         case MYSQL_TYPE_DECIMAL:
         case MYSQL_TYPE_NEWDECIMAL:
         case MYSQL_TYPE_FLOAT:
         case MYSQL_TYPE_DOUBLE:
            field->datatype.set( FLDTYPE_DOUBLE );
            break;
         case MYSQL_TYPE_DATE:
         case MYSQL_TYPE_TIME:
         case MYSQL_TYPE_DATETIME:
            field->datatype.set( FLDTYPE_DATE );
            break;
         case MYSQL_TYPE_TIMESTAMP:
            field->datatype.set( FLDTYPE_DATE );
            break;
         default:
            field->datatype.set( FLDTYPE_STRING );
      }

      if ( (f->flags & PRI_KEY_FLAG) > 0 ) {
         field->flags.set( FLDFLAG_PRIMARY );
      } else {
         field->flags.set( FLDFLAG_NORMAL );
      }
   }
}

void TMySQLSquirrel::setQuery( const TGFString *sQuery, bool bReplaceBackticksWithDoubleQuotes ) {
    sCurrentQuery.setValue( sQuery );

    if ( bReplaceBackticksWithDoubleQuotes && bAnsiMode ) {
        sCurrentQuery.replace_ansi( "`", "\"" );
    }
}

void TMySQLSquirrel::replaceInQuery( const char *sToken, TGFBValue *aValue, bool bIncludeQuotations ) {
    TGFString replacestr;

    TGFString *sValue = aValue->asString();
    replacestr.setLength( sValue->getLength() * 2 );

    unsigned long iNewLen = mysql_real_escape_string( static_cast<TMySQLSquirrelConnection *>(this->connection)->getMySQLObj(), replacestr.getValue(), sValue->getValue(), sValue->getLength() );
    replacestr.setLength( iNewLen );

    switch ( aValue->lasttypeset.get() ) {
        case FLDTYPE_DOUBLE:
        case FLDTYPE_INT:
        case FLDTYPE_INT64:
            break;
        case FLDTYPE_BOOL:
            if ( aValue->asBoolean() ) {
                replacestr.setValue_ansi( "1" );
            } else {
                replacestr.setValue_ansi( "0" );
            }
            break;
        case FLDTYPE_DATE:
            if ( bIncludeQuotations ) {
                replacestr.prepend_ansi( "'" );
                replacestr.append_ansi( "'" );
            }
            break;
        default:
            if ( bIncludeQuotations ) {
                replacestr.prepend_ansi( "'" );
                replacestr.append_ansi( "'" );
            }
            break;
    }

    TGFString tokenstr;
    tokenstr.setValue_ansi( sToken );

    sCurrentQuery.replace( &tokenstr, &replacestr );
}

__int64 TMySQLSquirrel::getLastID( const char *sTableName, TSquirrelReturnData *errData ) {
    return mysql_insert_id( static_cast<TMySQLSquirrelConnection *>(this->connection)->getMySQLObj() );
}

bool TMySQLSquirrel::open( TSquirrelReturnData *errData ) {
   this->prepareQuery();

   if ( this->connection->isConnected() ) {
      this->connection->lock.lockWhenAvailable();
      int iError = mysql_real_query( static_cast<TMySQLSquirrelConnection *>(this->connection)->getMySQLObj(), sCurrentQuery.getValue(), sCurrentQuery.getLength() );
      if ( iError == 0 ) {
         pCurrentResult = mysql_store_result( static_cast<TMySQLSquirrelConnection *>(this->connection)->getMySQLObj() );
         this->connection->lock.unlock();

         iRecordCount = 0 ;

         if ( pCurrentResult != NULL ) {
            iRecordCount = mysql_num_rows( pCurrentResult );

            retreiveFields( &aCurrentFields );
         }

         if ( errData != NULL ) {
            errData->eof = false;
            errData->error = false;
            errData->errorcode = 0;
            errData->errorstring.setValue_ansi( "" );
            errData->affected = mysql_affected_rows( static_cast<TMySQLSquirrelConnection *>(this->connection)->getMySQLObj() );
         }

         return true;
      } else {
         this->connection->lock.unlock();

         if ( errData != NULL ) {
            errData->eof = false;
            errData->error = true;
            errData->errorcode = mysql_errno( static_cast<TMySQLSquirrelConnection *>(this->connection)->getMySQLObj() );
            errData->errorstring.setValue_ansi( mysql_error( static_cast<TMySQLSquirrelConnection *>(this->connection)->getMySQLObj() ) );
            errData->affected = 0;
         }

      }
   } else {
   }

   return false;
}

bool TMySQLSquirrel::close() {
   if ( pCurrentResult != NULL ) {
      mysql_free_result( pCurrentResult );

      aCurrentFields.clear();
      pCurrentResult = NULL;

      return true;
   }

   return false;
}

bool TMySQLSquirrel::isOpen() {
   return ( pCurrentResult != NULL );
}

bool TMySQLSquirrel::isFirst() {
   return (iCurrentRecord == 1);
}

bool TMySQLSquirrel::isLast() {
   return (iCurrentRecord == iRecordCount);
}

bool TMySQLSquirrel::next() {
   if ( isOpen() ) {
      iCurrentRecord++;

      aCurrentRow = mysql_fetch_row( pCurrentResult );

      return ( aCurrentRow != NULL );
   }

   return false;
}

bool TMySQLSquirrel::fetchFields( TGFBFields *aFields ) {
   if ( isOpen() ) {
      retreiveFields( aFields );

      return true;
   }

   return false;
}

bool TMySQLSquirrel::fetchRecord( TGFBRecord *aRecord ) {
   if ( isOpen() ) {
      aRecord->clear();

      unsigned long *lengths = mysql_fetch_lengths( pCurrentResult );

      unsigned int c = aCurrentFields.size();

      aRecord->resizeVector( c );

      for ( unsigned int i = 0; i < c; i++ ) {
         TGFBValue *value = aRecord->newValue();

         switch ( aCurrentFields.getField(i)->datatype.get() ) {
            case FLDTYPE_STRING:
            case FLDTYPE_UNKNOWN:
               if ( aCurrentRow[i] != NULL ) {
                  value->setString( aCurrentRow[i], lengths[i] );
               } else {
                  value->setString( "", 0 );
               }
               break;
            case FLDTYPE_INT:
               if ( aCurrentRow[i] != NULL ) {
                  value->setInteger( atoi( aCurrentRow[i] ) );
               } else {
                  value->setInteger( 0 );
               }
               break;
            case FLDTYPE_DOUBLE:
               if ( aCurrentRow[i] != NULL ) {
                  value->setDouble( atof( aCurrentRow[i] ) );
               } else {
                  value->setDouble( 0 );
               }
               break;
            case FLDTYPE_DATE:
               if ( aCurrentRow[i] != NULL ) {
                  TGFString *sTmp = new TGFString( aCurrentRow[i], lengths[i] );
                  value->setTimestamp( GFDateStrToTimestamp( sTmp, false, false ) );
                  delete sTmp;
               } else {
                  value->setTimestamp( 0 );
               }

               break;
         }
      }

      return true;
   }

   return false;
}


