
#include "GFBFunctions.h"

#include "../Materials/GFFileWriter.h"
#include "../Materials/GFFunctions.h"

//-----------------------------------------------------------------------------


void GFBCsvWrite_Table( TGFBaseWriter *aWriter, const TGFBTable *aSource, TGFCsvSettings *aSettings ) {
   TGFString *sEscapedStr = new TGFString( "", 0 );
   sEscapedStr->setSize( aSettings->sep_string.getLength() * 2 + 1 );
   sEscapedStr->append( &aSettings->sep_string );
   sEscapedStr->append( &aSettings->sep_string );

   TGFString *sVal = new TGFString( "", 0 );

   TGFBRecords *records = aSource->records.get();
   TGFBFields *fields = aSource->fields.get();

   if ( aSettings->show_columnnames.get() ) {

      unsigned int s1 = fields->size();
      for ( unsigned int i = 0; i < s1; i++ ) {
         TGFBField *field = fields->getField(i);

         sVal->setValue( &field->name );
         sVal->replace( &aSettings->sep_string, sEscapedStr );

         aWriter->add( &aSettings->sep_string );
         aWriter->add( &field->name );
         aWriter->add( &aSettings->sep_string );

         if ( i == s1 - 1 ) {
            aWriter->add( &aSettings->sep_record );
         } else {
            aWriter->add( &aSettings->sep_column );
         }
      }

   }

   unsigned int s2 = records->size();
   for ( unsigned int j = 0; j < s2; j++ ) {
      TGFBRecord *record = records->getRecord(j);

      unsigned int s3 = record->size();
      for ( unsigned int k = 0; k < s3; k++ ) {
         int iDatatype = fields->getField(k)->datatype.get();

         sVal->setValue( record->getValue(k)->asString() );
         sVal->replace( &aSettings->sep_string, sEscapedStr );

         if ( iDatatype == FLDTYPE_STRING ) {
            aWriter->add( &aSettings->sep_string );
            aWriter->add( sVal );
            aWriter->add( &aSettings->sep_string );
         } else {
            aWriter->add( sVal );
         }

         if ( k < s3 - 1 ) {
            aWriter->add( &aSettings->sep_column );
         }

      }

      aWriter->add( &aSettings->sep_record );
   }

   delete sVal;

   delete sEscapedStr;
}

void GFBCsvExport_Table( const TGFString *sDestination, const TGFBTable *aSource, TGFCsvSettings *aSettings ) {
   TGFFileWriter *fw = new TGFFileWriter();
   fw->open( sDestination, false );

   fw->setInterval( 0 );
   fw->start();

   GFBCsvWrite_Table( fw, aSource, aSettings );
   fw->setStopWhenEmpty( true );

   while ( fw->isRunning() ) {
      GFMillisleep( 10 );
   }

   fw->close();
   delete fw;
}

//-----------------------------------------------------------------------------

