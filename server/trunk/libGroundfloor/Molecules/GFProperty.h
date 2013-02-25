
#ifndef __GFProperty_H
#define __GFProperty_H

#include "../Atoms/GFFreeable.h"
#include "../Molecules/GFCallbacks.h"
#include "../Molecules/GFString.h"

#include "../Atoms/GFLockable.h"

/** TGFBasicProperty, don't use this base class.
  *  Properties are object containers with functions to get/set the object (or object-pointer).
  *  The internalSet() and internalGet() functions are public to serve the implementor functions of the
  *   get() and set() functions that may be customized by non-inheriting classes.
  **/

template <class T>
class TGFBasicProperty: public TGFFreeable
{
   protected:
      T anObject;
   public:
      /// initializes internal object to 0
      TGFBasicProperty() : TGFFreeable() {
         this->anObject = 0;
      }
      ~TGFBasicProperty() {
      }

      /// the default SET function (invokes internalSet())
      virtual void set( T object ) {
         internalSet( object );
      }
      /// the default GET function (invokes internalGet())
      virtual T get() const {
         return internalGet();
      }

      /// sets the internal object to given object
      void internalSet( T object ) {
         this->anObject = object;
      }
      /// returns the internal object
      T internalGet() const {
         return this->anObject;
      }
};


/// TGFProperty is the default property class to use,
///  use it in the public section of your classes
///  defined by for example: TGFProperty<int> myInteger;
///  or TGFProperty<TMyObject *> myObject;
template <class T>
class TGFProperty: public TGFBasicProperty<T>
{
   protected:
      TGFNotifyAbstract<T> *aNotify;
      TGFRetreiveAbstract<T> *aRetreive;

      bool bAutoFreeNotify;
      bool bAutoFreeRetreive;
   public:
      /// initializes property, sets notify/retreive functions to NULL
      TGFProperty();
      /// finalizes property and deletes notify/retreive functions if AutoFree is True
      ~TGFProperty();

      /// sets the notifier function that is invoked when the SET function is called
      /// notifier must conform to format [void yourfunction( <T> data )]
      /** EXAMPLE: property.setNotify( GFCreateNotify( TMyObject *, TMyClass, this, &TMyClass::myFunction ) );
       */
      void setNotify( TGFNotifyAbstract<T> *anEventHandler, bool bAutoFree = true );
      /// sets the retreive function that is invoked when the GET function is called
      void setRetreive( TGFRetreiveAbstract<T> *anEventHandler, bool bAutoFree = true );

      /// implemenation SET function: calls Notify function if set, otherwise it invokes internalSet() directly
      void set( T object );
      /// implemenation GET function: calls Retreive function if set, otherwise it invokes internalGet() directly
      T get() const;

      void add(T object);
};

template <class T>
class TGFLockableProperty: public TGFProperty<T>
{
   protected:
      TGFLockable aLock;

   public:
      TGFLockableProperty();
      ~TGFLockableProperty();

      void set( T object );
      
      void lockedAdd(T anotherObject);
};

/// special class for string-properties, don't use this base class
class TGFBasicStringProperty: public TGFFreeable
{
   protected:
      TGFString anObject;
   public:
      /// initializes internal string to ""
      TGFBasicStringProperty();
      ~TGFBasicStringProperty();

      /// Returns a pointer to the TGFString that is used.
      /// Do not call this function unless you really need it due to optimisation for example.
      TGFString *link( bool bDoRetreive = true );

      /// invokes
      virtual void set( const char *object );
      virtual char *get() const;

      unsigned int getLength();

      void internalSetCopy( const TGFString *object );
      void internalSet( const char *object );
      char *internalGet() const;
};

/// String-property-class.
/// The reason why there is a seperate property-class for strings, is because changes to the actual string
///  cannot be tracked when a normal [TGFProperty<TGFString *> myProperty] is used.
/// By forcing the developer to use char * set- and get-values, we know exactly when the internal string is being changed.
class TGFStringProperty: public TGFBasicStringProperty
{
   protected:
      TGFNotifyAbstract<const char *> *aNotify;
      TGFRetreiveAbstract<char *> *aRetreive;

      bool bAutoFreeNotify;
      bool bAutoFreeRetreive;
   public:
      /// sets internal string to "" and sets notify/retreive functions to NULL
      TGFStringProperty();
      ~TGFStringProperty();

