
#ifndef __ROOM_H__
#define __ROOM_H__

#include <MySQLBooks/MySQLSquirrel.h>

#include <Groundfloor/Atoms/GFFreeable.h>
#include <Groundfloor/Bookshelfs/GFBRecord.h>
#include <Groundfloor/Bookshelfs/GFBFields.h>
#include <Groundfloor/Molecules/GFString.h>
#include <Groundfloor/Molecules/GFProperty.h>

class CRoom: public TGFFreeable {
protected:
public:
   long x;
   long y;
   TGFString description;
   BYTE envtype;
   bool traversable;

   CRoom();
   ~CRoom();
};


#endif //__ROOM_H__
