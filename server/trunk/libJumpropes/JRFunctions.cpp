
#include <Groundfloor/Atoms/GFDefines.h>

#include "JRFunctions.h"

#include <cstdio>
#include <Groundfloor/Bookshelfs/GFBValue.h>
#include <Groundfloor/Materials/GFFunctions.h>


bool JRisIPv4address( const TGFString *sIp ) {
   TGFVector *slices = GFsplit( sIp, "." );
   int c = 0;

   TGFBValue *val = new TGFBValue();
   if ( slices->size() == 4 ) {
      for ( int i = 0; i < 4; i++ ) {
         TGFString *s = static_cast<TGFString *>( slices->elementAt(i) );
         if ( GFisNumber( s->getValue(), s->getLength() ) ) {
            val->setString( s );

            int d = val->asInteger();
            if ( (d >= 0) && (d <= 255) ) {
               c++;
            }
         }
      }
   }
   delete val;

   delete slices;

   return (c == 4);
}


// http://nl.wikipedia.org/wiki/Internet_Protocol_Version_6

bool JRisIPv6address( const TGFString *sIp ) {
/*
   TGFString *sDot = new TGFString( ":" );
   TGFVector *slices = split( sIp, sDot );
   delete sDot;
   int c = 0;

   if ( slices->size() == 8 ) {
      for ( int i = 0; i < 8; i++ ) {
         TGFString *s = static_cast<TGFString *>( slices->elementAt(i) );
         if ( isNumber( s->getValue(), s->getLength() ) ) {
            HexToInt( s );
            val->setString( s );

            int d = val->asInteger();
            if ( (d >= 0) && (d <= 255) ) {
               c++;
            }
         }
      }
   }

   delete slices;

   return (c == 4);
*/

   return false;
}

bool JRresolveIP( const TGFString *sSrcHostname, TGFString *sDestIP, bool bSpecialCheck ) {
   struct hostent *remoteHost;
   bool bSucces = false;

   remoteHost = gethostbyname( sSrcHostname->getValue() );

   if ( remoteHost != NULL ) {
      struct in_addr addr;
	   addr.s_addr = *(unsigned long *) remoteHost->h_addr_list[0];

		if ( bSpecialCheck ) {
			struct hostent *nonvalidHost;

         char *pTmpAddrStr = inet_ntoa( addr );
         TGFString *sRemoteHost = new TGFString( pTmpAddrStr, strlen(pTmpAddrStr) );

         nonvalidHost = gethostbyname( C_NONVALIDHOSTNAME );
         if ( nonvalidHost != NULL ) {
            struct in_addr nonvalidAddr;
            nonvalidAddr.s_addr = *(unsigned long *) nonvalidHost->h_addr_list[0];

            pTmpAddrStr = inet_ntoa( nonvalidAddr );
            TGFString *sNonvalidHost = new TGFString( pTmpAddrStr, strlen(pTmpAddrStr) );

            bSucces = ( !sRemoteHost->match( sNonvalidHost ) );

            if ( bSucces && (sDestIP != NULL) ) {
               sDestIP->setValue( sRemoteHost );
            }

            delete sNonvalidHost;
         } else {
            bSucces = true;

            if (sDestIP != NULL) {
               sDestIP->setValue( sRemoteHost );
            }
         }

         delete sRemoteHost;

		} else {
			if ( sDestIP != NULL ) {
            char *sAddr = inet_ntoa( addr );
				sDestIP->setValue( sAddr, strlen(sAddr) );
			}

			return true;
      }
   }

   return bSucces;
}

