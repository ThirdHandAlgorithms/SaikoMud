#ifndef _JRSERVERSOCKET_H_
#define _JRSERVERSOCKET_H_

#include "JRBaseSocket.h"

class TJRServerSocket : public TJRBaseSocket
{
	protected:
	public:
		TJRServerSocket();
		~TJRServerSocket();

		/// Sets the socket's IP protocol. AF_INET is default, AF_INET6 is also possible
		void setAddressType( int iAFType );

      TGFProperty<int> maxconnections;

		bool connect();

		TJRBaseSocket *waitForNewClient();
		bool waitForNewClientAndReinitialize( TJRBaseSocket *anExistingSocket );
};

#endif /*_JRSERVERSOCKET_H_*/
