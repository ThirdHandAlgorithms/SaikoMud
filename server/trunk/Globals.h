
#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <MySQLBooks/MySQLSquirrel.h>

#include "world/World.h"
#include "world/CharacterUpdate.h"
#include "world/RespawnThread.h"
#include "general/TelnetServe.h"

CWorld *Global_World();
CCharacterUpdate *Global_CharacterUpdate();
TMySQLSquirrelConnection *Global_DBConnection();
CTelnetServe *Global_Server();
CRespawnThread *Global_RespawnThread();

bool initGameGlobals();
void finiGameGlobals();

#endif // __GLOBALS_H__
