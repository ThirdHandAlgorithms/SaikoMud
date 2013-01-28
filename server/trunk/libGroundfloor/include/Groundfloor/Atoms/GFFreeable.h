
#ifndef __GFFreeable_H_
#define __GFFreeable_H_
/***************************************************************************/

#include <cstdio>

/** TGFFreeable adds a virtual destructor, so that when
 *  an inherited class is created, it can still be destroyed by calling
 *  delete (TGFFreeable *)object;
 */
class TGFFreeable {
private:
   TGFFreeable& operator=(const TGFFreeable* s); // not allowed to make deep copies
public:
   TGFFreeable() {};
   /** a default virtual destructor with no implementation */
   virtual ~TGFFreeable() {};
};


/***************************************************************************/
#endif // __GFFreeable_H_

