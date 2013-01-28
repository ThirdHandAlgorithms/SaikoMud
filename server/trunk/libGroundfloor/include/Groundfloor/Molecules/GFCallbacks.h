
#ifndef __GFCallbacks_H
#define __GFCallbacks_H

#include "../Atoms/GFCallable.h"
#include "GFVector.h"

#define GFCreateNotify(a,b,c,d)    new TGFNotifyFunctionPointer<a,b>( c, d )
#define GFCreateRetreive(a,b,c,d)  new TGFRetreiveFunctionPointer<a,b>( c, d )

// template implementaties moeten helaas in de header file, omdat iedere
//  keer bij een nieuw type bij [new TGFNotifyFunction<type>()] een nieuwe implementatie
//  moet worden aangemaakt en worden gecompileerd in de uiteindelijke binary.

template <class P>
class TGFNotifyAbstract: public TGFCallable
{
   protected:
      P aDefaultParam;
   public:
      TGFNotifyAbstract() : TGFCallable() {
         aDefaultParam = 0;
      }
      ~TGFNotifyAbstract() {
      }

      void setDefaultParam( P aParam ) {
         aDefaultParam = aParam;
      }

      virtual void execute( P aParam ) = 0;
};

template <class P>
class TGFRetreiveAbstract: public TGFFreeable
{
   public:
      TGFRetreiveAbstract() : TGFFreeable() {
      }
      ~TGFRetreiveAbstract() {
      }

      virtual P retreive() = 0;
};



template <class P>
class TGFMultiNotify: public TGFNotifyAbstract<P>
{
   protected:
      TGFVector *aNotifyList;

      P aDefaultParam;
   public:
      TGFMultiNotify() : TGFNotifyAbstract<P>::TGFNotifyAbstract() {
         aNotifyList    = new TGFVector();
         aNotifyList->autoClear = true;
         aDefaultParam  = 0;
      }

      TGFMultiNotify( TGFMultiNotify<P> *pNotify ) : TGFNotifyAbstract<P>::TGFNotifyAbstract() {
         aNotifyList    = new TGFVector();
         pNotify->aNotifyList->fastCopy( aNotifyList );
         aNotifyList->autoClear = false;
         aDefaultParam  = pNotify->aDefaultParam;
      }

      ~TGFMultiNotify() {
         delete aNotifyList;
      }

      void setDefaultParam( P aParam ) {
         aDefaultParam = aParam;
      }

      void execute() {
         execute( aDefaultParam );
      }

      void execute( P aParam ) {
         unsigned int c = aNotifyList->size();
         for ( unsigned int i = 0; i < c; i++ ) {
            TGFNotifyAbstract<P> *obj = static_cast<TGFNotifyAbstract<P> *>( aNotifyList->elementAt(i) );
            if ( obj != 0 ) {
               obj->execute( aParam );
            }
         }
      }

      void addNotify( TGFNotifyAbstract<P> *aNotify ) {
         aNotifyList->addElement( aNotify );
      }

      TGFNotifyAbstract<P> *removeNotify( TGFNotifyAbstract<P> *aNotify ) {
         return static_cast<TGFNotifyAbstract<P> *>( aNotifyList->removeElement( aNotify ) );
      }

      TGFNotifyAbstract<P> *getNotify( unsigned int i ) {
         return static_cast<TGFNotifyAbstract<P> *>( aNotifyList->elementAt( i ) );
      }

      unsigned int getNotifyCount() {
         return aNotifyList->size();
      }

      // operators
      TGFMultiNotify& operator+= ( TGFNotifyAbstract<P> *aNotify ) {
         aNotifyList->addElement( aNotify );
         return *this;
      }
};


template <class P, class T>
class TGFNotifyFunctionPointer: public TGFNotifyAbstract<P>
{
   protected:
      typedef void (T::*FuncCallbackOneParam)( P );

      T *pObject;
      FuncCallbackOneParam pFunction;

   public:
      TGFNotifyFunctionPointer( T *pObject, FuncCallbackOneParam pFunction ) : TGFNotifyAbstract<P>::TGFNotifyAbstract() {
         this->pObject        = pObject;
         this->pFunction      = pFunction;
      };

      ~TGFNotifyFunctionPointer() {
      };

      void execute() {
         (pObject->*pFunction)( this->aDefaultParam );
      };

      void execute( P aParam ) {
         (pObject->*pFunction)( aParam );
      };
};

template <class P, class T>
class TGFRetreiveFunctionPointer: public TGFRetreiveAbstract<P>
{
   protected:
      typedef P (T::*FuncCallbackNoParam)();

      T *pObject;
      FuncCallbackNoParam pFunction;

   public:
      TGFRetreiveFunctionPointer( T *pObject, FuncCallbackNoParam pFunction ) : TGFRetreiveAbstract<P>::TGFRetreiveAbstract() {
         this->pObject        = pObject;
         this->pFunction      = pFunction;
      };

      ~TGFRetreiveFunctionPointer() {
      };

      P retreive() {
         return (pObject->*pFunction)();
      };
};


#endif // __GFCallbackPointerClasses_H

