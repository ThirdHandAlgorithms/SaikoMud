
#include "GFDefines.h"
#include "GFInitialize.h"

#include "GFMemFuncs.h"

#ifdef GF_OS_WIN32
   #include <locale.h>
#endif

bool initGroundfloor() {
   #ifdef GF_OS_WIN32
      // important, this is to synchronise codepage between this library and the application
      //  this is in case a windows control does not support Unicode (like RichEdit32.dll)
      char oldLocale[128] = "";
      sprintf( oldLocale, ".%d", GetACP() );
      setlocale( LC_ALL, oldLocale );
   #endif

   //return GFinitSmallMemory();
   // not using small memory maps anymore because of threading bugs
   return true;
}

bool finiGroundfloor() {
   //return GFfiniSmallMemory();
   return true;
}
