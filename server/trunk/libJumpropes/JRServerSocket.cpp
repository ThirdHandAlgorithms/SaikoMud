
#include "JRServerSocket.h"

#include <Groundfloor/Atoms/GFDefines.h>

#ifdef GF_OS_WIN32
   #include <winsock2.h>
   #include <ws2tcpip.h>
   #define socklen_t int
#else
   #include <netdb.h>
   #include <cstring>

	#ifdef GF_OS_MAC
		#include <sys/types.h>
	#endif
#endif


TJRServerSocket::TJRServerSocket() : TJRBaseSocket() {
   this->localPort.set( 0 );
   this->maxconnections.set( 2114125311 );
}

TJRServerSocket::~TJRServerSocket() {
}

void TJRServerSocket::setAddressType( int iAFType ) {
   if (( iAFType == AF_INET ) || ( iAFType == AF_INET6 )) {
      this->iAddressType = iAFType;
   }
}

bool TJRServerSocket::connect() {
   if ( !TJRBaseSocket::connect() ) {
      return false;
   }

   setBlocking( true );

	int iStatus = -1;

	if ( iAddressType == AF_INET ) {
      struct sockaddr_in aSocketAddressIPv4;
      memset( &aSocketAddressIPv4, 0, sizeof( struct sockaddr_in ) );

      aSocketAddressIPv4.sin_family       = AF_INET;
      aSocketAddressIPv4.sin_addr.s_addr  = INADDR_ANY;
      aSocketAddressIPv4.sin_port         = htons( this->localPort.get() );

      iStatus = ::bind( iSocketId, reinterpret_cast<sockaddr *>( &aSocketAddressIPv4 ), sizeof( struct sockaddr_in ) );
	} else if ( iAddressType == AF_INET6 ) {
      struct sockaddr_in6 aSocketAddressIPv6;
      struct in6_addr myIn6AddrAny = IN6ADDR_ANY_INIT;
      memset( &aSocketAddressIPv6, 0, sizeof( struct sockaddr_in6 ) );

      aSocketAddressIPv6.sin6_family       = AF_INET6;
      aSocketAddressIPv6.sin6_addr         = myIn6AddrAny;
      aSocketAddressIPv6.sin6_port         = htons( this->localPort.get() );

      iStatus = ::bind( iSocketId, reinterpret_cast<sockaddr *>( &aSocketAddressIPv6 ), sizeof( struct sockaddr_in6 ) );
	}

   if ( iStatus != -1 ) {
      iStatus = ::listen( iSocketId, this->maxconnections.get() );
   }

   setBlocking( false );

   return ( iStatus != -1 );
}

TJRBaseSocket *TJRServerSocket::waitForNewClient() {
	sockaddr_storage anAddress;
	socklen_t iAddrLen = sizeof(anAddress);
	int iAnOtherSocketId = -1;

	TJRBaseSocket *anOtherSocket = NULL;

	iAnOtherSocketId = accept( iSocketId, reinterpret_cast<sockaddr *>( &anAddress ), &iAddrLen );

	if ( iAnOtherSocketId > 0 ) {
		anOtherSocket = new TJRBaseSocket( iAnOtherSocketId, this );
	}

	return anOtherSocket;
}

bool TJRServerSocket::waitForNewClientAndReinitialize( TJRBaseSocket *anExistingSocket ) {
	sockaddr_storage anAddress;
	socklen_t iAddrLen = sizeof(anAddress);
	int iAnOtherSocketId = -1;

	iAnOtherSocketId = accept( iSocketId, reinterpret_cast<sockaddr *>( &anAddress ), &iAddrLen );

	if ( iAnOtherSocketId <= 0 ) {
		iAnOtherSocketId = -1;
	}

	anExistingSocket->reinitialize( iAnOtherSocketId );

	return (iAnOtherSocketId != -1);
}

