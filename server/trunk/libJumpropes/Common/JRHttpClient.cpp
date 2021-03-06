
#include "JRHttpClient.h"
#include <Groundfloor/Bookshelfs/GFBValue.h>

#include <Groundfloor/Materials/GFFunctions.h>

#include "../JRFunctions.h"


#ifdef GF_OS_LINUX
   #include <cstring>
   #include <cstdlib>
#endif

const char crlf[] = {13,10,0};
const char crlfcrlf[] = {13,10,13,10,0};
const char space[] = " ";
const char contentLenNeedle[] = "Content-Length: ";
const char contentTypeNeedle[] = "Content-Type: ";
const char locationNeedle[] = "Location: ";
const char connectionNeedle[] = "Connection: ";
const char httpNeedle[] = "HTTP/";

const char setcookieNeedle[] = "Set-Cookie: ";

const char sendgetNeedle[] = "GET ";
const char sendpostNeedle[] = "POST ";
const char sendhttpNeedle[] = " HTTP/";
const char hostNeedle[] = "Host: ";
const char useragentNeedle[] = "User-Agent: ";
const char refererNeedle[] = "Referrer: ";
const char cookieNeedle[] = "Cookie: ";

const char acceptrangeNeedle[] = "Accept-Ranges: ";
const char contentdispNeedle[] = "Content-Disposition: ";
const char transferencNeedle[] = "Transfer-Encoding: ";
const char rangeNeedle[] = "Range: ";

const char chunkedNeedle[] = "chunked";


void DEBUG_Range( const char *sName, TGFStringVectorRange *range ) {
   printf( "range (%s): [%d, %d] - [%d, %d]\n", sName, range->start_ind, range->start_pos, range->end_ind, range->end_pos );
}

void DEBUG_Str( const char *sName, const char *sStr ) {
   printf( "str (%s): [%s]\n", sName, sStr );
}


//----------------------------------------------------------------------------

/*

GET /SkypeSetup.exe HTTP/1.1
Host: download.skype.com
User-Agent: Mozilla/5.0 (Windows; U; Windows NT 6.0; nl; rv:1.8.1.11) Gecko/20071127 Firefox/2.0.0.11
Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*//*;q=0.5
Accept-Language: nl,en-us;q=0.7,en;q=0.3
Accept-Encoding: gzip,deflate
Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7
Keep-Alive: 300
Connection: keep-alive
Referer: http://tweakers.net/meuktracker/17066/skype-for-windows-360248.html
Cookie: SC=CC=:CCY=:LC=:TM=1201808799:TS=1201808799:TZ=:VER=0/3.6.0.244/

*/

TJRHttpHeader::TJRHttpHeader() : TGFFreeable() {
   cookies = new TGFVector();

   resetVars();
}

TJRHttpHeader::~TJRHttpHeader() {
   delete cookies;
}

void TJRHttpHeader::resetVars() {
   contentlength  = -1;
   httpversion    = 0.0;
   httpstatus     = -1;
   chunked        = false;
   cookies->clear();
   statusstring.setValue( "", 0 );
   location.setValue( "", 0 );
   contenttype.setValue( "", 0 );
   connection.setValue( "", 0 );
}

