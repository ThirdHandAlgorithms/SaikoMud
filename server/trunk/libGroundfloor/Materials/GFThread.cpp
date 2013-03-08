
#include "GFThread.h"
#include "../Atoms/GFDefines.h"

#ifdef GF_OS_LINUX
    #include <pthread.h>
    #include <unistd.h>

    #define THREADRETURN void *
#else
    #include <windows.h>

    #define THREADRETURN DWORD
#endif



TGFMultiNotify<TGFThread *> GThreadStartNotify;
TGFMultiNotify<TGFThread *> GThreadEndNotify;

void addThreadStartNotify(TGFNotifyAbstract<TGFThread *> *pFunc) {
   GThreadStartNotify.addNotify(pFunc);
}

void addThreadEndNotify(TGFNotifyAbstract<TGFThread *> *pFunc) {
   GThreadEndNotify.addNotify(pFunc);
}


// non-member function to be spawned as a thread by the OS, executes our own thread-loop
THREADRETURN runExecute( void *pParam ) {
   static_cast<TGFThread *>(pParam)->executionloop();

   return 0;
}

// TGFThread functions
TGFThread::TGFThread() {
   bStartedExecutionMethod = false;
   bRunning             = false;
   bShouldTerminate     = false;

   bThreadIsCreated     = false;
   bExceptionOccurred   = false;

   bSleepAhead          = false;

   iDefaultSleepValue   = 10;

   setInterval( 1 );
}

TGFThread::~TGFThread() {
   stopAndWait();

   cleanup();
}

void TGFThread::cleanup() {
   if ( bThreadIsCreated ) {
   #ifdef GF_OS_WIN32
      CloseHandle( pHandle );
   #else
      pthread_detach( aPThreadStruct );
   #endif
   }

   bThreadIsCreated = false;
}


void TGFThread::start() {
   // do nothing if we're already running
   if ( !bRunning ) {

      bStartedExecutionMethod = false;
      bShouldTerminate     = false;
      bExceptionOccurred   = false;

      // free any previous thread handles
      cleanup();

      #ifdef GF_OS_WIN32
         // Windows
         // WINBASEAPI HANDLE WINAPI CreateThread(LPSECURITY_ATTRIBUTES,DWORD,LPTHREAD_START_ROUTINE,PVOID,DWORD,PDWORD);
         pHandle = CreateThread( NULL, 0, (THREADRETURN (__stdcall *)(void *))(runExecute), static_cast<void *>(this), 0, NULL );
      #else
         // Linux PThread
         // extern int pthread_create (pthread_t *__restrict __newthread,__const pthread_attr_t *__restrict __attr,void *(*__start_routine) (void *),void *__restrict __arg) __THROW __nonnull ((1, 3));
         pHandle = reinterpret_cast<void *>( pthread_create( &aPThreadStruct, NULL, runExecute, static_cast<void *>(this) ) );
      #endif

      bThreadIsCreated = true;

      // wait till the thread-loop is really running
      while ( !bStartedExecutionMethod ) {
         GFMillisleep( iDefaultSleepValue );
      }

   }
}

void TGFThread::stop() {
   // flag thread for termination
   bShouldTerminate = true;
}

void TGFThread::stopAndWait() {
   stop();

   // wait till the thread-loop is at a full halt
   while ( bRunning ) {
      GFMillisleep( iCurrentInterval );
   }
}

bool TGFThread::isRunning() {
   return bRunning;
}

void TGFThread::executionloop() {
   bRunning = true;
   bStartedExecutionMethod = true;

   GThreadStartNotify.execute(this);
   try {

      if (bSleepAhead) {
         GFMillisleep( iCurrentInterval );
      }

      while ( !bShouldTerminate ) {
         execute();

         if ( !bShouldTerminate ) {
            GFMillisleep( iCurrentInterval );
         }
      }

   } catch ( ... ) {
      // the try-catch makes sure we catch any errors
      //  and just stop the thread
      //  instead of raising it to the OS handling of this thread
      bExceptionOccurred = true;
   }

   bRunning = false;

   GThreadEndNotify.execute(this);
}

void TGFThread::setInterval( unsigned int iInterval, bool bSleepAhead ) {
   this->iCurrentInterval = iInterval;
   this->bSleepAhead = bSleepAhead;
}

void TGFThread::waitWhileRunning() {
    while ( bRunning ) {
        GFMillisleep( iDefaultSleepValue );
    }
}

void TGFThread::join( TGFThread *anOtherThread ) {
    while ( bRunning || anOtherThread->bRunning ) {
        GFMillisleep( iDefaultSleepValue );
    }
}
