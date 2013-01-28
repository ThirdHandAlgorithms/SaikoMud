
#ifndef _GFFunctions_H_
#define _GFFunctions_H_

#include "../Molecules/GFVector.h"
#include "../Molecules/GFString.h"
#include "../Atoms/GFDefines.h"


/// returns position of c in string s, or -1 if not found
int strpos( const char *s, char c );

/// converts sInput (binary data) to Base64, overwriting sOutput
void StrToBase64( const TGFString *sInput, TGFString *sOutput );

/// converts sInput (binary data) to Ascii85, overwriting sOutput
void StrToAscii85( const TGFString *sInput, TGFString *sOutput );

/// appends the 2-character-hexadecimal-value of c to sOutput
void CharToHex( char c, TGFString *sOutput );
/// overwrites sOutput with full per-byte hexidecimal values, values per byte are not seperated
void StrToHex( const TGFString *sInput, TGFString *sOutput );

/// overwrites sOutput with full bit-to-ascii conversion of sInput, set bits are translated into '1', unset bits are '0'
/// Bits are ordered from MSB to LSB (big endian).
void StrToBin( const TGFString *sInput, TGFString *sOutput );

/// prepends the hexadecimal ascii representation of i, length depends on compiler implementation of long (32 bits or 64 bits)
void LongToHex( unsigned long i, TGFString *sOutput );
/// returns the integer value represented in the hexadecimal string sInput
unsigned int HexToInt( const TGFString *sInput );

/// simple implementation of compressing sInput to binary tree sOutputData and per-node translation sOutputTable.
/// only compresses single characters/bytes.
//void StrToCharHuffman( const TGFString *sInput, TGFString *sOutputData, TGFString *sOutputTable );
void StrToCharHuffman( const TGFString *sInput, TGFString *sOutputData, TGFString *sOutputTable1, TGFString *sOutputTable2 );

/// returns original string from binary tree data and translation table
void CharHuffmanToStr( const TGFString *sInputData, const TGFString *sInputTable1, const TGFString *sInputTable2, TGFString *sOutputString );

void GFEncodeHtmlEntities( TGFString *s );
void GFDecodeHtmlEntities( TGFString *s );

////////////////////////////////////////////////////////////////////////////////

/// returns filesize of sFilePath, assuming UTF8 string
unsigned long GFGetFileSize( const TGFString *sFilePath );

/// returns true if file exists, assuming UTF8 string
bool GFFileExists( const TGFString *sFilePath );
/// returns true if path is a directory, assuming UTF8 string
bool GFIsDirectory( const TGFString *sFilePath );

/// returns current directory represented as UTF8 string
TGFString *GFGetCurrentDirectory();

TGFString *createHybridString( const void *s, bool bWide );
void GFPrependNativeSlash( TGFString *sPath, bool bWide = false );
void GFAppendNativeSlash( TGFString *sPath, bool bWide = false );
void GFConvertToNativeSlashes( TGFString *sPath, bool bWide = false );

void GFConvertToURISlashes( TGFString *sPath, bool bWide = false );

/// returns timestamp representing the current date and time
__int64 GFGetTimestamp();
/// translates string into timestamp, acceptable formats are "hh:mm", "hh:mm:ss", "yyyy-mm-dd", "yyyy-mm-dd hh:mm" or "yyyy-mm-dd hh:mm:ss"
/** bTZCorrection can be optionally set to False to Not include a correction that counters the behaviour - of the functions that are used - to translate the timestamp to the local timezone.
     unless you know what you're doing, you should leave these at their default True values to get proper date translations that are compatible with TGFBValue's dates and timestamps */
__int64 GFDateStrToTimestamp( TGFString *sDate, bool bTZCorrection = true, bool bDaylightSavingsCorrection = true );


//------------------------------------------------------------------------------

/// Splits given string sHaystack into new strings that are stored in a new TGFVector,
///  manual deletion of returned vector is required.
TGFVector *GFsplit( const TGFString *sHaystack, const char *sNeedle, int iMax = -1 );
/// Determines whether given string sNeedle solely contains numbers between 0 and 9
bool GFisNumber( const char *sNeedle, unsigned int len );

/// returns the length in bytes of given widestring
unsigned int getWStrByteLen( const wchar_t *s );


#endif //_GFFunctions_H_
