
#include "JRClientSocket.h"

#include <Groundfloor/Atoms/GFDefines.h>

#include "JRFunctions.h"
#ifdef GF_OS_WIN32
        #include <winsock2.h>
        #include <ws2tcpip.h>
#else
        #include <arpa/inet.h>
        #include <netdb.h>

		  #ifdef GF_OS_LINUX
		  	  #include <cstring>
		  #endif
#endif


#include <errno.h>

#include <cstdio>

TJRClientSocket::TJRClientSocket() : TJRBaseSocket() {
}

TJRClientSocket::~TJRClientSocket() {
}

bool TJRClientSocket::connect() {
   TJRIPAddress *address = getRemoteAddress();
	this->iAddressType = address->family;

   if ( !TJRBaseSocket::connect() ) {
      return false;
   }

   setBlocking( true );

	int iStatus = -1;

	char *sIpAddr = address->ip.getValue();
   if ( strlen(sIpAddr) > 0 ) {
      if ( address->family == AF_INET ) {
         struct sockaddr_in aSocketAddressIPv4;

         memset( &aSocketAddressIPv4, 0, sizeof( struct sockaddr_in ) );
         aSocketAddressIPv4.sin_family    = address->family;
         aSocketAddressIPv4.sin_port      = htons( remotePort.get() );
         aSocketAddressIPv4.sin_addr      = *address->getIPv4AddressPtr();

         iStatus = ::connect( iSocketId, reinterpret_cast<sockaddr *>( &aSocketAddressIPv4 ), sizeof( struct sockaddr_in ) );
      } else if ( address->family == AF_INET6 ) {
         struct sockaddr_in6 aSocketAddressIPv6;

         memset( &aSocketAddressIPv6, 0, sizeof( struct sockaddr_in6 ) );
         aSocketAddressIPv6.sin6_family   = address->family;
         aSocketAddressIPv6.sin6_port     = htons( remotePort.get() );
         aSocketAddressIPv6.sin6_addr     = *address->getIPv6AddressPtr();

         iStatus = ::connect( iSocketId, reinterpret_cast<sockaddr *>( &aSocketAddressIPv6 ), sizeof( struct sockaddr_in6 ) );
      }
   }

   setBlocking( false );

	this->bConnected = ( iStatus == 0 );
	if ( bConnected ) {
		loadLocalAddressInfo();
		loadRemoteAddressInfo();
	}

	return this->bConnected;
}