// TODO: rewrite met split()
void TJRHttpHeader::parse( TGFString *sHeader ) {
   int iPosStart;
   int iPosEnd;
   int iValStart;
   int iValLen;
   int iSpacePos;
   TGFString tmp;

   TGFString dbg;

   resetVars();

   int iCrlfLen = strlen(crlf);

   // "HTTP/1.0 301 Moved Permanently\r\n"
   iPosStart = sHeader->pos( 0, httpNeedle, strlen(httpNeedle) );
   if ( iPosStart != -1 ) {
      iPosEnd = sHeader->pos( iPosStart, crlf, iCrlfLen );
      if ( iPosEnd == -1 ) {
         iPosEnd = sHeader->getLength();
      }

      iValStart   = iPosStart + strlen( httpNeedle );
      iValLen     = iPosEnd - iValStart;

      tmp.setValue( sHeader->getPointer( iValStart ), iValLen );

      // tmp="1.0 301 Moved Permanently"
      char *ptr = tmp.getValue();

      iSpacePos = tmp.pos( 0, space, strlen(space) );
      ptr[iSpacePos] = 0;
      httpversion = atof( ptr );

      int iStatPos = iSpacePos + 1;
      iSpacePos = tmp.pos( 0, space, strlen(space) );
      ptr[iSpacePos] = 0;
      httpstatus = atoi( tmp.getPointer( iStatPos ) );

      statusstring.setValue( tmp.getPointer( iSpacePos + 1 ), tmp.getLength() - iSpacePos - 1 );
   }

   iPosStart = sHeader->pos( 0, contentLenNeedle, strlen(contentLenNeedle) );
   if ( iPosStart != -1 ) {
      iPosEnd = sHeader->pos( iPosStart, crlf, iCrlfLen );
      if ( iPosEnd == -1 ) {
         iPosEnd = sHeader->getLength();
      }

      iValStart   = iPosStart + strlen( contentLenNeedle );
      iValLen     = iPosEnd - iValStart;

      tmp.setValue( sHeader->getPointer( iValStart ), iValLen );

      contentlength = atoi( tmp.getValue() );
   }

   iPosStart = sHeader->pos( 0, contentTypeNeedle, strlen(contentTypeNeedle) );
   if ( iPosStart != -1 ) {
      iPosEnd = sHeader->pos( iPosStart, crlf, iCrlfLen );
      if ( iPosEnd == -1 ) {
         iPosEnd = sHeader->getLength();
      }

      iValStart   = iPosStart + strlen( contentTypeNeedle );
      iValLen     = iPosEnd - iValStart;

      tmp.setValue( sHeader->getPointer( iValStart ), iValLen );

      contenttype.setValue( &tmp );
   }

   iPosStart = sHeader->pos( 0, locationNeedle, strlen(locationNeedle) );
   if ( iPosStart != -1 ) {
      iPosEnd = sHeader->pos( iPosStart, crlf, iCrlfLen );
      if ( iPosEnd == -1 ) {
         iPosEnd = sHeader->getLength();
      }

      iValStart   = iPosStart + strlen( locationNeedle );
      iValLen     = iPosEnd - iValStart;

      tmp.setValue( sHeader->getPointer( iValStart ), iValLen );

      location.setValue( &tmp );
   }

   iPosStart = sHeader->pos( 0, connectionNeedle, strlen(connectionNeedle) );
   if ( iPosStart != -1 ) {
      iPosEnd = sHeader->pos( iPosStart, crlf, iCrlfLen );
      if ( iPosEnd == -1 ) {
         iPosEnd = sHeader->getLength();
      }

      iValStart   = iPosStart + strlen( connectionNeedle );
      iValLen     = iPosEnd - iValStart;

      tmp.setValue( sHeader->getPointer( iValStart ), iValLen );

      connection.setValue( &tmp );
   }

   iPosStart = sHeader->pos( 0, transferencNeedle, strlen(transferencNeedle) );
   if ( iPosStart != -1 ) {
      iPosEnd = sHeader->pos( iPosStart, crlf, iCrlfLen );
      if ( iPosEnd == -1 ) {
         iPosEnd = sHeader->getLength();
      }

      iValStart   = iPosStart + strlen( transferencNeedle );
      iValLen     = iPosEnd - iValStart;

      tmp.setValue( sHeader->getPointer( iValStart ), iValLen );

      chunked = tmp.match( chunkedNeedle, strlen(chunkedNeedle) );
   }

   if ( chunked ) {
      contentlength = -1;
   }

   int iSetCookies = 0;
   int iSearchPos = 0;

   iPosStart = 0;
   while ( iPosStart != -1 ) {
      iPosStart = sHeader->pos( iSearchPos, setcookieNeedle, strlen(setcookieNeedle) );
      if ( iPosStart != -1 ) {
         iPosEnd = sHeader->pos( iPosStart, crlf, iCrlfLen );
         if ( iPosEnd == -1 ) {
            iPosEnd = sHeader->getLength();
         }

         iSearchPos = iPosEnd + strlen(crlf);

         iValStart   = iPosStart + strlen( setcookieNeedle );
         iValLen     = iPosEnd - iValStart;

         iSetCookies++;
         tmp.setValue( sHeader->getPointer( iValStart ), iValLen );
         TGFString *cookie = new TGFString();
         cookie->setValue( &tmp );
         cookies->addElement( cookie );
      }
   }
}



TJRHttpClient::TJRHttpClient( TJRLookupBase *pLookupBase ) : TGFThread() {
   this->pLookupBase = pLookupBase;
   socket         = NULL;
   connection     = NULL;

   useHttpVersion.set( 1.0 );

   cookie.setValue( "", 0 );
   referer.setValue( "", 0 );
   useragent.setValue( "", 0 );

   reset();
}

TJRHttpClient::~TJRHttpClient() {
   reset();
}

