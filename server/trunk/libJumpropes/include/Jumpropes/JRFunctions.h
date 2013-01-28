
#ifndef __JRFunctions_H
#define __JRFunctions_H

#include <Groundfloor/Atoms/GFDefines.h>
#include <Groundfloor/Molecules/GFString.h>
#include <Groundfloor/Molecules/GFVector.h>

#include "JRDefines.h"
#include "JRNetAddress.h"

#ifdef GF_OS_WIN32
   #include <winsock2.h>
   #include <ws2tcpip.h>
   #include "Wspiapi.h"
#else
   #include <netdb.h>
   #include <unistd.h>
   #include <fcntl.h>
   #include <arpa/inet.h>
   #include <sys/types.h>

   #include <sys/select.h>

   #include <cstring>
#endif


/// Tries to resolve the given Hostname into an IP address
/** If the given Hostname could be resolved True is returned and optionally
  *  sDestIP will be set to the ascii representation of the IP address.
  * sDestIP may be set to NULL.
  * If bSpecialCheck is set True, the result of the lookup will be compared
  *  to the result of the hostname loookup "nonvalid.domain.name.nonvalid".
  * This is added to make OS differences smaller and to even return false
  *  when a DNS provider like OpenDNS is used.
  **/
bool JRresolveIP( const TGFString *sSrcHostname, TJRLookupObject *aDestIP, bool bSpecialCheck = false );

bool JRresolveAll( TJRLookupObject *aDestIpArray );

//bool JRisIPv4address( const TGFString *sIp );
//bool JRisIPv6address( const TGFString *sIp );


#ifdef GF_OS_WIN32
   #ifdef GF_CP_MINGW
      int inet_pton( int af, const char *src, void *dst );
      const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt);
   #endif
#endif

#endif // __JRFunctions_H
