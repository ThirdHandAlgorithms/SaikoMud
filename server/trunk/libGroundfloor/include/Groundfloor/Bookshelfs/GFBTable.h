
#ifndef __GFBTable_H
#define __GFBTable_H

#include "../Atoms/GFFreeable.h"
#include "../Molecules/GFProperty.h"

#include "GFBFields.h"
#include "GFBRecords.h"

/// represents a table with fields and records
class TGFBTable: public TGFFreeable {
   public:
      /// Fields and records are initialized to NULL,
      ///  setting the fields and records must be done manually.
      /// Recommended: Try to fill the fields and records before setting properties.
      TGFBTable();
      ~TGFBTable();

      TGFProperty<TGFBFields *> fields;
      TGFProperty<TGFBRecords *> records;
};

#endif // __GFBTable_H