bool JRresolveIP( const TGFString *sSrcHostname, TJRLookupObject *aDestIP, bool bSpecialCheck ) {
   TJRLookupObject *all = new TJRLookupObject();
   all->name.setValue( sSrcHostname );
   if ( JRresolveAll( all ) ) {
      if ( bSpecialCheck ) {
         TJRLookupObject *nonValidHost = new TJRLookupObject();
         nonValidHost->name.setValue_ansi( C_NONVALIDHOSTNAME );

         while ( all->firstAddressEqualsOneOf( nonValidHost, false ) ) {
            delete all->removeElement( (unsigned int)0 );
            all->compress();

            if ( all->getAddress() == NULL ) {
               break;
            }
         }

         delete nonValidHost;
      }
   }

   if ( all->getAddress() != NULL ) {
      aDestIP->setValue( all );
   }
   delete all;

   return (aDestIP->getAddress() != NULL);
/*
   struct hostent *remoteHost;
   bool bSucces = false;

   remoteHost = gethostbyname( sSrcHostname->getValue() );
   if ( remoteHost != NULL ) {
      struct in_addr addr;
	   addr.s_addr = *(unsigned long *) remoteHost->h_addr_list[0];

		if ( bSpecialCheck ) {
			struct hostent *nonvalidHost;

         char *pTmpAddrStr = inet_ntoa( addr );
         TGFString *sRemoteHost = new TGFString( pTmpAddrStr, strlen(pTmpAddrStr) );

         nonvalidHost = gethostbyname( C_NONVALIDHOSTNAME );
         if ( nonvalidHost != NULL ) {
            struct in_addr nonvalidAddr;
            nonvalidAddr.s_addr = *(unsigned long *) nonvalidHost->h_addr_list[0];

            pTmpAddrStr = inet_ntoa( nonvalidAddr );
            TGFString *sNonvalidHost = new TGFString( pTmpAddrStr, strlen(pTmpAddrStr) );

            bSucces = ( !sRemoteHost->match( sNonvalidHost ) );

            if ( bSucces && (aDestIP != NULL) ) {
               TJRIPAddress *ip = aDestIP->newAddress();
               ip->family = AF_INET;
               ip->ip.setValue( sRemoteHost );
            }

            delete sNonvalidHost;
         } else {
            bSucces = true;

            if (aDestIP != NULL) {
               TJRIPAddress *ip = aDestIP->newAddress();
               ip->family = AF_INET;
               ip->ip.setValue( sRemoteHost );
            }
         }

         delete sRemoteHost;

		} else {
			if ( aDestIP != NULL ) {
            TJRIPAddress *ip = aDestIP->newAddress();
            ip->family = AF_INET;
            char *sAddr = inet_ntoa( addr );
            ip->ip.setValue( sAddr, strlen(sAddr) );
			}

			return true;
      }
   }

   return bSucces;
*/
}


#ifdef GF_OS_WIN32
#ifdef GF_CP_MINGW
const char *inet_ntop( int af, const void *src, char *dst, socklen_t cnt ) {
        if ( af == AF_INET )  {
                struct sockaddr_in in;
                memset( &in, 0, sizeof(struct sockaddr_in) );
                in.sin_family = AF_INET;

                memcpy( &in.sin_addr, src, sizeof(struct in_addr));

                getnameinfo( (struct sockaddr *)&in, sizeof(struct sockaddr_in), dst, cnt, NULL, 0, NI_NUMERICHOST );

                return dst;
        } else if ( af == AF_INET6 ) {
                struct sockaddr_in6 in;
                memset( &in, 0, sizeof(struct sockaddr_in6) );
                in.sin6_family = AF_INET6;

                memcpy( &in.sin6_addr, src, sizeof(struct in_addr6) );

                getnameinfo( (struct sockaddr *)&in, sizeof(struct sockaddr_in6), dst, cnt, NULL, 0, NI_NUMERICHOST );

                return dst;
        }
        return NULL;
}

int inet_pton( int af, const char *src, void *dst ) {
   if ( af == AF_INET ) {
      struct addrinfo hints;
      struct addrinfo *aiList = NULL;

      memset( &hints, 0, sizeof(struct addrinfo) );
      hints.ai_family = AF_INET;
      hints.ai_flags = AI_NUMERICHOST;

      int iRes = getaddrinfo( src, NULL, &hints, &aiList );
      if ( iRes == 0 ) {
         struct addrinfo *pInfo = aiList;
         if ( pInfo != NULL ) {
            memcpy( dst, &reinterpret_cast<sockaddr_in *>(pInfo->ai_addr)->sin_addr, sizeof(in_addr) );
            freeaddrinfo( pInfo );
            return 1;
         }
      }

      return -1;
   } else if ( af == AF_INET6 ) {
      struct addrinfo hints;
      struct addrinfo *aiList = NULL;

      memset( &hints, 0, sizeof(struct addrinfo) );
      hints.ai_family = AF_INET6;
      hints.ai_flags = AI_NUMERICHOST;

      int iRes = getaddrinfo( src, NULL, &hints, &aiList );
      if ( iRes == 0 ) {
         struct addrinfo *pInfo = aiList;
         if ( pInfo != NULL ) {
            memcpy( dst, &reinterpret_cast<sockaddr_in6 *>(pInfo->ai_addr)->sin6_addr, sizeof(in6_addr) );
            freeaddrinfo( pInfo );
            return 1;
         }
      }

      return -1;
   }
   return 0;
}
#endif
#endif