void GFBXmlExport_NodeIteration( TGFBaseWriter *fw, TGFBNode *aSource, TGFXmlSettings *aSettings ) {
   bool bIsTag = (aSource->name.getLength() != 0);

   if ( bIsTag ) {
       fw->add( &aSettings->node_open );
       fw->add( &aSource->name );

       if ( aSource->childCount() != 0 ) {
          if ( aSource->attributeCount() > 0 ) {
             unsigned int c = aSource->attributeCount();
             for ( unsigned int i = 0; i < c; i++ ) {
                TGFBAttribute *anAttrib = aSource->getAttribute( i );
                if ( anAttrib != NULL ) {
                   if ( !anAttrib->hideFromXML ) {
                      fw->add( &aSettings->node_space );
                      fw->add( &anAttrib->key );
                      fw->add( &aSettings->sep_keyval );
                      fw->add( &aSettings->sep_string );

                      TGFString tmp( anAttrib->value.asString() );
                      GFEncodeHtmlEntities( &tmp );
                      fw->add( &tmp );

                      fw->add( &aSettings->sep_string );
                   }
                }
             }
          }
          fw->add( &aSettings->node_close );

          bool bMayWriteSeperator = false;

          unsigned int d = aSource->childCount();
          for ( unsigned int j = 0; j < d; j++ ) {
             TGFBNode *aChild = aSource->getChild( j );
             if ( aChild != NULL ) {
                if ( aChild->name.getLength() != 0 ) {
                   bMayWriteSeperator = true;
                   fw->add( &aSettings->sep_node );
                }
                GFBXmlExport_NodeIteration( fw, aChild, aSettings );
             }
          }

          if ( bMayWriteSeperator ) {
             fw->add( &aSettings->sep_node );
          }

          fw->add( &aSettings->node_open );
          fw->add( &aSettings->node_end );
          fw->add( &aSource->name );
          fw->add( &aSettings->node_close );
       } else {
          if ( aSource->attributeCount() > 0 ) {
             unsigned int c = aSource->attributeCount();
             for ( unsigned int i = 0; i < c; i++ ) {
                TGFBAttribute *anAttrib = aSource->getAttribute( i );
                if ( anAttrib != NULL ) {
                   if ( !anAttrib->hideFromXML ) {
                      fw->add( &aSettings->node_space );
                      fw->add( &anAttrib->key );
                      fw->add( &aSettings->sep_keyval );
                      fw->add( &aSettings->sep_string );
                      
                      TGFString tmp( anAttrib->value.asString() );
                      GFEncodeHtmlEntities( &tmp );
                      fw->add( &tmp );

                      fw->add( &aSettings->sep_string );
                   }
                }
             }
          }

          if ( aSource->content.getLength() > 0 ) {
             fw->add( &aSettings->node_close );

             TGFString tmp( &aSource->content );
             GFEncodeHtmlEntities( &tmp );
             fw->add( &tmp );

             fw->add( &aSettings->node_open );
             fw->add( &aSettings->node_end );
             fw->add( &aSource->name );
             fw->add( &aSettings->node_close );
          } else {
             fw->add( &aSettings->node_space );
             fw->add( &aSettings->node_end );
             fw->add( &aSettings->node_close );
          }
       }

   } else { // not bIsTag

      unsigned int d = aSource->childCount();
      for ( unsigned int j = 0; j < d; j++ ) {
         TGFBNode *aChild = aSource->getChild( j );
         if ( aChild != NULL ) {
            GFBXmlExport_NodeIteration( fw, aChild, aSettings );
         }
      }

      if ( aSource->content.getLength() > 0 ) {
         TGFString tmp( &aSource->content );
         GFEncodeHtmlEntities( &tmp );
         fw->add( &tmp );
      }
   }
}

