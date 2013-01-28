
#include "GFResources.h"

#include "../Materials/GFFileCommunicator.h"
#include "../Materials/GFFileWriter.h"

#include "../Materials/GFFunctions.h"
#include "../Bookshelfs/GFBValue.h"

void writeHuffTable( TGFFileWriter *fw, const TGFString *name, const TGFString *table, unsigned int pass ) {
   char crlf[] = GFNATIVENEXTLINE;

   const char *p = table->getValue();

   unsigned int c = table->getLength();

   TGFBValue lenval, passval;
   lenval.setInteger(c);
   passval.setInteger(pass);

   TGFString txt;
   txt << "const unsigned int " << name << "_hufftablesize" << passval.asString() << " = " << lenval.asString() << ";" << crlf;
   txt << "const unsigned char " << name << "_hufftable" << passval.asString() << "[] = " << crlf << "{";
   for ( unsigned int i = 0; i < c; i++ ) {
      if ( i != 0 ) {
         txt << ",";

         if ( i % 20 == 0 ) {
            txt << crlf;
         }
      }

      txt << "0x";
      CharToHex( p[i], &txt );
   }
   txt << "};" << crlf;

   fw->add( &txt );
}

void writeContentTable( TGFFileWriter *fw, const TGFString *name, const TGFString *table ) {
   char crlf[] = GFNATIVENEXTLINE;

   const char *p = table->getValue();

   unsigned int c = table->getLength();

   TGFBValue lenval;
   lenval.setInteger(c);

   TGFString txt;
   txt << "const unsigned int " << name << "_contenttablesize = " << lenval.asString() << ";" << crlf;
   txt << "const unsigned char " << name << "_contenttable[] = " << crlf << "{";
   for ( unsigned int i = 0; i < c; i++ ) {
      if ( i != 0 ) {
         txt << ",";

         if ( i % 20 == 0 ) {
            txt << crlf;
         }
      }

      txt << "0x";
      CharToHex( p[i], &txt );
   }
   txt << "};" << crlf;

   fw->add( &txt );
}


void generateResourceFrom( const TGFString *source, const TGFString *destination, const TGFString *name ) {
   TGFCommReturnData err;
   TGFString buf, base64;

   TGFString hufftable1, hufftable2;


   char crlf[] = GFNATIVENEXTLINE;

   long w = 0;
   TGFString content;


   buf.setSize( 1024 );
   TGFFileCommunicator *fc = new TGFFileCommunicator();
   fc->filename.set( source->getValue() );
   fc->mode.set( GFFILEMODE_READ );
   fc->connect();
   while ( !err.eof ) {
      fc->receive( &buf, &err );
      content << &buf;
   }
   fc->disconnect();
   delete fc;
   buf.setSize( 1 );


   TGFString text;


   TGFFileWriter *fw = new TGFFileWriter();
   fw->open( destination, false );
   fw->start();

   text << crlf << "#ifndef __RESOURCE_" << name << "_H" << crlf;
   text << "#define __RESOURCE_" << name << "_H" << crlf << crlf;

   fw->add( &text );
   text.setLength(0);

   TGFString out1,out2;

   //content.setValue_wide( reinterpret_cast<wchar_t *>(content.getPointer(2)) );
   //content.transformWidestringToString( true, false );
   StrToCharHuffman( &content, &out1, &hufftable1, &hufftable2 );

   if ( out1.getLength() + hufftable1.getLength() + hufftable2.getLength() < content.getLength() ) {
      writeHuffTable( fw, name, &hufftable1, 1 );
      writeHuffTable( fw, name, &hufftable2, 2 );
      writeContentTable( fw, name, &out1 );
   } else {
      // no compression
      writeContentTable( fw, name, &content );
   }

   text << crlf << "#endif // __RESOURCE_" << name << "_H" << crlf;

   fw->add( &text );



   fw->setStopWhenEmpty( true );
   fw->waitWhileRunning();

   fw->close();

   delete fw;
}

