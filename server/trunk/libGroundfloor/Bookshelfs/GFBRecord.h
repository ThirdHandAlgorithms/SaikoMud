
#ifndef __GFBRecord_H
#define __GFBRecord_H

#include "../Molecules/GFVector.h"
#include "GFBValue.h"

class TGFBRecord: public TGFVector {
   public:
      TGFBRecord();
      ~TGFBRecord();

      /// appends a new empty value to record, usage not recommended
      TGFBValue *newValue();
      /// returns value by given index
      TGFBValue *getValue( unsigned int iIndex );

      /// creates or deletes values to match the given number of values
      void setValueCount( unsigned int iCount );

      /// returns true if the integer representation of the value at iColumn equals iNeedle
      bool matchInteger( unsigned int iColumn, int iNeedle );
};

#endif // __GFBRecord_H

