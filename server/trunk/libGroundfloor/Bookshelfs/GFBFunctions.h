
#ifndef __GFBFunctions_H
#define __GFBFunctions_H

#include "../Materials/GFCsvSettings.h"
#include "../Molecules/GFString.h"
#include "../Bookshelfs/GFBTable.h"
#include "../Bookshelfs/GFBNode.h"

#include "../Materials/GFFileWriter.h"


void GFBCsvWrite_Table( TGFBaseWriter *aWriter, const TGFBTable *aSource, TGFCsvSettings *aSettings );
void GFBCsvExport_Table( const TGFString *sDestination, const TGFBTable *aSource, TGFCsvSettings *aSettings );

void GFBXmlWrite_Node( TGFBaseWriter *aWriter, TGFBNode *aSource, TGFXmlSettings *aSettings, bool bWriteDefaultHeader );
void GFBXmlExport_Node( const TGFString *sDestination, TGFBNode *aSource, TGFXmlSettings *aSettings, bool bWriteDefaultHeader );

TGFBNode *GFBXmlImport_FromString( const TGFString *sXML );
TGFBNode *GFBXmlImport_FromFile( const TGFString *sSourceFile );

/// lossy function to try and generate a table from Nodes (attributes are lost, contents are translated to recordvalues)
TGFBTable *GFBNodeContentsToTable( const TGFBNode *aNode, int iDefaultDatatype = FLDTYPE_STRING );
/// lossy function to try and generate a table from Nodes (attributes are translated to recordvalues, contents are lost)
TGFBTable *GFBNodeAttributesToTable( const TGFBNode *aNode, int iDefaultDatatype = FLDTYPE_STRING );

/// semi-lossless function to generate Nodes from a table (every recordvalue is a seperate node)
TGFBNode *GFBTableToNodeContents( const TGFBTable *aTable, const char *sTableNodeName, const char *sRecordNodeName );
/// semi-lossless function to generate Nodes from a table (every recordvalue is a seperate attribute)
TGFBNode *GFBTableToNodeAttributes( const TGFBTable *aTable, const char *sTableNodeName, const char *sRecordNodeName );

#endif // __GFBFunctions_H

