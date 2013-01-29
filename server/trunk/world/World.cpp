
#include "World.h"

#include "Character.h"

CWorld::CWorld() : TGFFreeable() {
}

CWorld::~CWorld() {
}

CCombat *CWorld::getCombat( long x, long y ) {
   unsigned long vpos = (y + y_fix) * w + x + x_fix;

   CCombat *obj = static_cast<CCombat *>( combats.elementAt(vpos) );
   if ( obj != NULL ) {
      if ( !obj->isRunning() ) {
         this->endCombat(obj);

         obj = NULL;
      }
   }

   if ( obj == NULL ) {
      obj = new CCombat();
      combats.insertAt(vpos,obj);
   }

   return obj;
}

void CWorld::endCombat( CCombat *c ) {
   combats.removeElement(c);
   
   delete c;
}

void CWorld::reloadFromDatabase( TMySQLSquirrelConnection *pConn ) {
   rooms.clear();

   this->conn = pConn;

   TMySQLSquirrel qry( pConn );
   TGFBRecord rec;

   TGFString sql("select min(x), min(y), max(x), max(y), count(x) from grid where active=1");
   qry.setQuery(&sql);
   if ( qry.open() ) {
      qry.next();

      qry.fetchRecord(&rec);

      x_min = rec.getValue(0)->asInteger();
      y_min = rec.getValue(1)->asInteger();
      x_max = rec.getValue(2)->asInteger();
      y_max = rec.getValue(3)->asInteger();
      reccount = rec.getValue(4)->asInteger();

      qry.close();
   }

   h = y_max - y_min + 1;
   w = x_max - x_min + 1;

   x_fix = x_min * -1;
   y_fix = y_min * -1;

   rooms.resizeVector( w * h );
   rooms.setElementCount( w * h );

   combats.resizeVector( w * h );
   combats.setElementCount( w * h );

   sql.setValue_ansi("select * from grid where active=1 order by y asc, x asc");
   qry.setQuery(&sql);
   TSquirrelReturnData errData;
   if ( qry.open(&errData) ) {
      TGFBFields flds;
      qry.fetchFields(&flds);

      int xind = flds.getFieldIndex_ansi("x");
      int yind = flds.getFieldIndex_ansi("y");
      int descind = flds.getFieldIndex_ansi("desc");
      int envtypeind = flds.getFieldIndex_ansi("envtype");

      while ( qry.next() ) {
         qry.fetchRecord(&rec);

         CRoom *room = new CRoom();

         // this was previously a function within CRoom, inlining for speed
         room->x.internalSet( rec.getValue(xind)->asInteger() );
         room->y.internalSet( rec.getValue(yind)->asInteger() );

         room->envtype.internalSet( static_cast<BYTE>(rec.getValue(envtypeind)->asInteger()) );

         room->description.internalSetCopy( rec.getValue(descind)->asString() );
         // end

         long x = room->x.get();
         long y = room->y.get();

         rooms.insertAt( (y + y_fix) * w + x + x_fix, room );
      }

      qry.close();
   } else {
      printf("Error %d: %s\n", errData.errorcode, errData.errorstring.getValue());
   }

   this->reloadNpcs();
}

void CWorld::reloadNpcs() {
   npcs.clear();

   TMySQLSquirrel qry( this->conn );
   TGFBRecord rec;

   CCharacter *c;

   TSquirrelReturnData errData;

   TGFString sql("select * from `char` where account_id=0");
   qry.setQuery(&sql);
   if ( qry.open(&errData) ) {
      while ( qry.next() ) {
         qry.fetchRecord(&rec);

         c = new CCharacter(this->conn, &qry);
         this->npcs.addElement(c);
      }

      qry.close();
   } else {
      printf("Error %d: %s\n", errData.errorcode, errData.errorstring.getValue());
   }
}

bool CWorld::hasNPCsAt(long x, long y) {
   CCharacter *obj;
   unsigned long c = this->npcs.size();
   for (unsigned long i = 0; i < c; i++) {
      obj = static_cast<CCharacter *>( this->npcs.elementAt(i) );
      if (obj != NULL) {
         if ((obj->x.get() == x) && (obj->y.get() == y)) {
            return true;
         }
      }
   }

   return false;
}

void CWorld::preloadInteriors( long x, long y ) {
   CRoom *r;

}

CRoom *CWorld::getRoom( long x, long y ) {
   if (  (x < this->x_min) || (x > this->x_max) ||
         (y < this->y_min) || (y > this->y_max) ) {
      return NULL;
   }

   unsigned long vpos = (y + y_fix) * w + x + x_fix;

   return static_cast<CRoom *>( rooms.elementAt(vpos) );
}

void CWorld::echoAsciiMap( TGFString *s, long x, long y, unsigned int radius ) {
   long size_w = (radius << 1) + 1;  // +1 for center
   long size_h = (radius << 1) + 1;
   long start_x = x - radius;
   long start_y = y - radius;

   this->preloadInteriors(x,y);

   s->setSize(size_w*size_h + size_h*2);
   TGFString line;
   line.setSize(size_w + 2);

   // following is really slow, but the maths of this is way easier
   for ( long i = start_y; i < (start_y + size_h); i++ ) {
      line.setLength(0);
      for ( long j = start_x; j < (start_x + size_w); j++ ) {
         if ( (x == j) && (y == i) ) {
            CRoom *room = this->getRoom(j,i);
            if ( room != NULL ) {
               if (this->hasNPCsAt(i,j)) {
                  line.append(static_cast<char>(97 + room->envtype.get()));
               } else {
                  line.append_ansi("@");
               }
            } else {
               line.append_ansi("@");
            }
         } else {
            CRoom *room = this->getRoom(j,i);
            if ( room != NULL ) {
               if (this->hasNPCsAt(i,j)) {
                  line.append(static_cast<char>((65 + room->envtype.get())));
               } else {
                  line.append(static_cast<char>((48 + room->envtype.get())));
               }
            } else {
               line.append_ansi(" ");
            }
         }
      }

      line.append(13);
      line.append(10);
      s->prepend(&line);
   }
}
