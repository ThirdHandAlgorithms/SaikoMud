
#ifndef __WORLD_H__
#define __WORLD_H__

#include <Groundfloor/Molecules/GFVector.h>
#include <Groundfloor/Atoms/GFLockable.h>
#include <MySQLBooks/MySQLSquirrel.h>

#include "Room.h"

#include "../combat/Combat.h"
#include "Character.h"
#include "Quest.h"
#include "Items.h"

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

   TMySQLSquirrelConnection *conn;

   TGFLockable worldidlock;

   TGFVector rooms;
   TGFVector combats;

   TGFVector characters;
   TGFVector npcs;
   TGFVector itemcache;

   // pointerlinks to characters and npcs with (hopefully) non-retraceable id's to the database id's - no-auto-free
   TGFVector worldids;

   // indexed questlists - auto-free
   TGFVector quests;

   // todo: items to load on demand
   // TGFVector items;

   bool hasNPCsAt(long x, long y);

   void loadNeededQuests(CNPCharacter *cNpc);

   long completeQuest(CQuest *q, CCharacter *cFor);
public:
   CWorld();
   ~CWorld();

   void reloadFromDatabase( TMySQLSquirrelConnection *pConn );
   void reloadNpcs();

   CRoom *getRoom( long x, long y );
   CCombat *getCombat( long x, long y );
   void endCombat( CCombat *c );

   void preloadInteriors( long x, long y );

   int getNpcsInRoom_fromdb(long x, long y, TGFVector *v);

   CNPCharacter *getNpcByName(TGFString *s);

   CCharacter *getCharacter(uint32_t id);
   CQuest *getQuest(uint32_t id);

   CItem *getItem(uint32_t id);

   bool getQuestStory(long iQuestId, CCharacter *cFor, TGFString *sStory, long *rewards_xp);
   bool getGreeting(long iCharId, CCharacter *cFor, TGFString *sGreeting);

   void onRespawnTimerCharacter(TGFFreeable *c);
   void handleDeath(CCharacter *cFor, CCharacter *cKilledBy);

   void echoAsciiMap( TGFString *s, long x, long y, unsigned int radius );

   uint32_t generateUniqueWorldId(CCharacter *c);
   void unloadCharacter(CCharacter *c);

   void informAboutAllStats(CCharacter *cFor, CCharacter *cAbout);

   void printf_world_stats(bool preloadThings);
};

#endif //__WORLD_H__