void TJRHttpClient::reset() {
   if ( connection != NULL ) {
      if ( bAlternateSocketUsed ) {
         connection->setSocket( NULL );
      } else {
         socket = NULL;
      }

      delete connection;

      connection = NULL;
   }

   if ( (!bAlternateSocketUsed) && (socket != NULL) ) {
      delete socket;

      socket = NULL;
   }

   bAlternateSocketUsed = false;

   port           = 80;
   bHeaderParsed  = false;
   iHeaderSize    = 0;

   iChunkScanLastSize   = -1;
   bWaitForChunk  = false;

   lastChunkRange.start_ind   = 0;
   lastChunkRange.start_pos   = 0;
   lastChunkRange.end_ind     = 0;
   lastChunkRange.end_pos     = 0;

   rLastSend.start_ind = 0;
   rLastSend.start_pos = 0;
   rLastSend.end_ind = 0;
   rLastSend.end_pos = 0;

   iChunkCount    = 0;
}

void TJRHttpClient::get( const TJRURI *uri, bool bBlocking, TJRBaseSocket *anAlternateSocket ) {
   reset();

   this->host.setValue( &uri->host );
   if ( this->host.size() == 0 ) {
      if ( pLookupBase != NULL ) {
         pLookupBase->lookupHost( &this->host );
      } else {
         JRresolveAll( &this->host );
      }
   }

   this->path.setValue( &uri->path );
   this->port = uri->port.get();

   if ( !this->path.startsWith( "/", 1 ) ) {
      this->path.prepend( "/", 1 );
   }

   if ( uri->query.getLength() > 0 ) {
      this->path.append( "?", 1 );
      this->path.append( &uri->query );
   }

   TJRIPAddress *aDefaultIp = this->host.getAddress();
   if ( aDefaultIp != NULL ) {
      if ( anAlternateSocket != NULL ) {
         socket = anAlternateSocket;
         bAlternateSocketUsed = true;
      } else {
         socket = new TJRClientSocket();
      }
      socket->getRemoteAddress()->setValue( aDefaultIp );
      socket->remotePort.set( this->port );
      if ( socket->connect() ) {
         connection = new TJRHttpConnection( socket );

         connection->start();
         start();

         onStatusUpdate.execute( JRHTTPSTATUS_CONNECTED );

         sendGet();

         if ( bBlocking ) {
            while ( isRunning() ) {
               GFMillisleep( 50 );
            }
         }
      } else {
         reset();

         onStatusUpdate.execute( JRHTTPSTATUS_ERROR );
      }
   } else {
      reset();

      onStatusUpdate.execute( JRHTTPSTATUS_ERROR );
   }
}

void TJRHttpClient::post( const TJRURI *uri, const TGFString *sDataType, const TGFString *sData, bool bBlocking, TJRBaseSocket *anAlternateSocket ) {
   reset();

   this->host.setValue( &uri->host );
   if ( this->host.size() == 0 ) {
      if ( pLookupBase != NULL ) {
         pLookupBase->lookupHost( &this->host );
      } else {
         JRresolveAll( &this->host );
      }
   }

   this->path.setValue( &uri->path );
   this->port = uri->port.get();

   if ( !this->path.startsWith( "/", 1 ) ) {
      this->path.prepend( "/", 1 );
   }

   if ( uri->query.getLength() > 0 ) {
      this->path.append( "?", 1 );
      this->path.append( &uri->query );
   }


   TJRIPAddress *aDefaultIp = this->host.getAddress();
   if ( aDefaultIp != NULL ) {
      if ( anAlternateSocket != NULL ) {
         socket = anAlternateSocket;
         bAlternateSocketUsed = true;
      } else {
         socket = new TJRClientSocket();
      }
      socket->getRemoteAddress()->setValue( aDefaultIp );
      socket->remotePort.set( this->port );
      if ( socket->connect() ) {
         connection = new TJRHttpConnection( socket );

         connection->start();
         start();

         onStatusUpdate.execute( JRHTTPSTATUS_CONNECTED );

         sendPost( sDataType, sData );

         if ( bBlocking ) {
            while ( isRunning() ) {
               GFMillisleep( 50 );
            }
         }
      } else {
         reset();

         onStatusUpdate.execute( JRHTTPSTATUS_ERROR );
      }
   } else {
      reset();

      onStatusUpdate.execute( JRHTTPSTATUS_ERROR );
   }
}

