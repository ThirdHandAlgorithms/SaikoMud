
#ifndef __GFBRecords_H
#define __GFBRecords_H

#include "../Molecules/GFVector.h"
#include "GFBRecord.h"

class TGFBRecords: public TGFVector {
   public:
      // initializes empty vector of records
      TGFBRecords();
      ~TGFBRecords();

      /// returns record at given index
      TGFBRecord *getRecord( unsigned int iIndex );
      /// appends a new empty record, usage not recommended
      TGFBRecord *newRecord();

      /// Looks up first record where given columnindex equals the given integer value iNeedle.
      ///  This is primarily added to simplify ID-field lookups.
      TGFBRecord *findRecordByInteger( unsigned int iColumn, int iNeedle );
};

#endif // __GFBRecords_H

