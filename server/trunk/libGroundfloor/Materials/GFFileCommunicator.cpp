
#include "GFFileCommunicator.h"

TGFFileCommunicator::TGFFileCommunicator() : TGFBaseCommunicator() {
   fp = NULL;
   mode.set( GFFILEMODE_READ );
}

TGFFileCommunicator::~TGFFileCommunicator() {
   disconnect();
}

bool TGFFileCommunicator::connect() {
   disconnect();

   int iMode = mode.get();

   bool bRead     = ((iMode & GFFILEMODE_READ) != 0);
   bool bWrite    = ((iMode & GFFILEMODE_WRITE) != 0);
   bool bAppend   = ((iMode & GFFILEMODE_APPEND) != 0);

   if ( filename.getLength() != 0 ) {
      #ifdef GF_OS_WIN32
      TGFString *sTmpFile = new TGFString( filename.link() );
      sTmpFile->transformToWidestring( true );

      if (bAppend) {
         if ( bRead ) {
            fp = _wfopen( sTmpFile->getValueAsWide(), L"r+b" );
         } else {
            fp = _wfopen( sTmpFile->getValueAsWide(), L"ab" );
         }
      } else {
         if ( bWrite && bRead ) {
            fp = _wfopen( sTmpFile->getValueAsWide(), L"w+b" );
         } else if ( bWrite ) {
            fp = _wfopen( sTmpFile->getValueAsWide(), L"wb" );
         } else {
            fp = _wfopen( sTmpFile->getValueAsWide(), L"rb" );
         }
      }

      delete sTmpFile;
      #else
      if (bAppend) {
         if ( bRead ) {
            fp = fopen( filename.get(), "r+b" );
         } else {
            fp = fopen( filename.get(), "ab" );
         }
      } else {
         if ( bWrite && bRead ) {
            fp = fopen( filename.get(), "w+b" );
         } else if ( bWrite ) {
            fp = fopen( filename.get(), "wb" );
         } else {
            fp = fopen( filename.get(), "rb" );
         }
      }
      #endif
   }

   bConnected = (fp != NULL);

   return bConnected;
}

bool TGFFileCommunicator::disconnect() {
   if ( bConnected ) {
      fclose( fp );

      fp          = NULL;
      bConnected  = false;
   }

   return true;
}

bool TGFFileCommunicator::send( const TGFString *sData, TGFCommReturnData *errData ) {
   if ( bConnected ) {
      lockReadWrite.lockWhenAvailable( GFLOCK_INFINITEWAIT );

      size_t iLastWritten = fwrite( sData->getPointer(0), 1, sData->getLength(), fp );

      if ( errData != NULL ) {
         errData->affected    = iLastWritten;
         errData->eof         = (feof( fp ) != 0);
         errData->errorcode   = ferror( fp );
         errData->error       = (errData->errorcode != 0);
      }

      lockReadWrite.unlock();
      return true;
   }

   return false;
}

bool TGFFileCommunicator::receive( TGFString *sData, TGFCommReturnData *errData ) {
   if ( bConnected ) {
      lockReadWrite.lockWhenAvailable( GFLOCK_INFINITEWAIT );

      size_t iLastRead = fread( sData->getPointer(0), 1, sData->getSize(), fp );
      sData->setLength( iLastRead );

      if ( errData != NULL ) {
         errData->affected    = iLastRead;
         errData->eof         = (feof( fp ) != 0);
         errData->errorcode   = ferror( fp );
         errData->error       = (errData->errorcode != 0);
      }

      lockReadWrite.unlock();
      return true;
   }

   return false;
}
