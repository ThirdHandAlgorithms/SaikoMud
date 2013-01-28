
#include "GFString.h"

#include <cstring>
#include <cstdlib>

#include <cstdio>

#define C_CASEDIFF   32

#include "locale.h"


#ifndef __BORLANDC__
   // shameslessly copy pasted from BORLAND RTL Sourcecode :/
   #include <ctype.h>

   int custom_caseinsensitive_stringcomparison( const char *str1, const char *str2, size_t maxlen ) {
      size_t iMax = maxlen;
      while ((maxlen != 0) && (*str1 != '\0') && toupper(*str1) == toupper(*str2) ) {
         maxlen--;
         str1++;
         str2++;
      }

      if (maxlen == 0) {
         return (0);
      } else {
         return (toupper(*str1) - toupper(*str2));
      }
   }
#endif

/*
         #define nativestringcompare_length_casesensitive(a,b,c)       strcasecmp(a,b,c)
         #define nativestringcompare_length_caseinsensitive(a,b,c)     strncasecmp(a,b,c)
      #else
         #if _MSC_VER >= 1400
            #define nativestringcompare_length_casesensitive(a,b,c)       strncmp(a,b,c)
            #define nativestringcompare_length_caseinsensitive(a,b,c)     _strnicmp(a,b,c)
         #else
            #define nativestringcompare_length_casesensitive(a,b,c)       strncmp(a,b,c)
            #define nativestringcompare_length_caseinsensitive(a,b,c)     custom_caseinsensitive_stringcomparison(a,b,c)
         #endif
      #endif
*/

#define nativestringcompare_length_casesensitive(a,b,c)       strncmp(a,b,c)
#define nativestringcompare_length_caseinsensitive(a,b,c)     custom_caseinsensitive_stringcomparison(a,b,c)

#ifndef min
   #define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include <wchar.h>

void fakeAnsiToWide( const char *ansistr, wchar_t *widestr, size_t ansistrlen ) {
   for ( unsigned int i = 0; i < ansistrlen; i++ ) {
      widestr[i] = ansistr[i];
   }
}

void fakeWideToAnsi( const wchar_t *widestr, char *ansistr, size_t widestrleninbytes ) {
   size_t ansistrlen = widestrleninbytes / sizeof(wchar_t);
   for ( unsigned int i = 0; i < ansistrlen; i++ ) {
      ansistr[i] = static_cast<char>( widestr[i] );
   }
}


#if defined(__STRICT_ANSI__) || defined(GF_OS_LINUX)
wchar_t *_wcslwr( wchar_t *sValue ) {
   unsigned int l = wcslen(sValue);
   for ( unsigned int i = 0; i < l; i++ ) {
      wchar_t c = sValue[i];
      if ( (c >= 'A') && (c <= 'Z') ) {
         sValue[i] = c + 32;
      }
   }
   return sValue;
}
wchar_t *_wcsupr( wchar_t *sValue ) {
   unsigned int l = wcslen(sValue);
   for ( unsigned int i = 0; i < l; i++ ) {
      wchar_t c = sValue[i];
      if ( (c >= 'a') && (c <= 'z') ) {
         sValue[i] = c - 32;
      }
   }
   return sValue;
}
#endif


bool bytewisematch_reference( const char *s1, const char *s2, unsigned int iLen ) {
   for ( unsigned int i = 0; i < iLen; i++ ) {
      if ( s1[i] != s2[i] ) {
         return false;
      }
   }

   return true;
}

bool bytewisematch( const char *s1, const char *s2, unsigned int iLen ) {
   const long *a1 = reinterpret_cast<const long*>(s1);
   const long *a2 = reinterpret_cast<const long*>(s2);
   unsigned int c = iLen / sizeof(long);
   for ( unsigned int i = 0; i < c; i++ ) {
      if ( a1[i] != a2[i] ) {
         return false;
      }
   }

   unsigned int d = c * sizeof(long);
   for ( unsigned int j = d; j < iLen; j++ ) {
      if ( s1[j] != s2[j] ) {
         return false;
      }
   }

   return true;
}

