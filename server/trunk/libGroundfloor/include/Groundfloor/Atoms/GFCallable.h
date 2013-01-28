
#ifndef __GFCALLABLE_H
#define __GFCALLABLE_H

#include "GFFreeable.h"

/** TGFCallable defines an abstruct function execute() that
  * other objects may call upon.
  */
class TGFCallable: public TGFFreeable
{
    public:
         /// abstract function to implement
        virtual void execute() = 0;
};


#endif // __GFCALLABLE_H