void TJRHttpClient::sendGet() {
   TGFBValue val;

   int iCrlfLen = strlen(crlf);

   TGFString *sCmd = new TGFString();
   sCmd->append( sendgetNeedle, strlen(sendgetNeedle) );
   sCmd->append( &this->path );
   sCmd->append( sendhttpNeedle, strlen(sendhttpNeedle) );

   val.precision.set( 1 );
   val.setDouble( useHttpVersion.get() );
   sCmd->append( val.asString() );

   sCmd->append( crlf, iCrlfLen );

   sCmd->append( hostNeedle, strlen(hostNeedle) );
   sCmd->append( &this->host.name );
   sCmd->append( crlf, iCrlfLen );

   if ( useragent.getLength() > 0 ) {
      sCmd->append( useragentNeedle, strlen(useragentNeedle) );
      sCmd->append( &this->useragent );
      sCmd->append( crlf, iCrlfLen );
   }

   if ( referer.getLength() > 0 ) {
      sCmd->append( refererNeedle, strlen(refererNeedle) );
      sCmd->append( &this->referer );
      sCmd->append( crlf, iCrlfLen );
   }

   if ( cookie.getLength() > 0 ) {
      sCmd->append( cookieNeedle, strlen(cookieNeedle) );
      sCmd->append( &this->cookie );
      sCmd->append( crlf, iCrlfLen );
   }

   sCmd->append( crlf, iCrlfLen );
   socket->send( sCmd );

   delete sCmd;
}

void TJRHttpClient::sendPost( const TGFString *sDataType, const TGFString *sData ) {
   TGFBValue val;

   int iCrlfLen = strlen(crlf);

   TGFString *sCmd = new TGFString();
   sCmd->append( sendpostNeedle, strlen(sendpostNeedle) );
   sCmd->append( &this->path );
   sCmd->append( sendhttpNeedle, strlen(sendhttpNeedle) );

   val.precision.set( 1 );
   val.setDouble( useHttpVersion.get() );
   sCmd->append( val.asString() );

   sCmd->append( crlf, iCrlfLen );

   sCmd->append( hostNeedle, strlen(hostNeedle) );
   sCmd->append( &this->host.name );
   sCmd->append( crlf, iCrlfLen );

   if ( useragent.getLength() > 0 ) {
      sCmd->append( useragentNeedle, strlen(useragentNeedle) );
      sCmd->append( &this->useragent );
      sCmd->append( crlf, iCrlfLen );
   }

   if ( referer.getLength() > 0 ) {
      sCmd->append( refererNeedle, strlen(refererNeedle) );
      sCmd->append( &this->referer );
      sCmd->append( crlf, iCrlfLen );
   }

   if ( cookie.getLength() > 0 ) {
      sCmd->append( cookieNeedle, strlen(cookieNeedle) );
      sCmd->append( &this->cookie );
      sCmd->append( crlf, iCrlfLen );
   }

   sCmd->append( contentTypeNeedle, strlen(contentTypeNeedle) );
   sCmd->append( sDataType );
   sCmd->append( crlf, iCrlfLen );

   val.setInteger( sData->getLength() );
   sCmd->append( contentLenNeedle, strlen(contentLenNeedle) );
   sCmd->append( val.asString() );
   sCmd->append( crlf, iCrlfLen );

   sCmd->append( crlf, iCrlfLen );
   socket->send( sCmd );

   socket->send( sData );
   delete sCmd;
}

void TJRHttpClient::done() {
   parseReceivedData();
   stop();

   onStatusUpdate.execute( JRHTTPSTATUS_DONE );
}

void TJRHttpClient::locateAndParseHeader() {
   connection->bufferlock.lockWhenAvailable( GFLOCK_INFINITEWAIT );

   TGFStringVectorRange *tmpRange = new TGFStringVectorRange(0,0);

   bool bPos = connection->receivedData.pos( tmpRange, crlfcrlf, strlen(crlfcrlf) );
   if ( bPos ) {
      lastChunkRange.copyValues( tmpRange );
      lastChunkRange.start_ind = 0;
      lastChunkRange.start_pos = 0;

      rHeader.copyValues( &lastChunkRange );

      lastChunkRange.end_pos++;

      TGFString *sHeaderData = connection->receivedData.copy( &rHeader );
      iHeaderSize = sHeaderData->getLength();

      header.parse( sHeaderData );
      parsedHeader.setValue( sHeaderData );
      delete sHeaderData;

      rLastSend.start_pos = iHeaderSize;
      rLastSend.end_pos = iHeaderSize;

      bHeaderParsed  = true;

      if ( !bHeaderParsed ) {
         stop();

         onStatusUpdate.execute( JRHTTPSTATUS_ERROR );
      } else {
         onStatusUpdate.execute( JRHTTPSTATUS_HEADER );
      }
   }

   delete tmpRange;

   connection->bufferlock.unlock();
}


