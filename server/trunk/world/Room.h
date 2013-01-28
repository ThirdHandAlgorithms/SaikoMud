
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
   TGFVector npcs;
public:
   TGFProperty<long> x;
   TGFProperty<long> y;
   TGFStringProperty description;
   TGFProperty<BYTE> envtype;

   CRoom();
   ~CRoom();

   void loadNPCs();
};


#endif //__ROOM_H__
