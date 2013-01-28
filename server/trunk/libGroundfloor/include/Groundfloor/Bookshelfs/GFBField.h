
#ifndef __GFBField_H
#define __GFBField_H

class TGFBField;

#include "../Atoms/GFFreeable.h"
#include "../Molecules/GFString.h"
#include "../Molecules/GFProperty.h"

#define FLDTYPE_STRING 0
#define FLDTYPE_INT 1
#define FLDTYPE_DOUBLE 2
#define FLDTYPE_DATE 3
#define FLDTYPE_INT64 4
#define FLDTYPE_BOOL 5
#define FLDTYPE_UNKNOWN 255

#define FLDFLAG_NORMAL 0
#define FLDFLAG_PRIMARY 1


class TGFBField: public TGFFreeable {
   public:
      /// intializes field to unnamed, datatype -1, no maximum length, flagged normal
      TGFBField();
      TGFBField( char *n, int dt );
      ~TGFBField();

      /// name of field
      TGFString name;
      /// datatype of data
      TGFProperty<int> datatype;
      /// maximum length of data, set to -1 when no maximum length needed
      TGFProperty<int> maxlength;
      /// flags belonging to this field
      TGFProperty<int> flags;
};

#endif // __GFBField_H