//------------------------------------------------------------------------------
// Constructors/destructor
//------------------------------------------------------------------------------
TGFString::TGFString() : TGFFreeable() {
   this->value = NULL;
   this->strlength = 0;
   this->size = 0;

   setValue( "", 0 );
}

TGFString::TGFString( const char *sValue, unsigned int iLength ) : TGFFreeable() {
   this->value = NULL;
   this->strlength = 0;
   this->size = 0;

   setValue( sValue, iLength );
}

TGFString::TGFString( const TGFString *sValue ) : TGFFreeable() {
   this->value = NULL;
   this->strlength = 0;
   this->size = 0;

   setValue( sValue->value, sValue->strlength );
}

TGFString::TGFString( const char *sValue ) : TGFFreeable() {
   this->value = NULL;
   this->strlength = 0;
   this->size = 0;

   setValue( sValue, strlen( sValue ) );
}

TGFString::TGFString( const wchar_t *sValue ) : TGFFreeable() {
   this->value = NULL;
   this->strlength = 0;
   this->size = 0;

   setValue( reinterpret_cast<const char *>( sValue ), wcslen(sValue) * sizeof(wchar_t) );
}

TGFString::~TGFString() {
   if ( this->value != NULL ) {
      GFFree( this->value );
   }
}


//------------------------------------------------------------------------------
// match
//------------------------------------------------------------------------------
bool TGFString::match( const char *sNeedle, unsigned int l ) const {
/*
   unsigned int c = max( l, this->strlength );
   return ( strncmp( this->value, sNeedle, c ) == 0 );
*/
   if ( this->strlength == l ) {
      return bytewisematch( this->value, sNeedle, l );
   }

   return false;
}

bool TGFString::match( const TGFString *sNeedle ) const {
/*
   unsigned int c = max( sNeedle->getLength(), this->strlength );
   return ( strncmp( this->value, sNeedle->getValue(), c ) == 0 );
*/
   if ( this->strlength == sNeedle->getLength() ) {
      return bytewisematch( this->value, sNeedle->getValue(), this->strlength );
   }

   return false;
}

bool TGFString::match_ansi( const char *sNeedle ) const {
/*
   unsigned int c = max( strlen(sNeedle), this->strlength );
   return ( strncmp( this->value, sNeedle, c ) == 0 );
*/
   if ( this->strlength == strlen(sNeedle) ) {
      return bytewisematch( this->value, sNeedle, this->strlength );
   }

   return false;
}

bool TGFString::match_wide( const wchar_t *sNeedle ) const {
/*
   unsigned int c = max( wcslen(sNeedle) * sizeof(wchar_t), this->strlength );
   return ( strncmp( this->value, reinterpret_cast<const char *>(sNeedle), c ) == 0 );
*/
   if ( this->strlength == wcslen(sNeedle) * sizeof(wchar_t) ) {
      return bytewisematch( this->value, reinterpret_cast<const char *>(sNeedle), this->strlength );
   }

   return false;
}

//------------------------------------------------------------------------------
// startsWith
//------------------------------------------------------------------------------
bool TGFString::startsWith( const char *sNeedle, unsigned int l ) const {
   if ( l <= this->strlength ) {
      return ( nativestringcompare_length_caseinsensitive( this->value, sNeedle, l ) == 0 );
   }
   return false;
}

bool TGFString::startsWith( const TGFString *sNeedle ) const {
  return startsWith( sNeedle->getValue(), sNeedle->getLength() );
}

bool TGFString::startsWith_ansi( const char *sNeedle ) const {
   //   return ( strncmp( this->value, sNeedle, strlen(sNeedle) ) == 0 );
   // case insensitive (alleen voor Win32) -> nu niet meer, zie private function
   unsigned int c = strlen(sNeedle);
   if ( c <= this->strlength ) {
      return ( nativestringcompare_length_caseinsensitive( this->value, sNeedle, strlen(sNeedle) ) == 0 );
   }

   return false;
}


//------------------------------------------------------------------------------
// endsWith
//------------------------------------------------------------------------------
bool TGFString::endsWith( const char *sNeedle, unsigned int l ) const {
   if ( l <= this->strlength ) {
      return ( nativestringcompare_length_caseinsensitive( this->value + (this->strlength - l), sNeedle, l ) == 0 );
   }

   return false;
}

