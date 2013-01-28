
#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <MySQLBooks/MySQLSquirrel.h>

#include "world/World.h"
#include "world/CharacterUpdate.h"
#include "general/TelnetServe.h"

CWorld *Global_World();
CCharacterUpdate *Global_CharacterUpdate();
TMySQLSquirrelConnection *Global_DBConnection();
CTelnetServe *Global_Server();

bool initGameGlobals();
void finiGameGlobals();

#endif // __GLOBALS_H__
