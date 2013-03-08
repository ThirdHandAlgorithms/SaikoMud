
#include "Globals.h"

CWorld *world = NULL;
TMySQLSquirrelConnection *connection = NULL;
CCharacterUpdate *ccupdate = NULL;
CTelnetServe *server = NULL;
CRespawnThread *respawnthread = NULL;


CWorld *Global_World() {
   return world;
}

TMySQLSquirrelConnection *Global_DBConnection() {
   return connection;
}

CCharacterUpdate *Global_CharacterUpdate() {
   return ccupdate;
}

CTelnetServe *Global_Server() {
   return server;
}

CRespawnThread *Global_RespawnThread() {
   return respawnthread;
}

bool initGameGlobals() {
   connection = new TMySQLSquirrelConnection();
   world = new CWorld();
   ccupdate = new CCharacterUpdate();
   server = new CTelnetServe();
   respawnthread = new CRespawnThread();

   respawnthread->start();

   return true;
}

void finiGameGlobals() {
   delete respawnthread;
   server = NULL;

   delete server;
   server = NULL;

   delete ccupdate;
   ccupdate = NULL;

   delete world;
   world = NULL;

   delete connection;
   connection = NULL;
}
