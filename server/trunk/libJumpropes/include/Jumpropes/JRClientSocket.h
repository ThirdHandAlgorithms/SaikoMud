#ifndef _JRCLIENTSOCKET_H_
#define _JRCLIENTSOCKET_H_

#include "JRBaseSocket.h"


class TJRClientSocket : public TJRBaseSocket
{
	public:
		TJRClientSocket();
		~TJRClientSocket();
		
		/// Open socket to ip:port
		/** Socket is temporarily set blocking while connection is being
        *  established. Before bConnected is set and the function has returned
        *  the socket will have been reset to the non-blocking default.
        **/
		bool connect();
};

#endif /*_JRCLIENTSOCKET_*/
