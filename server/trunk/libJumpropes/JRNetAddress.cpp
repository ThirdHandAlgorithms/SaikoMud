
#include "JRNetAddress.h"

#ifdef GF_OS_LINUX
   #include <sys/socket.h>
#endif

#ifdef GF_CP_MVC
   #define AF_UNSPEC 0
#endif

//------------------------------------------------------------------------------

TJRIPAddress *TJRLookupObject::getAddress( unsigned int i ) const {
   return static_cast<TJRIPAddress *>( elementAt(i) );
}

void TJRLookupObject::setValue( const TJRLookupObject *obj ) {
   this->name.setValue( &obj->name );

   this->clear();
   unsigned int c = obj->size();
   for ( unsigned int i = 0; i < c; i++ ) {
      TJRIPAddress *oldAddr = obj->getAddress( i );
      TJRIPAddress *newAddr = new TJRIPAddress();
      newAddr->setValue( oldAddr );
      this->addElement(newAddr);
   }
}


bool TJRLookupObject::firstAddressEqualsOneOf( const TJRLookupObject *obj, bool bBothNullIsWin ) const {
   TJRIPAddress *first = getAddress();
   if ( obj != NULL ) {
      if ( first != NULL ) {
         unsigned int c = obj->size();
         for ( unsigned int i = 0; i < c; i++ ) {
            if ( first->equals( obj->getAddress(i) ) ) {
               return true;
            }
         }
      } else {
         if ( ( obj->size() == 0 ) && bBothNullIsWin ) {
            return true;
         }
      }
   } else {
      return ( first == NULL );
   }

   return false;
}

//------------------------------------------------------------------------------

#include <cstring>

TJRIPAddress::TJRIPAddress() : TGFFreeable() {
   family = AF_UNSPEC;
   ip.setValue( "", 0 );
   memset( this->addressbytes, 0, 16 );
}

void TJRIPAddress::setValue( const TJRIPAddress *addr ) {
   this->family = addr->family;
   this->ip.setValue( &addr->ip );
   memcpy( this->addressbytes, addr->addressbytes, 16 );
}

bool TJRIPAddress::equals( const TJRIPAddress *addr ) {
   return
      (
         ( this->family == addr->family ) &&
         ( this->ip.match( &addr->ip ) )
      );
}

struct in_addr *TJRIPAddress::getIPv4AddressPtr() {
   return reinterpret_cast<struct in_addr *>( &addressbytes );
}

struct in6_addr *TJRIPAddress::getIPv6AddressPtr() {
   return reinterpret_cast<struct in6_addr *>( &addressbytes );
}

void TJRIPAddress::setAsIPv4Address( struct in_addr *address ) {
   memset( this->addressbytes, 0, 16 );
   memcpy( this->addressbytes, address, 4 );
}

void TJRIPAddress::setAsIPv6Address( struct in6_addr *address ) {
   memcpy( this->addressbytes, address, 16 );
}