bool TGFString::endsWith( const TGFString *sNeedle ) const {
   return endsWith( sNeedle->getValue(), sNeedle->getLength() );
}

bool TGFString::endsWith_ansi( const char *sNeedle ) const {
   unsigned int l = strlen(sNeedle);
   if ( l <= this->strlength ) {
      return ( nativestringcompare_length_caseinsensitive( this->value + (this->strlength - l), sNeedle, l ) == 0 );
   }

   return false;
}


//------------------------------------------------------------------------------
// pos
//------------------------------------------------------------------------------
int TGFString::pos( unsigned int iStart, const char *sNeedle, unsigned int iNeedleLen ) const {
   unsigned int c = 0;
   unsigned int iReturnPos = 0;

   unsigned int i = iStart;
   while ( i != this->strlength ) {
      if ( this->value[i] == sNeedle[c] ) {
         if ( c == 0 ) {
            iReturnPos = i;
         }
         c++;
      } else {
         if ( c != 0 ) {
            i = iReturnPos;
            c = 0;
         }
      }

      if ( c == iNeedleLen ) {
         return iReturnPos;
      }

      i++;
   }

   return -1;
}

int TGFString::pos( unsigned int iStart, const TGFString *sNeedle ) const {
   return pos( iStart, sNeedle->getValue(), sNeedle->getLength() );
}

int TGFString::pos_ansi( unsigned int iStart, const char *sNeedle ) const {
   return pos( iStart, sNeedle, strlen( sNeedle ) );
}
int TGFString::pos_ansi( const char *sNeedle ) const {
   return pos( 0, sNeedle, strlen( sNeedle ) );
}


//------------------------------------------------------------------------------
// lastpos
//------------------------------------------------------------------------------
int TGFString::lastpos( const char *sNeedle, unsigned int l ) const {
   int iLastPos = -1;
   int iPos = pos( 0, sNeedle, l );
   while ( iPos != -1 ) {
      iLastPos = iPos;
      iPos = pos( iLastPos + 1, sNeedle, l );
   }
   return iLastPos;
}

int TGFString::lastpos( const TGFString *sNeedle ) const {
   return lastpos( sNeedle->getValue(), sNeedle->getLength() );
}

int TGFString::lastpos_ansi( const char *sNeedle ) const {
   return lastpos( sNeedle, strlen(sNeedle) );
}

//------------------------------------------------------------------------------
// getValue
//------------------------------------------------------------------------------
char *TGFString::getValue() const {
   return this->value;
}

wchar_t *TGFString::getValueAsWide() const {
   return reinterpret_cast<wchar_t *>( this->value );
}

char *TGFString::getPointer( unsigned int iPosition ) const {
   return ( this->value + iPosition );
}


//------------------------------------------------------------------------------
// setValue
//------------------------------------------------------------------------------
bool TGFString::setValue( const char *sValue, unsigned int iLength ) {
   char *newstr = static_cast<char *>(GFMalloc( iLength + 4 ));

   if ( newstr != NULL ) {
      void *ret = memcpy( newstr, sValue, iLength );
      newstr[iLength]   = '\0';
      newstr[iLength+1] = '\0';
      newstr[iLength+2] = '\0';
      newstr[iLength+3] = '\0';
   } else {
      return false;
   }

   char *oldValue = this->value;

   this->value       = newstr;
   this->strlength   = iLength;
   this->size        = iLength;

   if ( oldValue != NULL ) {
      GFFree( oldValue );
   }

   return true;
}

bool TGFString::setValue( const TGFString *sValue ) {
   return setValue( sValue->value, sValue->strlength );
}

bool TGFString::setValue_ansi( const char *sValue ) {
   return setValue( sValue, strlen( sValue ) );
}

bool TGFString::setValue_wide( const wchar_t *sValue ) {
   return setValue( reinterpret_cast<const char *>(sValue), wcslen( sValue ) * sizeof(wchar_t) );
}


