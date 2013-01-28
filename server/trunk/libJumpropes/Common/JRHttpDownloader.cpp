
#include "JRHttpDownloader.h"

TJRHttpDownloader::TJRHttpDownloader( TJRLookupBase *aLookup, double dHttpVersion, const char *sUseragent ) : TGFFreeable() {
   fw = new TGFFileWriter();
   client = new TJRHttpClient( aLookup );
   uri = new TJRURI( aLookup );

   client->useHttpVersion.set( dHttpVersion );
   client->useragent.setValue_ansi( sUseragent );

   client->onContent.addNotify( GFCreateNotify( TGFString *, TJRHttpDownloader, this, &TJRHttpDownloader::onData ) );
}

TJRHttpDownloader::~TJRHttpDownloader() {
   delete uri;
   delete client;
   delete fw;
}

void TJRHttpDownloader::download( const char *sUri, const char *sOutputFile ) {
   uri->parseUri( sUri, true );

   // empty file if it already exists
   fw->open( sOutputFile, false );
   fw->close();

   fw->open( sOutputFile, true );
   fw->start();

   client->get( uri, true, NULL );
}

void TJRHttpDownloader::waitToFinish() {
   while ( client->isRunning() ) {
      GFMillisleep(10);
   }

   while ( !fw->isEmpty() ) {
      GFMillisleep(10);
   }

   fw->stopAndWait();
}

void TJRHttpDownloader::onData( TGFString *s ) {
   fw->add( s );
}
