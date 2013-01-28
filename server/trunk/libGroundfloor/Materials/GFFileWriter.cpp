
#include "GFFileWriter.h"



//-----------------------------------------------------------------------------

TGFBaseWriter::TGFBaseWriter() : TGFThreadedBuffer() {
   this->aBuffer->resizeVector( 100 );
}

TGFBaseWriter::~TGFBaseWriter() {
}

void TGFBaseWriter::add( TGFFreeable *obj ) {
   TGFString *sDataCopy = new TGFString( static_cast<TGFString *>(obj) );

   TGFThreadedBuffer::add( sDataCopy );
}

//-----------------------------------------------------------------------------

TGFStringWriter::TGFStringWriter() : TGFBaseWriter() {
}

TGFStringWriter::~TGFStringWriter() {
}

bool TGFStringWriter::processObject( TGFFreeable *obj ) {
   TGFString *str = static_cast<TGFString *>(obj);

   Output.addChunk( str );

   return true;
}

//-----------------------------------------------------------------------------

TGFFileWriter::TGFFileWriter() : TGFBaseWriter() {
   bAppendmode = false;
}

TGFFileWriter::~TGFFileWriter() {
}

void TGFFileWriter::open( const TGFString *sFile, bool bAppendmode ) {
   open( sFile->getValue(), bAppendmode );
}

void TGFFileWriter::open( const char *sFile, bool bAppendmode ) {
   this->bAppendmode = bAppendmode;
   file.filename.set( sFile );

   if ( bAppendmode ) {
      file.mode.set( GFFILEMODE_WRITE | GFFILEMODE_APPEND );
   } else {
      file.mode.set( GFFILEMODE_WRITE );
      file.connect();
   }
}

void TGFFileWriter::close() {
   if ( !bAppendmode ) {
      file.disconnect();
   }
}

bool TGFFileWriter::processObject( TGFFreeable *obj ) {
   TGFString *str = static_cast<TGFString *>(obj);

   if ( bAppendmode ) {
      file.connect();
      file.send( str );
      file.disconnect();
   } else {
      file.send( str );
   }

   delete str;

   return true;
}