//------------------------------------------------------------------------------
// append
//------------------------------------------------------------------------------
bool TGFString::append( const char *sValue, unsigned int iLength ) {
   unsigned int newlen = this->strlength + iLength;

   if ( newlen != 0 ) {
      if ( this->size < newlen ) {
         if ( !setSize( newlen ) ) {
   			return false;
   		}
      }

      memcpy( this->value + this->strlength, sValue, iLength );

      this->strlength = newlen;

      this->value[this->strlength]   = '\0';
      this->value[this->strlength+1] = '\0';
      this->value[this->strlength+2] = '\0';
      this->value[this->strlength+3] = '\0';
   }

	return true;
}

bool TGFString::append( const TGFString *sValue ) {
   return append( sValue->value, sValue->strlength );
}

bool TGFString::append( const TGFString *sValue, unsigned int iLength ) {
   return append( sValue->value, iLength );
}

bool TGFString::append_ansi( const char *sValue ) {
   return append( sValue, strlen( sValue ) );
}

bool TGFString::append_wide( const wchar_t *sValue ) {
   return append( reinterpret_cast<const char *>( sValue ), wcslen( sValue ) * sizeof(wchar_t) );
}
bool TGFString::append( char aChar ) {
   char sTmp[] = { aChar };

   return append( sTmp, 1 );
}


//------------------------------------------------------------------------------
// prepend
//------------------------------------------------------------------------------
bool TGFString::prepend( const char *sValue, unsigned int iLength ) {
   unsigned int newlen = this->strlength + iLength;

   if ( newlen <= 0x3FFFFFFF ) {
		char *newstr = static_cast<char *>(GFMalloc( newlen + 4 ));

		if ( newstr != 0 ) {
			memcpy( newstr, sValue, iLength );

			char *oldvalue = this->value;

			if ( oldvalue != NULL ) {
				memcpy( newstr + iLength, oldvalue, this->strlength );
				newstr[newlen]     = '\0';
				newstr[newlen + 1] = '\0';
            newstr[newlen + 2] = '\0';
            newstr[newlen + 3] = '\0';

            this->value = newstr;
				GFFree( oldvalue );
			} else {
            this->value = newstr;
         }

			this->strlength = newlen;
			this->size = newlen;

			return true;
		}
	}

	return false;
}

bool TGFString::prepend( const TGFString *sValue ) {
   return prepend( sValue->value, sValue->strlength );
}

bool TGFString::prepend( const TGFString *sValue, unsigned int iLength ) {
   return prepend( sValue->value, iLength );
}

bool TGFString::prepend( char aChar ) {
   char sTmp[] = { aChar };

   return prepend( sTmp, 1 );
}

bool TGFString::prepend_ansi( const char *sValue ) {
   return prepend( sValue, strlen( sValue ) );
}


//------------------------------------------------------------------------------
// remove
//------------------------------------------------------------------------------

bool TGFString::remove( unsigned int iStartPos, unsigned int iEndPos ) {
    if ( iEndPos >= iStartPos ) {
        if ( iEndPos < this->strlength ) {

            unsigned int iNewLen = strlength - (iEndPos - iStartPos + 1);

            memcpy( this->value + iStartPos, this->value + iEndPos + 1, strlength - iEndPos - 1 );

            this->strlength = iNewLen;
            value[iNewLen]     = '\0';
            value[iNewLen + 1] = '\0';
            value[iNewLen + 2] = '\0';
            value[iNewLen + 3] = '\0';

            return true;
        }
    }

    return false;
}


//------------------------------------------------------------------------------
// replace
//------------------------------------------------------------------------------
bool TGFString::replace( const TGFString *sSubStringA, const TGFString *sSubStringB ) {
	bool bSucces = true;
   unsigned int iLenA = sSubStringA->getLength();
   unsigned int iLenB = sSubStringB->getLength();

   int iPosA;

   TGFString *sTempBuffer = new TGFString();
   sTempBuffer->setSize( this->strlength );

   iPosA = this->pos( 0, sSubStringA );
   while ( iPosA != -1 ) {
      unsigned int iLenC = this->strlength - iPosA - iLenA;

      sTempBuffer->setLength( 0 );
      sTempBuffer->append( this->value + iPosA + iLenA, iLenC );

      if ( !setLength( this->strlength - iLenA + iLenB ) ) {
			bSucces = false;
			break;
		}

      memcpy( this->value + iPosA + iLenB, sTempBuffer->value, iLenC );

      memcpy( this->value + iPosA, sSubStringB->value, iLenB );

      iPosA = this->pos( iPosA + iLenB, sSubStringA );
   }

   delete sTempBuffer;

	return true;
}

