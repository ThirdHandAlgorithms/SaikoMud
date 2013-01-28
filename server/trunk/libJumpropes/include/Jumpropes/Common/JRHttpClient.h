
#ifndef __JRHttpClient_H
#define __JRHttpClient_H


class TJRHttpHeader;
class TJRHttpClient;

#include <Groundfloor/Molecules/GFCallbacks.h>
#include <Groundfloor/Molecules/GFProperty.h>
#include <Groundfloor/Materials/GFFileWriter.h>
#include "JRHttpConnection.h"
#include "../JRClientSocket.h"

#include "../JRURI.h"
#include "../JRLookupCache.h"

#define JRHTTPSTATUS_CONNECTED      1
#define JRHTTPSTATUS_DISCONNECTED   2
#define JRHTTPSTATUS_ERROR          3
#define JRHTTPSTATUS_HEADER         4
#define JRHTTPSTATUS_DONE           5

class TJRHttpHeader: public TGFFreeable {
   protected:
      void resetVars();
   public:
      TJRHttpHeader();
      ~TJRHttpHeader();

      void parse( TGFString *sHeader );

      double httpversion;
      int httpstatus;
      TGFString statusstring;

      TGFString location;

      TGFString contenttype;
      TGFString connection;
      long contentlength;

      TGFVector *cookies;

      bool chunked;
};

class TJRHttpClient: public TGFThread {
   protected:
      TJRLookupBase *pLookupBase;

      TJRLookupObject host;
      TGFString path;
      int port;

      bool bHeaderParsed;
      long iHeaderSize;
      TGFStringVectorRange rHeader;

      TGFStringVectorRange lastChunkRange;
      long iChunkScanLastSize;
      bool bWaitForChunk;

      int iChunkCount;

      TGFStringVectorRange rLastSend;

      TJRHttpConnection *connection;
      TJRBaseSocket *socket;
      bool bAlternateSocketUsed;

      TGFString parsedHeader;

      void reset();
      void parseReceivedData();
      void sendGet();
      void parseHeader( int iSize );

      bool isEndOfChunkedData();

      void sendPost( const TGFString *sDataType, const TGFString *sData );

      void done();
      void locateAndParseHeader();

   public:
      TJRHttpClient( TJRLookupBase *pLookupBase );
      ~TJRHttpClient();

      TJRHttpHeader header;

      TGFMultiNotify<int> onStatusUpdate;
      TGFProperty<double> useHttpVersion;

      TGFMultiNotify<TGFString *> onContent;

      TGFString cookie;
      TGFString referer;
      TGFString useragent;

      void get( const TJRURI *uri, bool bBlocking = false, TJRBaseSocket *anAlternateSocket = NULL );
      void post( const TJRURI *uri, const TGFString *sDataType, const TGFString *sData, bool bBlocking = false, TJRBaseSocket *anAlternateSocket = NULL );

      TGFString *getHeader();

      void execute();
};


#endif // __JRHttpClient_H
