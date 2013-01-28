
#ifndef __GFStringVector_H
#define __GFStringVector_H

#include "GFString.h"
#include "GFVector.h"

class TGFStringVectorRange: public TGFFreeable {
   protected:
   public:
      TGFStringVectorRange( unsigned int a = 0, unsigned long b = 0, unsigned int c = 0, unsigned long d = 0 );
      TGFStringVectorRange( TGFStringVectorRange *aRange );
      ~TGFStringVectorRange();

      void copyValues( TGFStringVectorRange *aRange );

      unsigned int start_ind;
      unsigned long start_pos;
      unsigned int end_ind;
      unsigned long end_pos;
};


/** TGFStringVector is a dynamic string array with autoresize behaviour.
 *  It provides a scale of functions that allow multiple strings
 *   to be seen as One, without requiring a single large memory block.
 *  This class will be of benefit when dealing with large chunks of data
 *   that need to be in memory for process.
 *  It also allows hypothetically a larger amount data then a normal TGFString
 *   can store, however this remains untested.
 *
 *  Please note that this class does NOT natively support thread-safety, but
 *   will not require outside locking either when used in a normal fassion.
 *  Outside locking is recommended though when you know the thread is being
 *   written to and read from by seperate threads.
 *
 *  Note that several functions on this class use entirely different arguments
 *   and return values than most functions in the Groundfloor library.
 */
class TGFStringVector: public TGFVector {
   protected:
   public:
      TGFStringVector();
      ~TGFStringVector();

      /// Appends a string onto the Vector using addElement()
      /** Terminology for added strings are Chunks, hench the name.
        * Strings are not copied, but added as pointer that will be Freed
        *  automatically on deletion of the vector when the autoClear default
        *  has not been changed.
        **/
      void addChunk( TGFString *pString );

      /// Retreives the pointer to the TGFString given the specified chunkindex.
      /** Ranges from 0 to size() minus one **/
      TGFString *getChunk( unsigned int iChunkIndex );

      /// Creaties a new TGFString combined from various chunks.
      /** Chunkindex ranges from 0 to size() minus one. iStartPos is the starting
        *  position from where the stringcopy must commence. iStartPos can be
        *  anywhere situated in the starting Chunk or a chunk after that.
        *  iLength is the specified length of the string to be created and thus
        *  also the range from which must be copied. If the given iLength is
        *  beyond the current content of the StringVector, it will be truncated
        *  and can be compared with the TGFString->getLength() function.
        * The created TGFString is not freed automatically.
        **/
      TGFString *copy( unsigned int iChunkIndex, unsigned long iStartPos, unsigned int iLength );

      /// Creates a new TGFString combined from various chunks.
      /** This copy function is intended to work with the data given back by
        *  the pos() function. Using specific chunkindices and positions, a
        *  new TGFString will include all data between and on the given Start
        *  and End indices. Do not confuse the position 0 with a length of 0,
        *  a position has a length of 1 as in a regular TGFString. Given positions
        *  do not have to be in range of the given chunks. The length of the
        *  string is dynamically allocated, which makes this function more
        *  cpu intensive than the first copy function.
        * The created TGFString is not freed automatically.
        **/
      TGFString *copy( TGFStringVectorRange *aRange );

      /// Determines the starting and end position of a given Needle.
      /** This functions uses a pointer to a user allocated TStringVectorRange for
        *  index storage. The start_ind and start_pos variables are [in,out] recycled
        *  to return the starting point of the Needle. The end_ind and end_pos
        *  variables are [out] only used to return the end of the needle, while
        *  search range is determined by the function itself.
        *
        * True is returned when the Needle has been found, all integers with
        *  pointers are filled with positioning data.
        *
        * False is returned when the Needle has not been found, integers
        *  will remain unaltered.
        **/
      bool pos( TGFStringVectorRange *aRange, const char *sNeedle, unsigned int iNeedleLen );

      bool pos_ansi( TGFStringVectorRange *aRange, const char *sNeedle );

      /// Returns the total accummulated length from iStartChunk till the end
      ///  of the last chunk of data.
      unsigned long getLength( unsigned int iStartChunk );

      /// Returns the chunk index and position to a given flat position
      TGFStringVectorRange *translatePosToChunkedIndex( unsigned long iPos );

      /// special function that subtracts 1 from start_pos and checks for underflow translates it to a valid position if needed
      void startMinusOne( TGFStringVectorRange *range );
      /// special function that subtracts 1 from end_pos and checks for underflow translates it to a valid position if needed
      void endMinusOne( TGFStringVectorRange *range );
};

#endif
