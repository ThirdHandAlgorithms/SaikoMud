
#ifndef __GFCALLBACKTHREAD_H
#define __GFCALLBACKTHREAD_H

#include "GFThread.h"

#include "../Atoms/GFCallable.h"
#include "../Molecules/GFCallbacks.h"



/** TGFCallbackThread is a runnable thread that will call
  * the execute function of a given TGFCallable object every given interval.
  */
class TGFCallbackThread: public TGFThread
{
   protected:
      TGFCallable *aCallableObject;

   public:
      TGFCallbackThread( TGFCallable *anObject, int iSleeptime );
      ~TGFCallbackThread();

      /// Implementation of execute() that calls the execute() member function of another object passed at creation.
      void execute();
};


/** TGFCallbackThread is a runnable thread that will call
  * the execute function of the onTimerEvent MultiNotify object every given interval.
  */
class TGFTimer: public TGFThread {
   protected:
   public:
      TGFTimer();
      ~TGFTimer();

      TGFMultiNotify<TGFFreeable *> onTimerEvent;

      /// Implementation of execute()
      void execute();
};


template <class P>
class TGFRunnableFunction: public TGFThread {
    protected:
        typedef void (*FuncCallbackOneParam)( P );

        FuncCallbackOneParam pFunction;
        P aParam;
    public:
		  TGFRunnableFunction( FuncCallbackOneParam pFunction ) : TGFThread() {
            this->pFunction = pFunction;

            start();
        }
        TGFRunnableFunction( FuncCallbackOneParam pFunction, P aParam ) {
            this->pFunction = pFunction;
            this->aParam = aParam;

            start();
        }
        ~TGFRunnableFunction() {
            stop();
        }

        P getParam() {
            return aParam;
        }

        void runWithParam( P aParam ) {
            this->aParam = aParam;

            start();
        }

        void execute() {
            (*pFunction)( aParam );

            stop();
        }
};

template <typename P, typename R>
class TGFRunnableFunctionWithReturn: public TGFThread {
    protected:
        typedef R (*FuncCallbackOneParam)( P );
		  typedef void (*FuncReturnCallbackOneParam)( R );

        FuncCallbackOneParam pFunction;
		  FuncReturnCallbackOneParam pReturnFunction;
        P aParam;
		  R aReturn;
    public:
		 TGFRunnableFunctionWithReturn( FuncCallbackOneParam pFunction, P aParam, FuncReturnCallbackOneParam pReturnFunction ) : TGFThread() {
            this->pFunction = pFunction;
				this->pReturnFunction = pReturnFunction;
            this->aParam = aParam;

            start();
        }
        ~TGFRunnableFunctionWithReturn() {
            stop();
        }

        P getParam() {
            return aParam;
        }

		  R getReturnValue() {
			  return aReturn;
		  }

        void runWithParam( P aParam ) {
            this->aParam = aParam;

            start();
        }

        void execute() {
            aReturn = (*pFunction)( aParam );
				if ( pReturnFunction != 0 ) {
					(*pReturnFunction)( aReturn );
				}

            stop();
        }
};



#endif // __GFCALLBACKTHREAD_H
