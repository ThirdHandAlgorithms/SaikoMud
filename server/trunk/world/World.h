
#ifndef __WORLD_H__
#define __WORLD_H__

#include <MySQLBooks/MySQLSquirrel.h>
#include "Room.h"

#include "../combat/Combat.h"

class CWorld: TGFFreeable {
protected:
   // note: model like this might result in vectors that are way too big, restructure when width*height exceeds MAX_INT/2
   long x_min;
   long y_min;
   long x_max;
   long y_max;
   long x_fix;
   long y_fix;
   long w;
   long h;
   long reccount;

   TGFVector rooms;
   TGFVector combats;

   TGFVector characters;
public:
   CWorld();
   ~CWorld();

   void reloadFromDatabase( TMySQLSquirrelConnection *pConn );

   CRoom *getRoom( long x, long y );
   CCombat *getCombat( long x, long y );
   void endCombat( CCombat *c );

   void preloadInteriors( long x, long y );

   void echoAsciiMap( TGFString *s, long x, long y, unsigned int radius );
};

#endif //__WORLD_H__
