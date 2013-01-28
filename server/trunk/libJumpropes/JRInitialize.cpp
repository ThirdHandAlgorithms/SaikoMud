
#include "JRInitialize.h"

#include <Groundfloor/Atoms/GFDefines.h>

#ifdef GF_OS_WIN32
    #include <winsock2.h>
#endif


bool initJumpropes() {
    
    #ifdef GF_OS_WIN32
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;
        
        wVersionRequested = MAKEWORD( 1, 0 );
        
        err = WSAStartup( wVersionRequested, &wsaData );
        if ( err != 0 ) {
            return false;
        }
        
    /* version check?
        if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) {
            WSACleanup( );
            return 1; 
        }
    */
    #endif

    return true;
}

void finiJumpropes() {
    
    #ifdef GF_OS_WIN32
        WSACleanup();
    #endif

}