bool JRresolveAllWithGetAddrInfo( TJRLookupObject *aDestIpArray ) {
   bool bSuccess = false;
   struct addrinfo aiHints;
   struct addrinfo *aiList = NULL;

   memset(&aiHints, 0, sizeof(aiHints));
   aiHints.ai_family = AF_UNSPEC;
   //aiHints.ai_socktype = SOCK_STREAM;
   //aiHints.ai_protocol = IPPROTO_TCP;

   int iRet = getaddrinfo( aDestIpArray->name.getValue(), NULL, &aiHints, &aiList );
   if ( iRet == 0 ) {
      int i = 0;
      struct addrinfo *pInfo = aiList;

      while ( pInfo != NULL ) {
         TJRIPAddress *address = new TJRIPAddress();
         address->family = pInfo->ai_family;

         if ( address->family == AF_INET ) {
            address->ip.setSize( INET_ADDRSTRLEN );
            address->setAsIPv4Address( &reinterpret_cast<struct sockaddr_in *>(pInfo->ai_addr)->sin_addr );
            inet_ntop( address->family, reinterpret_cast<char *>( address->getIPv4AddressPtr() ), address->ip.getPointer(0), INET_ADDRSTRLEN );

            address->ip.setLength( strlen( address->ip.getValue() ) );
            aDestIpArray->addElement( address );

            bSuccess = true;
         } else if ( address->family == AF_INET6 ) {
            address->ip.setSize( INET6_ADDRSTRLEN );
            address->setAsIPv6Address( &reinterpret_cast<struct sockaddr_in6 *>(pInfo->ai_addr)->sin6_addr );
            inet_ntop( address->family, reinterpret_cast<char *>( address->getIPv6AddressPtr() ), address->ip.getPointer(0), INET6_ADDRSTRLEN );

            address->ip.setLength( strlen( address->ip.getValue() ) );
            aDestIpArray->addElement( address );

            bSuccess = true;
         }

         i++;
         struct addrinfo *pPrev = pInfo;
         pInfo = pPrev->ai_next;
         freeaddrinfo( pPrev );
      }
   }

   return bSuccess;
}

bool JRresolveAll( TJRLookupObject *aDestIpArray ) {
   struct hostent *remoteHost;
   bool bSuccess = false;

   // gethostbyname() only retreives IPv4 addresses under Microsoft Windows and is deprecated as well
   remoteHost = gethostbyname( aDestIpArray->name.getValue() );
   if ( remoteHost != NULL ) {
      int i = 0;
      char *pAddr = remoteHost->h_addr_list[i];
      while ( pAddr != NULL ) {
         TJRIPAddress *address = new TJRIPAddress();
         address->family = remoteHost->h_addrtype;

         if ( address->family == AF_INET ) {
            address->ip.setSize( INET_ADDRSTRLEN );
            address->setAsIPv4Address( reinterpret_cast<struct in_addr *>( remoteHost->h_addr_list[i] ) );
            inet_ntop( address->family, pAddr, address->ip.getPointer(0), INET_ADDRSTRLEN );
            address->ip.setLength( strlen( address->ip.getValue() ) );

            aDestIpArray->addElement( address );
            bSuccess = true;
         } else if ( address->family == AF_INET6 ) {
            address->ip.setSize( INET6_ADDRSTRLEN );
            address->setAsIPv6Address( reinterpret_cast<struct in6_addr *>( remoteHost->h_addr_list[i] ) );
            inet_ntop( address->family, pAddr, address->ip.getPointer(0), INET6_ADDRSTRLEN );
            address->ip.setLength( strlen( address->ip.getValue() ) );

            aDestIpArray->addElement( address );
            bSuccess = true;
         }

         i++;
         pAddr = remoteHost->h_addr_list[i];
      }
   }

   if ( !bSuccess ) {
      return JRresolveAllWithGetAddrInfo( aDestIpArray );
   }

   return bSuccess;
}