void GFBXmlWrite_Node( TGFBaseWriter *aWriter, TGFBNode *aSource, TGFXmlSettings *aSettings, bool bWriteDefaultHeader ) {
   if ( bWriteDefaultHeader ) {
      TGFString *sHeader = new TGFString();
      sHeader->setValue_ansi( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" );
      aWriter->add( sHeader );
      aWriter->add( &aSettings->sep_node );
      delete sHeader;
   }

   GFBXmlExport_NodeIteration( aWriter, aSource, aSettings );
}

void GFBXmlExport_Node( const TGFString *sDestination, TGFBNode *aSource, TGFXmlSettings *aSettings, bool bWriteDefaultHeader ) {
   TGFFileWriter *fw = new TGFFileWriter();
   fw->open( sDestination, false );

   fw->setInterval( 0 );
   fw->start();

   GFBXmlWrite_Node( fw, aSource, aSettings, bWriteDefaultHeader );
   fw->setStopWhenEmpty( true );

   while ( fw->isRunning() ) {
      GFMillisleep( 10 );
   }

   fw->close();
   delete fw;
}


//-----------------------------------------------------------------------------

#define ctagopen '<'
#define ctagclose '>'
#define ctagslash '/'
#define cspace ' '

#define cattrsep '='
#define cattrquote1 '"'
#define cattrquote2 '\''
#define cattrescape '\\'
#define cheadersign '?'
#define ccommentsign '!'

#include "../Materials/GFFunctions.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))

void parseAttributes( TGFBNode *node, const TGFString *s ) {
    TGFString tmp;

    bool bIsQuote1 = false;
    bool bIsQuote2 = false;
    bool bIsEscape = false;

    int iLastAttrSep = -1;
    int iLastQuote1 = -1;
    int iLastQuote2 = -1;

    int iStartNewAttr = 0;

    char *cs = s->getValue();
    unsigned int d = s->getLength();
    for ( unsigned int j = 0; j < d; j++ ) {
        char c = cs[j];

        if ( !(bIsQuote1 || bIsQuote2 || bIsEscape) ) {

            switch (c) {
                case cattrsep:
                    iLastAttrSep = j;
                    break;
                case cattrquote1:
                    bIsQuote1 = true;
                    iLastQuote1 = j;
                    break;
                case cattrquote2:
                    bIsQuote2 = true;
                    iLastQuote2 = j;
                    break;
            }

        } else {

            int iLastQuotePos = -1;
            bool bEoq = false;
            if ( bIsEscape ) {
                bIsEscape = false;
            } else {
                switch (c) {
                    case cattrquote1:
                        if ( bIsQuote1 ) {

                            iLastQuotePos = iLastQuote1;
                            bEoq = true;

                            bIsQuote1 = false;
                        }
                        break;
                    case cattrquote2:
                        if ( bIsQuote2 ) {
                            iLastQuotePos = iLastQuote2;
                            bEoq = true;

                            bIsQuote2 = false;
                        }
                        break;
                    case cattrescape:
                        bIsEscape = true;
                        break;
                }
            }

            if ( bEoq ) {
                // value = iLastQuote1 ... j
                // key = iStartNewAttr ... iLastAttrSep - 1
                TGFBAttribute *a = new TGFBAttribute();
                a->key.setValue( s->getPointer(iStartNewAttr), iLastAttrSep - iStartNewAttr );
                a->key.ltrim_ansi();
                a->key.rtrim_ansi();

                TGFString tmp;
                tmp.setValue( s->getPointer(iLastQuotePos), j - iLastQuotePos + 1 );
                tmp.ltrim_ansi();
                tmp.rtrim_ansi();

                if ( tmp.startsWith( "\"", 1 ) && tmp.endsWith( "\"", 1 ) ) {
                    tmp.setValue( tmp.getPointer(1), tmp.getLength() - 2 );
                }
                if ( tmp.startsWith( "'", 1 ) && tmp.endsWith( "'", 1 ) ) {
                    tmp.setValue( tmp.getPointer(1), tmp.getLength() - 2 );
                }

                GFDecodeHtmlEntities( &tmp );
                a->value.setString( &tmp );
                

                iStartNewAttr = j + 1;

                node->addAttribute( a );
            }
        }

    }
}


TGFBNode *parseNode( TGFBNode *parent, TGFString *s ) {
    TGFBNode *node = NULL;

    if ( s->getLength() != 0 ) {
        if ( s->startsWith( "</", 2 ) ) {
            if ( s->endsWith( ">", 1 ) ) {
                // is closetag
                TGFString tmp;
                tmp.setValue( s->getPointer(2), s->getLength() - 3 );
                if ( parent != NULL ) {
                    if ( tmp.match( &parent->name ) ) {
                        return parent->getParentNode();
                    } else {
                        // what?
                        printf( "s=[%s], parent=[%s], tmp=[%s]\n", s->getValue(), parent->name.getValue(), tmp.getValue() );
                        printf( "error 1\n" );
                        exit(1);
                    }
                } else {
                    // !?!?
                    printf( "error 2\n" );
                    exit(1);
                }
            } else {
                // invalid?
                printf( "error 3\n" );
                exit(1);
            }
        } else if ( s->startsWith( "<?", 2 ) ) {
            // header, skip
        } else if ( s->startsWith( "<!", 2 ) ) {
            // comment, skip
        } else if ( s->startsWith( "<", 1 ) ) {
            if ( s->endsWith( "/>", 2 ) ) {
                // is empty tag
                bool bHasAttr = true;
                int iPos = s->pos( 0, " ", 1 );
                if ( iPos == -1 ) {
                    iPos = s->pos( 0, "/", 1 );
                    bHasAttr = false;
                }

                node = new TGFBNode();
                node->name.setValue( s->getPointer(1), iPos - 1 );

                if ( bHasAttr ) {
                    TGFString *sAttr = new TGFString( s->getPointer( iPos + 1 ), s->getLength() - iPos - 3 );
                    parseAttributes( node, sAttr );
                    delete sAttr;
                }

                if ( parent != NULL ) {
                    parent->addChildNode( node );
                }

                return parent;
            } else if ( s->endsWith( ">", 1 ) ) {
                bool bHasAttr = true;
                int iPos = s->pos( 0, " ", 1 );
                if ( iPos == -1 ) {
                    iPos = s->pos( 0, ">", 1 );
                    bHasAttr = false;
                }

                node = new TGFBNode();
                node->name.setValue( s->getPointer(1), iPos - 1 );

                if ( bHasAttr ) {
                    TGFString *sAttr = new TGFString( s->getPointer( iPos + 1 ), s->getLength() - iPos - 2 );
                    parseAttributes( node, sAttr );
                    delete sAttr;
                }

                if ( parent != NULL ) {
                    parent->addChildNode( node );
                }

                return node;
            } else {
                // invalid?
            }
        } else {
            // is content
            node = new TGFBNode();
            GFDecodeHtmlEntities( s );
            node->content.setValue( s );

            if ( parent != NULL ) {
                parent->addChildNode( node );
            }
            return parent;
        }
    }

    return NULL;
}



TGFBNode *GFBXmlImport_FromString( const TGFString *sXml ) {
    TGFBNode *rootNode = NULL;

    TGFString *s = NULL;

    TGFString myTagName;
    TGFBNode *currentNode = NULL;

    bool bIsQuote1 = false;
    bool bIsQuote2 = false;
    bool bIsEscape = false;
    bool bIsTag = false;

    s = new TGFString();

    unsigned int i = 0;
    unsigned int cLen = sXml->getLength();
    char *cXml = sXml->getValue();
    while ( i < cLen ) {
        unsigned char c = cXml[i];
        i++;

        if ( !(bIsQuote1 || bIsQuote2) ) {

            if ( bIsTag ) {
                s->append( c );

                switch (c) {
                    case cattrquote1:
                        bIsQuote1 = true;
                        break;
                    case cattrquote2:
                        bIsQuote2 = true;
                        break;
                    case ctagclose:
                        bIsTag = false;

                        s->ltrim_ansi();
                        if ( s->getLength() != 0 ) {
                            currentNode = parseNode( currentNode, s );
                            if ( rootNode == NULL ) {
                                rootNode = currentNode;
                            }

                            delete s;
                            s = new TGFString();
                        }
                        break;
                }

            } else {

                switch (c) {
                    case ctagopen:
                        bIsTag = true;

                        s->ltrim_ansi();
                        if ( s->getLength() != 0 ) {
                            currentNode = parseNode( currentNode, s );
                            if ( rootNode == NULL ) {
                                rootNode = currentNode;
                            }

                            delete s;
                            s = new TGFString();
                        }
                        break;
                }


                s->append( c );
            }

        } else {    // if quote

            if ( bIsEscape ) {
                bIsEscape = false;
            } else {
                switch (c) {
                    case cattrquote1:
                        bIsQuote1 = false;
                        break;
                    case cattrquote2:
                        bIsQuote2 = false;
                        break;
                    case cattrescape:
                        bIsEscape = true;
                        break;
                }
            }

            s->append( c );
        }
    }

    s->ltrim_ansi();
    if ( s->getLength() != 0 ) {
        currentNode = parseNode( currentNode, s );
    }
    delete s;

    return rootNode;
}



TGFBNode *GFBXmlImport_FromFile( const TGFString *sSourceFile ) {
   TGFString *s = new TGFString();

   TGFFileCommunicator *fc = new TGFFileCommunicator();
   fc->filename.set( sSourceFile->getValue() );
   fc->mode.set( GFFILEMODE_READ );
   fc->connect();

   TGFCommReturnData *err = new TGFCommReturnData();

   TGFString *buf = new TGFString();
   buf->setSize( 1024 );
   while ( !err->eof ) {
      fc->receive( buf, err );
      s->append( buf );
   }
   delete buf;

   delete err;

   fc->disconnect();
   delete fc;

   TGFBNode *node = GFBXmlImport_FromString( s );
   delete s;

   return node;
}

//-----------------------------------------------------------------------------


TGFBTable *GFBNodeContentsToTable( const TGFBNode *aNode, int iDefaultDatatype ) {
   TGFBTable *table = new TGFBTable();
   TGFBFields *fields = new TGFBFields();
   TGFBRecords *records = new TGFBRecords();

   unsigned int crecords = aNode->childCount();
   for ( unsigned int r = 0; r < crecords; r++ ) {
      TGFBNode *recordnode = aNode->getChild(r);
      TGFBRecord *record = records->newRecord();

      unsigned int c = recordnode->childCount();
      for ( unsigned int i = 0; i < c; i++ ) {
         TGFBNode *subnode = recordnode->getChild(i);
         int ifield = fields->getFieldIndex( &subnode->name );
         if ( ifield == -1 ) {
            ifield = fields->size();
            TGFBField *field = fields->newField();
            field->name.setValue( &subnode->name );
            field->datatype.set( iDefaultDatatype );
         }

         record->setValueCount( ifield + 1);

         TGFBValue *value = record->getValue(ifield);
         value->setString( &subnode->content );
      }
   }

   table->fields.set( fields );
   table->records.set( records );

   return table;
}

TGFBTable *GFBNodeAttributesToTable( const TGFBNode *aNode, int iDefaultDatatype ) {
   TGFBTable *table = new TGFBTable();
   TGFBFields *fields = new TGFBFields();
   TGFBRecords *records = new TGFBRecords();

   unsigned int crecords = aNode->childCount();
   for ( unsigned int r = 0; r < crecords; r++ ) {
      TGFBNode *recordnode = aNode->getChild(r);
      TGFBRecord *record = records->newRecord();

      unsigned int c = recordnode->attributeCount();
      for ( unsigned int i = 0; i < c; i++ ) {
         TGFBAttribute *attr = recordnode->getAttribute(i);
         int iattr = fields->getFieldIndex( &attr->key );
         if ( iattr == -1 ) {
            iattr = fields->size();
            TGFBField *field = fields->newField();
            field->name.setValue( &attr->key );
            field->datatype.set( iDefaultDatatype );
         }

         record->setValueCount( iattr + 1 );

         TGFBValue *value = record->getValue(iattr);
         value->setValue( &attr->value );
      }
   }

   table->fields.set( fields );
   table->records.set( records );

   return table;
}

//-----------------------------------------------------------------------------

TGFBNode *GFBTableToNodeContents( const TGFBTable *aTable, const char *sTableNodeName, const char *sRecordNodeName ) {
   TGFBNode *tablenode = NULL;
   if ( aTable != NULL ) {
      TGFBFields *fields = aTable->fields.get();
      TGFBRecords *records = aTable->records.get();

      if ( (fields != NULL) && (records != NULL) ) {
         tablenode = new TGFBNode( sTableNodeName );

         unsigned int cfields = fields->size();
         unsigned int crecords = records->size();
         for ( unsigned int r = 0; r < crecords; r++ ) {
            TGFBRecord *record = records->getRecord(r);
            TGFBNode *recordnode = new TGFBNode( sRecordNodeName );

            for ( unsigned int f = 0; f < cfields; f++ ) {
               TGFBValue *value = record->getValue(f);
               TGFBField *field = fields->getField(f);

               TGFBNode *fieldnode = new TGFBNode( field->name.getValue() );
               fieldnode->content.setValue( value->asString() );

               recordnode->addChildNode( fieldnode );
            }

            tablenode->addChildNode( recordnode );
         }
      }
   }

   return tablenode;
}

TGFBNode *GFBTableToNodeAttributes( const TGFBTable *aTable, const char *sTableNodeName, const char *sRecordNodeName ) {
   TGFBNode *tablenode = NULL;
   if ( aTable != NULL ) {
      TGFBFields *fields = aTable->fields.get();
      TGFBRecords *records = aTable->records.get();

      if ( (fields != NULL) && (records != NULL) ) {
         tablenode = new TGFBNode( sTableNodeName );

         unsigned int cfields = fields->size();
         unsigned int crecords = records->size();
         for ( unsigned int r = 0; r < crecords; r++ ) {
            TGFBRecord *record = records->getRecord(r);
            TGFBNode *recordnode = new TGFBNode( sRecordNodeName );

            for ( unsigned int f = 0; f < cfields; f++ ) {
               TGFBValue *value = record->getValue(f);
               TGFBField *field = fields->getField(f);

               TGFBAttribute *attr = new TGFBAttribute( field->name.getValue(), value->asString()->getValue() );
               recordnode->addAttribute( attr );
            }

            tablenode->addChildNode( recordnode );
         }
      }
   }

   return tablenode;
}
