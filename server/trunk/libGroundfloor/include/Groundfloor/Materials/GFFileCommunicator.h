
#ifndef __GFFileCommunicator_H
#define __GFFileCommunicator_H

class TGFFileCommunicator;

#include "../Molecules/GFBaseCommunicator.h"
#include "../Atoms/GFLockable.h"
#include "../Molecules/GFProperty.h"

#include <cstdio>

#define GFFILEMODE_READ    1
#define GFFILEMODE_WRITE   2
#define GFFILEMODE_APPEND  4

/** Communicator implementation for Files
 */
class TGFFileCommunicator: public TGFBaseCommunicator {
   protected:
      FILE *fp;
      TGFLockable lockReadWrite;

   public:
      TGFFileCommunicator();
      ~TGFFileCommunicator();

      /// filename is assumed to be UTF-8
      TGFStringProperty filename;
      TGFProperty<int> mode;

      /// opens file
      bool connect();
      /// closes file
      bool disconnect();

      /// writes data to file
      bool send( const TGFString *sData, TGFCommReturnData *errData = NULL );
      /// reads data from file (sData->getSize() is used as bufferlength)
      bool receive( TGFString *sData, TGFCommReturnData *errData = NULL  );
};

#endif // __GFFileCommunicator_H
