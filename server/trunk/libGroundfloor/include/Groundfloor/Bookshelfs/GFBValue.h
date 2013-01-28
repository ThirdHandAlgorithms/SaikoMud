
#ifndef __GFBValue_H
#define __GFBValue_H

class TGFBValue;

#include "../Atoms/GFFreeable.h"
#include "../Molecules/GFString.h"
#include "../Molecules/GFProperty.h"

#include "../Atoms/GFDefines.h"


class TGFBValue: public TGFFreeable {
   private:
      TGFBValue& operator=(const TGFBValue* s); // not allowed to make deep copies
   protected:
      TGFString sInternalValue;
      double dInternalValue;
      __int64 iInternalValue;

      void identifyBooleanString();

   public:
      /// initializes to 0-values
      TGFBValue();
      /// initializes and copies value from anotherValue
      TGFBValue( TGFBValue *anotherValue );
      ~TGFBValue();

      /// set to a FLDTYPE_X constant depending on what SET function was used last
      TGFProperty<int> lasttypeset;
      /// precision of double value to reflect in automatically generated string representations
      TGFProperty<int> precision;

      /// sets data to match the data in anotherValue
      void setValue( const TGFBValue *anotherValue );
      /// compares data to anotherValue and returns true if their equal
      bool equals( const TGFBValue *anotherValue ) const;

      /// returns TGFString representation of set value
      TGFString *asString();
      /// returns integer representation of set value
      long asInteger() const;
      /// returns int64 representation of set value
      __int64 asInt64() const;
      /// returns double representation of set value
      double asDouble() const;
      /// returns boolean representation of set value
      bool asBoolean() const;
      /// returns 'timestamp' representation of set value (currently same as asInteger(), but unsigned)
      unsigned int asTimestamp() const;

      /// sets current value to string, tries to automatically extract integer representation and boolean representation
      void setString( const char *sValue );
      /// sets current value to string, tries to automatically extract integer representation and boolean representation
      void setString( const char *sValue, unsigned int iLength );
      /// sets current value to string, tries to automatically extract integer representation and boolean representation
      void setString( const TGFString *sValue );
      /// sets integer value, automatically sets string representation to an ascii translation of the integer
      void setInteger( long iValue );
      /// sets int64 value (and truncated integer value), automatically sets string representation to an ascii translation of the integer (currently only 32bits of the value)
      void setInt64( __int64 iValue );
      /// sets double value, automatically sets string representation to an ascii translation of the double, uses precision for number of decimals, uses '.' as seperator
      void setDouble( double dValue );
      /// sets boolean value (integer 1 when true, 0 when false), automatically sets string representation to 'true' or 'false'
      void setBoolean( bool bValue );
      /// sets timestamp value, automatically sets string representation to standard date format "yyyy-mm-dd hh:mm:ss" or "yyyy-mm-ddThh:mm:ss" when bXml is set true
      void setTimestamp( __int64 iValue, bool bXml = false );



      // operators
      friend void operator<<(std::ostream& Ostr, const TGFBValue* s) {
         Ostr << s->sInternalValue.getValue();
      }
      friend void operator<<(TGFString& Ostr, const TGFBValue* s) {
         Ostr.append( &s->sInternalValue );
      }
/*
      friend void operator<<(TGFBValue& Ostr, const char* s) {
         Ostr.append_ansi( s );
      }
*/
};


#endif // __GFBValue_H

