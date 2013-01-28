
#ifndef __GFLOGGER_H
#define __GFLOGGER_H

#include "GFThread.h"
#include "../Molecules/GFString.h"
#include "../Molecules/GFVector.h"
#include "../Atoms/GFLockable.h"


#define NO_CRLF 0
#define UNIX_FILE 1
#define DOS_FILE 2
#define MAC_FILE 3

#define NO_OUTPUT 0
#define CONSOLE_OUTPUT 1
#define FILE_OUTPUT 2
#define FULL_OUTPUT 3

/// class is not used
class TGFLogger: public TGFThread {
   protected:
      TGFVector *lstMessages;

      TGFString *sOutputFile;

      TGFString *sStartPreMsg;
      TGFString *sEndPreMsg;
      TGFString *sStartPostMsg;
      TGFString *sEndPostMsg;

      TGFLockable *lBufferLock;

      int iLogType;
      int iFileType;
      int iNestlevel;

      void resetVars();

      void logInFile( const TGFString *sMsg );
      void logInStdOut( const TGFString *sMsg );

   public:
      void begin( const TGFString *sMsg );
      void begin( const char *sMsg );
      void end( const TGFString *sMsg );
      void end( const char *sMsg );


      TGFLogger( int logType, char *logFile, int logFileType, bool verbose );
      ~TGFLogger();

      void log( const TGFString *sMsg );
      void log( const char *sMsg );
};

#endif // __GFLOGGER_H
