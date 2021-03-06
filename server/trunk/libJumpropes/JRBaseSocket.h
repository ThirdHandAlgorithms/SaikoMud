#ifndef _JRBASESOCKET_H_
#define _JRBASESOCKET_H_

#include <Groundfloor/Atoms/GFDefines.h>
#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Molecules/GFString.h>
#include <Groundfloor/Molecules/GFProperty.h>
#include <Groundfloor/Molecules/GFBaseCommunicator.h>

#include "JRDefines.h"
#include "JRNetAddress.h"


/** TJRBaseSocket is a base class for both client- and server-oriented native sockets.
 *  Most of this code is based upon POSIX functions.
 */
class TJRBaseSocket : public TGFBaseCommunicator
{
	protected:
		int 		iSocketId;

		TJRIPAddress aRemoteAddress;
		TJRIPAddress aLocalAddress;

		bool loadLocalAddressInfo();
		bool loadRemoteAddressInfo();
		
		int       iConnectionType;
		int       iAddressType;
		
		void resetVars();
		
      /// Set the socket in blocking or non-blocking mode.
      /** It is prefered the socket stays in its Default Non-Blocking state,
        *  for connection purposes you may temporarily set this to blocking
        *  while the connection is being established.
        */
		void setBlocking( bool bBlocking );
		
	public:
      /// Creates a new native socket.
		TJRBaseSocket();
      /// Initializes the socket with the given native socket identifier.
		TJRBaseSocket( int iSocketId, const TJRBaseSocket *aServer = NULL );
		~TJRBaseSocket();
		
		/// Default = true, splits data to send in chunks of [autosplit_size] length and chain-sends these chunks.
		/** Please just use JRThreadedSender.
		  **/
		TGFProperty<bool> autosplit_sendbuffer;
		
		/// Default = 1024 bytes
		TGFProperty<unsigned int> autosplit_size;
		
      /// Reinitializes the current socket with a given native socket identifier.
		void reinitialize( int iSocketId );
		
      /// Returns the ip-address that is used on this machine for this socket.
		TJRIPAddress *getLocalAddress() { return &aLocalAddress; }
		
      /// Returns the ip-address of the remote machine that is connected to this socket.
		TJRIPAddress *getRemoteAddress() { return &aRemoteAddress; }

      TGFProperty<int>   remotePort;
      TGFProperty<int>   localPort;

      /// Returns when socket is ready to be written whether or not socket is blocking.
      /** This function is - internally - used to determine when to continue send_data(),
        *  therefor send_data() is always "blocking".
        * Don't override (or use for that matter) this function
        *  unless you know what you're doing.
        **/
      bool isReadyToSend();
      
      /// open socket
      bool connect();
      /// close socket
      bool disconnect();
      
      /// Send data to the remote machine via the current socket.
      /** Please mind comments on the autosplit variables */
		bool send( const TGFString *sData, TGFCommReturnData *errData = NULL );
      /// Return whatever data that was sent to this socket and that is currently stored in the socket buffer.
		bool receive( TGFString *sData, TGFCommReturnData *errData = NULL );
};


#endif /*_JRBASESOCKET_H_*/