      /// sets the notify function, must conform to [void myFunction( const char *myText )]
      void setNotify( TGFNotifyAbstract<const char *> *anEventHandler, bool bAutoFree = true );
      /// sets the retreive function, must conform to [char *myFunction()]
      void setRetreive( TGFRetreiveAbstract<char *> *anEventHandler, bool bAutoFree = true );

      /// invokes notify function if available, otherwise internalSet() is invoked directly
      void set( const char * object );
      /// invokes retreive function if available, otherwise internalGet() is invoked directly
      char *get() const;

      TGFString *link( bool bDoRetreive = true );
};

/// base class for widechar strings, do not use this base class
class TGFBasicWideStringProperty: public TGFFreeable
{
   protected:
      TGFString anObject;
   public:
      TGFBasicWideStringProperty();
      ~TGFBasicWideStringProperty();

      TGFString *link();

      virtual void set( const wchar_t *object );
      virtual wchar_t *get() const;

      unsigned int getLength();

      void internalSet( const wchar_t *object );
      wchar_t *internalGet() const;
};

/// Special property class for Widestrings, same as TGFStringProperty, except it forces the use of wchar_t * values.
class TGFWideStringProperty: public TGFBasicWideStringProperty
{
   protected:
      TGFNotifyAbstract<const wchar_t *> *aNotify;
      TGFRetreiveAbstract<wchar_t *> *aRetreive;

      bool bAutoFreeNotify;
      bool bAutoFreeRetreive;
   public:
      TGFWideStringProperty();
      ~TGFWideStringProperty();

      void setNotify( TGFNotifyAbstract<const wchar_t *> *anEventHandler, bool bAutoFree = true );
      void setRetreive( TGFRetreiveAbstract<wchar_t *> *anEventHandler, bool bAutoFree = true );

      void set( const wchar_t * object );
      wchar_t *get() const;
};


//----------------------------------------------------------------------------



template <class T>TGFProperty<T>::TGFProperty() {
   this->aNotify = 0;
   this->aRetreive = 0;
}

template <class T>TGFProperty<T>::~TGFProperty() {
   if ( ( this->aNotify != 0 ) && (this->bAutoFreeNotify) ) {
      delete this->aNotify;
   }

   if ( ( this->aRetreive != 0 ) && (this->bAutoFreeRetreive) ) {
      delete this->aRetreive;
   }
}

template <class T>void TGFProperty<T>::setNotify( TGFNotifyAbstract<T> *anEventHandler, bool bAutoFree ) {
   if ( ( this->aNotify != 0 ) && (this->bAutoFreeNotify) ) {
      delete this->aNotify;
   }

   this->aNotify = anEventHandler;
   this->bAutoFreeNotify = bAutoFree;
}

template <class T>void TGFProperty<T>::setRetreive( TGFRetreiveAbstract<T> *anEventHandler, bool bAutoFree ) {
   if ( ( this->aRetreive != 0 ) && (this->bAutoFreeRetreive) ) {
      delete this->aRetreive;
   }

   this->aRetreive = anEventHandler;
   this->bAutoFreeRetreive = bAutoFree;
}

template <class T>void TGFProperty<T>::set( T object ) {
   if ( this->aNotify != 0 ) {
      this->aNotify->execute( object );
   } else {
      this->internalSet( object );
   }
}

template <class T>T TGFProperty<T>::get() const {
   if ( this->aRetreive != 0 ) {
      return this->aRetreive->retreive();
   } else {
      return this->internalGet();
   }
}

template <class T>void TGFProperty<T>::add(T object) {
   this->set(this->get() + object);
}

//----------------------------------------------------------------------------

template <class T>TGFLockableProperty<T>::TGFLockableProperty() : TGFProperty<T>() {
}

template <class T>TGFLockableProperty<T>::~TGFLockableProperty() {
}

template <class T>void TGFLockableProperty<T>::set( T object ) {
   aLock.lockWhenAvailable();
   try {
      if ( this->aNotify != 0 ) {
         this->aNotify->execute( object );
      } else {
         this->internalSet( object );
      }
   } catch(...) {
      aLock.unlock();
      throw;
   }
   aLock.unlock();
}

template <class T>void TGFLockableProperty<T>::lockedAdd(T anotherObject) {
   aLock.lockWhenAvailable();
   try {
      this->anObject += anotherObject;
   } catch(...) {
      aLock.unlock();
      throw;
   }
   aLock.unlock();
}

//----------------------------------------------------------------------------

#endif // __GFProperty_H

