
#ifndef __GFCsvSettings_H
#define __GFCsvSettings_H

#include "../Atoms/GFFreeable.h"
#include "../Molecules/GFProperty.h"
#include "../Molecules/GFString.h"


class TGFCsvSettings: public TGFFreeable {
   public:
      TGFCsvSettings();
      ~TGFCsvSettings();

      TGFProperty<bool> show_columnnames;

      TGFString sep_column;
      TGFString sep_record;
      TGFString sep_string;
};

class TGFXmlSettings: public TGFFreeable {
   public:
      TGFXmlSettings();
      ~TGFXmlSettings();

      TGFString node_open;
      TGFString node_close;
      TGFString node_space;
      TGFString node_end;
      TGFString sep_node;
      TGFString sep_string;
      TGFString sep_keyval;
};


#endif // __GFCsvSettings_H

