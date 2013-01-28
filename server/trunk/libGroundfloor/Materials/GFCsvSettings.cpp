
#include "GFCsvSettings.h"

TGFCsvSettings::TGFCsvSettings() {
   show_columnnames.set( false );

   sep_column.setValue_ansi( ";" );
   sep_record.setValue_ansi( "\r\n" );
   sep_string.setValue_ansi( "\"" );
}

TGFCsvSettings::~TGFCsvSettings() {
}

TGFXmlSettings::TGFXmlSettings() {
   node_open.setValue_ansi( "<" );
   node_close.setValue_ansi( ">" );
   node_end.setValue_ansi( "/" );
   node_space.setValue_ansi( " " );
   sep_string.setValue_ansi( "\"" );
   sep_keyval.setValue_ansi( "=" );

   sep_node.setValue_ansi( "\r\n" );
}

TGFXmlSettings::~TGFXmlSettings() {
}
