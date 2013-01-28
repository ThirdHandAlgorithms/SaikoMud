
#ifndef __JRHttpDownloader_H
#define __JRHttpDownloader_H

#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Materials/GFFileWriter.h>
#include "JRHttpClient.h"

class TJRHttpDownloader: TGFFreeable {
   protected:
      TGFFileWriter *fw;
      TJRHttpClient *client;
      TJRURI *uri;
   public:
      TJRHttpDownloader( TJRLookupBase *aLookup, double dHttpVersion, const char *sUseragent );
      ~TJRHttpDownloader();

      void download( const char *sUri, const char *sOutputFile );
      void waitToFinish();

      void onData( TGFString *s );
};

#endif //__JRHttpDownloader_H
