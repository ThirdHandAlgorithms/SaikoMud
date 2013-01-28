
#ifndef __GFFileWriter_H
#define __GFFileWriter_H


#include "GFThreadedBuffer.h"
#include "../Atoms/GFFreeable.h"
#include "../Molecules/GFString.h"
#include "../Materials/GFFileCommunicator.h"
#include "../Molecules/GFStringVector.h"

class TGFBaseWriter: public TGFThreadedBuffer {
   public:
      TGFBaseWriter();
      ~TGFBaseWriter();

      /// obj is assumed to be a TGFString that is copied to a new TGFString and placed in the buffer
      void add( TGFFreeable *obj );
};

class TGFStringWriter: public TGFBaseWriter {
   public:
      TGFStringWriter();
      ~TGFStringWriter();

      TGFStringVector Output;

      bool processObject( TGFFreeable *obj );
};

/** TGFFileWriter is a Threaded Buffer that periodically writes
  *  data from the buffer to a file.
  * Warning: if you prematurely delete this object, data might be lossed.
  *  To prevent loss of data, implement a protection loop by e.g.: while ( !filewriter.isEmpty() ) { GFMilliSleep(100); );
 */
class TGFFileWriter: public TGFBaseWriter {
   protected:
      TGFFileCommunicator file;
      bool bAppendmode;

   public:
      TGFFileWriter();
      ~TGFFileWriter();

      /// opens file in Write mode (or Append mode if bAppendMode is true), sFile must be in UTF8
      void open( const TGFString *sFile, bool bAppendmode = false );
      /// opens file in Write mode (or Append mode if bAppendMode is true), sFile must be in UTF8
      void open( const char *sFile, bool bAppendmode = false );
      /// closes file (warning: does not automatically flush buffer!)
      void close();

      bool processObject( TGFFreeable *obj );
};


#endif // __GFFileWriter_H

