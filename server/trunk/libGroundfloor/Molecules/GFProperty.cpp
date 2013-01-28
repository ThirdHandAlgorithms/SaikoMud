

#include "GFProperty.h"
#include "../Molecules/GFCallbacks.h"
#include "../Materials/GFFunctions.h"

TGFBasicStringProperty::TGFBasicStringProperty() : TGFFreeable() {
}

TGFBasicStringProperty::~TGFBasicStringProperty() {
}

TGFString *TGFBasicStringProperty::link( bool bDoRetreive ) {
   return &anObject;
}

void TGFBasicStringProperty::set( const char *object ) {
   internalSet( object );
}

char *TGFBasicStringProperty::get() const {
   return internalGet();
}

void TGFBasicStringProperty::internalSet( const char *object ) {
   this->anObject.setValue_ansi( object );
}
void TGFBasicStringProperty::internalSetCopy( const TGFString *object ) {
   this->anObject.setValue( object );
}

char *TGFBasicStringProperty::internalGet() const {
   return this->anObject.getValue();
}

unsigned int TGFBasicStringProperty::getLength() {
   return this->anObject.getLength();
}
//----------------------------------------------------------------------------


TGFStringProperty::TGFStringProperty() {
   this->aNotify = 0;
   this->aRetreive = 0;
}

TGFStringProperty::~TGFStringProperty() {
   if ( ( this->aNotify != 0 ) && (this->bAutoFreeNotify) ) {
      delete this->aNotify;
   }

   if ( ( this->aRetreive != 0 ) && (this->bAutoFreeRetreive) ) {
      delete this->aRetreive;
   }
}

void TGFStringProperty::setNotify( TGFNotifyAbstract<const char *> *anEventHandler, bool bAutoFree ) {
   if ( ( this->aNotify != 0 ) && (this->bAutoFreeNotify) ) {
      delete this->aNotify;
   }

   this->aNotify = anEventHandler;
   this->bAutoFreeNotify = bAutoFree;
}

void TGFStringProperty::setRetreive( TGFRetreiveAbstract<char *> *anEventHandler, bool bAutoFree ) {
   if ( ( this->aRetreive != 0 ) && (this->bAutoFreeRetreive) ) {
      delete this->aRetreive;
   }

   this->aRetreive = anEventHandler;
   this->bAutoFreeRetreive = bAutoFree;
}

TGFString *TGFStringProperty::link( bool bDoRetreive ) {
   if ( bDoRetreive && (this->aRetreive != 0 )) {
      this->aRetreive->retreive();
   }

   return &anObject;
}

void TGFStringProperty::set( const char * object ) {
   if ( this->aNotify != 0 ) {
      this->aNotify->execute( object );
   } else {
      this->internalSet( object );
   }
}

char *TGFStringProperty::get() const {
   if ( this->aRetreive != 0 ) {
      return this->aRetreive->retreive();
   } else {
      return this->internalGet();
   }
}

//------------------------------------------------------------------------------


TGFBasicWideStringProperty::TGFBasicWideStringProperty() : TGFFreeable() {
}

TGFBasicWideStringProperty::~TGFBasicWideStringProperty() {
}

void TGFBasicWideStringProperty::set( const wchar_t *object ) {
   internalSet( object );
}

wchar_t *TGFBasicWideStringProperty::get() const {
   return internalGet();
}

TGFString *TGFBasicWideStringProperty::link() {
   return &anObject;
}

void TGFBasicWideStringProperty::internalSet( const wchar_t *object ) {
   this->anObject.setValue_wide( object );
}
wchar_t *TGFBasicWideStringProperty::internalGet() const {
   return reinterpret_cast<wchar_t *>( this->anObject.getValue() );
}

unsigned int TGFBasicWideStringProperty::getLength() {
   return this->anObject.getLength();
}
//----------------------------------------------------------------------------


TGFWideStringProperty::TGFWideStringProperty() {
   this->aNotify = 0;
   this->aRetreive = 0;
}

TGFWideStringProperty::~TGFWideStringProperty() {
   if ( ( this->aNotify != 0 ) && (this->bAutoFreeNotify) ) {
      delete this->aNotify;
   }

   if ( ( this->aRetreive != 0 ) && (this->bAutoFreeRetreive) ) {
      delete this->aRetreive;
   }
}

void TGFWideStringProperty::setNotify( TGFNotifyAbstract<const wchar_t *> *anEventHandler, bool bAutoFree ) {
   if ( ( this->aNotify != 0 ) && (this->bAutoFreeNotify) ) {
      delete this->aNotify;
   }

   this->aNotify = anEventHandler;
   this->bAutoFreeNotify = bAutoFree;
}

void TGFWideStringProperty::setRetreive( TGFRetreiveAbstract<wchar_t *> *anEventHandler, bool bAutoFree ) {
   if ( ( this->aRetreive != 0 ) && (this->bAutoFreeRetreive) ) {
      delete this->aRetreive;
   }

   this->aRetreive = anEventHandler;
   this->bAutoFreeRetreive = bAutoFree;
}

void TGFWideStringProperty::set( const wchar_t * object ) {
   if ( this->aNotify != 0 ) {
      this->aNotify->execute( object );
   } else {
      this->internalSet( object );
   }
}

wchar_t *TGFWideStringProperty::get() const {
   if ( this->aRetreive != 0 ) {
      return this->aRetreive->retreive();
   } else {
      return this->internalGet();
   }
}
