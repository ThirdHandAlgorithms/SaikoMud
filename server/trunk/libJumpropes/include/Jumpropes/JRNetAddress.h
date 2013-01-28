
#ifndef __JRNetAddress_H
#define __JRNetAddress_H

class TJRIPAddress;
class TJRLookupObject;


#include <Groundfloor/Atoms/GFDefines.h>
#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Molecules/GFString.h>
#include <Groundfloor/Molecules/GFVector.h>
#include <Groundfloor/Molecules/GFProperty.h>

#ifdef GF_OS_BSD
   #include <netinet/in.h>
#endif


class TJRLookupObject: public TGFVector {
   protected:
   public:
      TGFString name;

      void setValue( const TJRLookupObject *obj );
      bool firstAddressEqualsOneOf( const TJRLookupObject *obj, bool bBothNullIsWin = true ) const;

      TJRIPAddress *getAddress( unsigned int i = 0 ) const;
};

class TJRIPAddress: public TGFFreeable {
   protected:
      BYTE addressbytes[16];
   public:
      TJRIPAddress();

      void setValue( const TJRIPAddress *addr );
      bool equals( const TJRIPAddress *addr );

      struct in_addr *getIPv4AddressPtr();
      struct in6_addr *getIPv6AddressPtr();

      void setAsIPv4Address( struct in_addr *address );
      void setAsIPv6Address( struct in6_addr *address );

      unsigned short family;
      TGFString ip;
};

#endif // __JRNetAddress_H