bool TGFString::replace_ansi( const char *sSubStringA, const char *sSubStringB ) {
	bool bSucces = true;
   unsigned int iLenA = strlen( sSubStringA );
   unsigned int iLenB = strlen( sSubStringB );

   int iPosA;

   TGFString *sTempBuffer = new TGFString();
   sTempBuffer->setSize( this->strlength );

   iPosA = this->pos_ansi( 0, sSubStringA );
   while ( iPosA != -1 ) {
      unsigned int iLenC = this->strlength - iPosA - iLenA;

      sTempBuffer->setLength( 0 );
      sTempBuffer->append( this->value + iPosA + iLenA, iLenC );

      if ( !setLength( this->strlength - iLenA + iLenB ) ) {
			bSucces = false;
			break;
		}

      memcpy( this->value + iPosA + iLenB, sTempBuffer->value, iLenC );

      memcpy( this->value + iPosA, sSubStringB, iLenB );

      iPosA = this->pos_ansi( iPosA + iLenB, sSubStringA );
   }

   delete sTempBuffer;

	return true;
}




//------------------------------------------------------------------------------
// setSize
//------------------------------------------------------------------------------

bool TGFString::setSize_old( unsigned int iSize ) {
   if ( iSize <= 0x3FFFFFFF ) {
      char *newstr = static_cast<char *>(GFMalloc( iSize + 4 ));

      if ( newstr > 0 ) {
         char *oldvalue = this->value;

         if ( oldvalue != NULL ) {
            memcpy( newstr, oldvalue, min( this->strlength, iSize ) );

            newstr[iSize]     = '\0';
            newstr[iSize + 1] = '\0';
            newstr[iSize + 2] = '\0';
            newstr[iSize + 3] = '\0';

            this->value = newstr;
            GFFree( oldvalue );
         } else {
            this->value = newstr;
         }

         this->size = iSize;
         this->strlength = min( this->strlength, iSize );

         return true;
      } else {
         // not enough memory
      }
   } else {
      printf( "TGFString Warning: Memory allocation too large, please use TGFStringVector instead.\n" );
   }

   return false;
}

bool TGFString::setSize( unsigned int iSize ) {
   if ( iSize <= 0x3FFFFFFF ) {
      if ( value != NULL ) {
         this->value = reinterpret_cast<char *>( GFRealloc( this->value, iSize + 4 ) );
      } else {
         this->value = reinterpret_cast<char *>( GFMalloc( iSize + 4 ) );
      }

      this->value[iSize]     = '\0';
      this->value[iSize + 1] = '\0';
      this->value[iSize + 2] = '\0';
      this->value[iSize + 3] = '\0';

      this->size = iSize;
      this->strlength = min( this->strlength, iSize );

      return true;
   } else {
      printf( "TGFString Warning: Memory allocation too large, please use TGFStringVector instead.\n" );
   }

   return false;
}


//------------------------------------------------------------------------------
// setLength
//------------------------------------------------------------------------------
bool TGFString::setLength( unsigned int iLength ) {
   bool bSucces = true;

   if ( this->size < iLength ) {
      bSucces = setSize( iLength );
   }

   if ( bSucces ) {
      this->value[iLength]       = '\0';
      this->value[iLength + 1]   = '\0';
      this->value[iLength + 2]   = '\0';
      this->value[iLength + 3]   = '\0';

      this->strlength = iLength;
   }

   return bSucces;
}


//------------------------------------------------------------------------------
// getLength
//------------------------------------------------------------------------------
unsigned int TGFString::getLength() const {
   return this->strlength;
}

