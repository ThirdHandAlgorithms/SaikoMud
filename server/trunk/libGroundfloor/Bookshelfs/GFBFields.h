
#ifndef __GFBFields_H
#define __GFBFields_H

#include "../Molecules/GFVector.h"
#include "GFBField.h"

class TGFBFields: public TGFVector {
   public:
      TGFBFields();
      ~TGFBFields();

      /// returns field index with given name, -1 if not found
      int getFieldIndex( const char *sName, unsigned int len );
      /// returns field index with given name, -1 if not found
      int getFieldIndex_ansi( const char *sName );
      /// returns field index with given name, -1 if not found
      int getFieldIndex( const TGFString *sName );

      /// returns field at given index
      TGFBField *getField( unsigned int iIndex );

      /// appends a newly created field, usage not recommended
      TGFBField *newField();
      TGFBField *newField( char *n, int dt );
};

#endif // __GFBFields_H