void TJRHttpClient::execute() {
   if ( !connection->isRunning() ) {
      done();
   } else {

      if ( !bHeaderParsed ) {
         locateAndParseHeader();
      } else {
         if ( header.contentlength != -1 ) {
            connection->bufferlock.lockWhenAvailable( GFLOCK_INFINITEWAIT );
            unsigned long iDataReceived = connection->receivedData.getLength( 0 );

            if ( (iDataReceived - iHeaderSize) >= header.contentlength ) {
               connection->stopAndWait();
            }

            if ( iDataReceived > rLastSend.end_pos ) {
               rLastSend.start_pos = rLastSend.end_pos;
               rLastSend.end_pos = iDataReceived;

               TGFString *sData = connection->receivedData.copy( 0, rLastSend.start_pos, rLastSend.end_pos - rLastSend.start_pos );
               onContent.execute( sData );
               delete sData;
            }
            connection->bufferlock.unlock();
         }

         if ( header.chunked ) {
            // check on 0 bytes remaining
            if ( isEndOfChunkedData() ) {
               connection->stopAndWait();
            }
         }
      }

   }
}

void TJRHttpClient::parseReceivedData() {
   if ( bHeaderParsed ) {
      if ( !header.chunked ) {
         int iStart = iHeaderSize;
         connection->bufferlock.lockWhenAvailable( GFLOCK_INFINITEWAIT );

         if ( header.contentlength == -1 ) {
            header.contentlength = connection->receivedData.getLength( 0 ) - iStart;

            TGFString *sData = connection->receivedData.copy( 0, iStart, header.contentlength );
            onContent.execute( sData );
            delete sData;
         } else {
            unsigned long iDataReceived = connection->receivedData.getLength( 0 );
            if ( iDataReceived > rLastSend.end_pos ) {
               rLastSend.start_pos = rLastSend.end_pos;
               rLastSend.end_pos = iDataReceived;

               TGFString *sData = connection->receivedData.copy( 0, rLastSend.start_pos, rLastSend.end_pos - rLastSend.start_pos );
               onContent.execute( sData );
               delete sData;
            }

         }

         connection->bufferlock.unlock();
      }
   }
}

bool TJRHttpClient::isEndOfChunkedData() {
   TGFStringVectorRange *searchRange;

   if ( iChunkScanLastSize == 0 ) {
      return true;
   }

   searchRange = new TGFStringVectorRange();

   // nog geen chunklength string gevonden
   if ( iChunkScanLastSize == -1 ) {
      bWaitForChunk = true;

      searchRange->start_ind = lastChunkRange.end_ind;
      searchRange->start_pos = lastChunkRange.end_pos;

      connection->bufferlock.lockWhenAvailable( GFLOCK_INFINITEWAIT );

      bool bPos = connection->receivedData.pos( searchRange, crlf, strlen(crlf) );
      if ( bPos ) {
         bWaitForChunk = false;

         TGFStringVectorRange *tmpRange = new TGFStringVectorRange();
         tmpRange->start_ind = lastChunkRange.end_ind;
         tmpRange->start_pos = lastChunkRange.end_pos;
         tmpRange->end_ind   = searchRange->start_ind;
         tmpRange->end_pos   = searchRange->start_pos;
         connection->receivedData.endMinusOne( tmpRange );

         lastChunkRange.end_ind = searchRange->end_ind;
         lastChunkRange.end_pos = searchRange->end_pos + 1;

         TGFString *tmp = connection->receivedData.copy( tmpRange );
         tmp->ltrim_ansi();
         tmp->uppercase_ansi();

         iChunkScanLastSize = HexToInt( tmp );
         delete tmp;

         delete tmpRange;
      }

      connection->bufferlock.unlock();
   }

   delete searchRange;

   if ( iChunkScanLastSize == 0 ) {
      return true;
   }

   if ( !bWaitForChunk ) {
      // do stuff
      connection->bufferlock.lockWhenAvailable( GFLOCK_INFINITEWAIT );
      if ( ( connection->receivedData.getLength( lastChunkRange.end_ind ) - lastChunkRange.end_pos ) >= iChunkScanLastSize ) {
         TGFString *tmpData = connection->receivedData.copy( lastChunkRange.end_ind, lastChunkRange.end_pos, iChunkScanLastSize );
         onContent.execute( tmpData );
         delete tmpData;
         lastChunkRange.end_pos += iChunkScanLastSize + strlen(crlf);
         iChunkScanLastSize = -1;
      }
      connection->bufferlock.unlock();
   }

   return false;
}

/*
TGFString *TJRHttpClient::getParsedData() {
   return &parsedData;
}
*/

TGFString *TJRHttpClient::getHeader() {
   return &parsedHeader;
}