unsigned int TGFString::getSize() const {
   return this->size;
}


//------------------------------------------------------------------------------
// trim
//------------------------------------------------------------------------------
void TGFString::rtrim_ansi( bool bOnlyRemoveCrlf ) {
   int i = 0;

   if ( this->strlength != 0 ) {
      if ( bOnlyRemoveCrlf ) {
          for ( i = this->strlength - 1; i >= 0; i-- ) {
             if ( ( this->value[i] != '\r' ) && ( this->value[i] != '\n' ) && ( this->value[i] != 0 ) ) {
                break;
             }
          }
      } else {
          for ( i = this->strlength - 1; i >= 0; i-- ) {
             if ( ( this->value[i] != ' ' ) && ( this->value[i] != '\r' ) && ( this->value[i] != '\n' ) && ( this->value[i] != '\t' ) && ( this->value[i] != 0 ) ) {
                break;
             }
          }
      }

      this->value[i+1] = 0;
      this->strlength = i + 1;
   }
}

void TGFString::ltrim_ansi() {
   unsigned int i = 0;

   if ( this->strlength != 0 ) {
      for ( i = 0; i < this->strlength; i++ ) {
         if ( ( this->value[i] != ' ' ) && ( this->value[i] != '\r' ) && ( this->value[i] != '\n' ) && ( this->value[i] != '\t' ) && ( this->value[i] != 0 ) ) {
            break;
         }
      }

      this->setValue( &this->value[i], this->strlength - i );
   }
}


//------------------------------------------------------------------------------
// case
//------------------------------------------------------------------------------
void TGFString::uppercase_ansi() {
   for ( unsigned int i = 0; i < strlength; i++ ) {
      char c = value[i];
      if ( ( c >= 'a' ) && ( c <= 'z' ) ) {
         value[i] = c - C_CASEDIFF;
      }
   }
}

void TGFString::lowercase_ansi() {
   for ( unsigned int i = 0; i < strlength; i++ ) {
      char c = value[i];
      if ( ( c >= 'A' ) && ( c <= 'Z' ) ) {
         value[i] = c + C_CASEDIFF;
      }
   }
}

void TGFString::uppercase_wide() {
   _wcsupr( reinterpret_cast<wchar_t *>(value) );
}

void TGFString::lowercase_wide() {
   _wcslwr( reinterpret_cast<wchar_t *>(value) );
}


//------------------------------------------------------------------------------
// transform
//------------------------------------------------------------------------------
bool TGFString::transformToWidestring( bool bCurrentIsUtf8, bool bFakeIfFailed ) {
   unsigned int iNewLen = this->strlength * sizeof(wchar_t);
   char *newstr = static_cast<char *>(GFMalloc( iNewLen + 4 ));

   if ( newstr > 0 ) {
      char *oldvalue = this->value;

      if ( oldvalue != NULL ) {
         size_t convertedChars = 0;

         #ifdef GF_OS_LINUX
            if ( bCurrentIsUtf8 ) {
               char oldLocale[128] = "";
               sprintf( oldLocale, "%s", setlocale( LC_CTYPE, NULL ) );
               setlocale( LC_CTYPE, ".UTF-8" );
               convertedChars = mbstowcs( reinterpret_cast<wchar_t *>( newstr ), oldvalue, this->strlength );
               setlocale( LC_CTYPE, oldLocale );
            } else {
               convertedChars = mbstowcs( reinterpret_cast<wchar_t *>( newstr ), oldvalue, this->strlength );
            }
         #else
         // well, setlocale doesn't work on windows, so...
         if ( bCurrentIsUtf8 ) {
            convertedChars = MultiByteToWideChar( CP_UTF8, 0, oldvalue, strlength, reinterpret_cast<wchar_t *>( newstr ), strlength );
         } else {
            convertedChars = mbstowcs( reinterpret_cast<wchar_t *>( newstr ), oldvalue, this->strlength );
         }
         #endif
         if ( ( convertedChars <= 0 ) && (strlength != 0) ) {
            if ( bFakeIfFailed ) {
               convertedChars = strlength;
               fakeAnsiToWide( oldvalue, reinterpret_cast<wchar_t *>( newstr ), strlength );
            } else {
               convertedChars = 0;
               GFFree( newstr );
               return false;
            }
         }
         iNewLen = convertedChars * sizeof(wchar_t);

         newstr[iNewLen]     = '\0';
         newstr[iNewLen + 1] = '\0';
         newstr[iNewLen + 2] = '\0';
         newstr[iNewLen + 3] = '\0';

         this->value = newstr;
         GFFree( oldvalue );
      } else {
         this->value = newstr;
      }

      this->size = iNewLen;
      this->strlength = iNewLen;

      return true;
   } else {
   // not enough memory
   }

   return false;
}

