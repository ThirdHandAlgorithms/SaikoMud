
#ifndef __AbstractSquirrel_H
#define __AbstractSquirrel_H

#include "../Atoms/GFFreeable.h"

#include "GFBRecord.h"
#include "GFBFields.h"


#include "../Molecules/GFBaseCommunicator.h"

/// commreturndata extension for errorstring
class TSquirrelReturnData: public TGFCommReturnData {
    public:
        TGFString errorstring;
};

class TAbstractSquirrelConnection: public TGFFreeable {
   protected:
      bool bConnected;
   public:
      TAbstractSquirrelConnection() : TGFFreeable() {
         bConnected = false;
      };
      ~TAbstractSquirrelConnection() {
      };

      /// connects
      virtual bool connect() = 0;
      /// disconnects
      virtual bool disconnect() = 0;

      virtual bool isConnected() {
         return bConnected;
      }
};


/// base class for database connections
/** The generic way of using this class to traverse through records is:
  *
  * mySquirrel->open();
  * mySquirrel->fetchFields( myFields );
  * while ( mySquirrel->next() ) {
  *    mySquirrel->fetchRecord( myRecord );
  * }
  * mySquirrel->close();
  */
class TAbstractSquirrel: public TGFFreeable {
   protected:
      TAbstractSquirrelConnection *connection;
   public:
     TAbstractSquirrel( TAbstractSquirrelConnection *pConnection ) : TGFFreeable() {
        this->connection = pConnection;
	  };
      ~TAbstractSquirrel() {
	  };

      /// opens 'resource' (a table, a query, depending on implementation)
      virtual bool open( TSquirrelReturnData *errData = NULL ) = 0;
      /// closes 'resource'
      virtual bool close() = 0;

      /// returns the number of individual rows or records (after open())
      virtual unsigned int getRecordCount() = 0;

      /// returns true if a 'resource' has been opened succesfully
      virtual bool isOpen() = 0;

      /// returns true if the current row/record is the First row/record
      virtual bool isFirst() = 0;
      /// returns true if the current row/record is the Last row/record
      virtual bool isLast() = 0;

      /// sets current row/record pointer to the first or next row/record.
      virtual bool next() = 0;

      /// returns the fields selected into the resource, should only be called once
      virtual bool fetchFields( TGFBFields *aFields ) = 0;
      /// returns the current row/record
      virtual bool fetchRecord( TGFBRecord *aRecord ) = 0;
};

#endif // __AbstractSquirrel_H

