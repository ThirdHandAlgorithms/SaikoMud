
#include "GFLogger.h"

#include <cstdio>
#include <cstdlib>


TGFLogger::TGFLogger( int logType, char *logFile, int logFileType, bool verbose ) {
   lstMessages    = new TGFVector();

   sOutputFile    = new TGFString( "", 0 );

   sStartPreMsg   = new TGFString( "", 0 );
   sEndPreMsg     = new TGFString( "", 0 );
   sStartPostMsg  = new TGFString( "", 0 );
   sEndPostMsg    = new TGFString( "", 0 );

   resetVars();
}

void TGFLogger::resetVars() {
   sStartPreMsg->setValue_ansi( "START: " );
   sEndPreMsg->setValue( "", 0 );
   sStartPostMsg->setValue_ansi( "END: " );
   sEndPostMsg->setValue( "", 0 );

	iLogType       = CONSOLE_OUTPUT;
   iFileType      = NO_CRLF;
   iNestlevel     = 0;
}

TGFLogger::~TGFLogger() {
   delete sEndPostMsg;
   delete sStartPostMsg;
   delete sEndPreMsg;
   delete sStartPreMsg;
   delete sOutputFile;
   delete lstMessages;
}

void TGFLogger::logInFile( const TGFString *sMsg ) {
   FILE *fp = fopen( sOutputFile->getValue(), "ab" );

   fwrite( sMsg->getValue(), sMsg->getLength(), 1, fp );

   fclose( fp );
}

void TGFLogger::logInStdOut( const TGFString *sMsg ) {
   printf( "%s", sMsg->getValue() );
}


void TGFLogger::log( const TGFString *sMsg ) {
   TGFString *sCopy = new TGFString( sMsg );

//   sCopy->prepend(  );
   lstMessages->addElement( sCopy );
}

void TGFLogger::log( const char *sMsg ) {
   TGFString *sCopy = new TGFString( sMsg );

   lstMessages->addElement( sCopy );
}