bool TGFString::transformWidestringToString( bool bForceToUtf8, bool bFakeIfFailed ) {
   unsigned int iNewLen = this->strlength;
   char *newstr = static_cast<char *>(GFMalloc( iNewLen + 4 ));

   if ( newstr > 0 ) {
      char *oldvalue = this->value;

      if ( oldvalue != NULL ) {
         size_t convertedChars = 0;

         #ifdef GF_OS_LINUX
            if ( bForceToUtf8 ) {
               char oldLocale[128] = "";
               sprintf( oldLocale, "%s", setlocale( LC_CTYPE, NULL ) );
               setlocale( LC_CTYPE, ".UTF-8" );
               convertedChars = wcstombs( reinterpret_cast<char *>( newstr ), reinterpret_cast<const wchar_t *>(oldvalue), this->strlength );
               setlocale( LC_CTYPE, oldLocale );
            } else {
               convertedChars = wcstombs( reinterpret_cast<char *>( newstr ), reinterpret_cast<const wchar_t *>(oldvalue), this->strlength );
            }
         #else
            // well, setlocale doesn't work on windows, so...
            if ( bForceToUtf8 ) {
               convertedChars = WideCharToMultiByte( CP_UTF8, 0, reinterpret_cast<wchar_t *>( oldvalue ), this->strlength / sizeof(wchar_t), reinterpret_cast<char *>( newstr ), this->strlength, NULL, NULL );
            } else {
               convertedChars = wcstombs( reinterpret_cast<char *>( newstr ), reinterpret_cast<const wchar_t *>(oldvalue), this->strlength );
            }
         #endif
         if ( ( convertedChars <= 0 ) && (strlength != 0) ) {
            if ( bFakeIfFailed ) {
               convertedChars = strlength;
               fakeWideToAnsi( reinterpret_cast<wchar_t *>( newstr ), oldvalue, strlength );
            } else {
               convertedChars = 0;
               GFFree( newstr );
               return false;
            }
         }

         iNewLen = convertedChars;

         newstr[iNewLen]     = '\0';
         newstr[iNewLen + 1] = '\0';
         newstr[iNewLen + 2] = '\0';
         newstr[iNewLen + 3] = '\0';

         this->value = newstr;
         GFFree( oldvalue );
      } else {
         this->value = newstr;
      }

      this->size = iNewLen;
      this->strlength = iNewLen;

      return true;
   } else {
   // not enough memory
   }

   return false;
}

//------------------------------------------------------------------------------

// overloaded = operators

TGFString& TGFString::operator=(const char* s) {
   this->setValue_ansi( s );
   return *this;
}

TGFString& TGFString::operator=(const wchar_t* s) {
   this->setValue_wide( s );
   return *this;
}

// overloaded << operators

TGFString& TGFString::operator<<(const TGFString *s ) {
   this->append( s );
   return *this;
}

TGFString& TGFString::operator<<(const char *s ) {
   this->append_ansi( s );
   return *this;
}

TGFString& TGFString::operator<<(const wchar_t *s ) {
   this->append_wide( s );
   return *this;
}

// friends overloaded << operators

std::ostream& operator<<(std::ostream& Ostr, TGFString s) {
   Ostr << s.getValue();
   return Ostr;
}

std::ostream& operator<<(std::ostream& Ostr, TGFString* s) {
   Ostr << s->getValue();
   return Ostr;
}

//------------------------------------------------------------------------------
// eof
//------------------------------------------------------------------------------
