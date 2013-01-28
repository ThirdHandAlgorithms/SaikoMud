// SaikoMUD.cpp : Defines the entry point for the console application.
//

#include <cstdio>
#include <cstdlib>

#include <Groundfloor/Atoms/GFInitialize.h>
#include <Groundfloor/Materials/GFGarbageCollector.h>
#include <Jumpropes/JRInitialize.h>
#include <MySQLBooks/MySQLSquirrel.h>

#include "Globals.h"

#include "general/TelnetServe.h"
#include "general/ChatChannel.h"
#include "world/World.h"

int main(int argc, char* argv[]) {
   if ( initGroundfloor() ) {
      if ( initGlobalGarbageCollector() ) {
         if ( initJumpropes() ) {
            if ( initMySQLBooks() ) {
               if ( initGameGlobals() ) {
                  Global_DBConnection()->host.set("www.saikosoft.net");
                  Global_DBConnection()->username.set("saikomud");
                  Global_DBConnection()->password.set("dL5HRSbL5RSZFXfR");

                  if ( Global_DBConnection()->connect() ) {
                     Global_DBConnection()->selectDatabase("saikomud");

                     Global_World()->reloadFromDatabase( Global_DBConnection() );

                     if ( initGlobalChatChannel() ) {
                        CTelnetServe *server = Global_Server();
                        while ( server->isRunning() ) {
                           server->cleanup();

                           GFMillisleep(500);
                        }

                        finiGlobalChatChannel();
                     }
                     Global_DBConnection()->disconnect();
                  }
                  finiGameGlobals();
               }
               finiMySQLBooks();
            }
            finiJumpropes();
         }
         finiGlobalGarbageCollector();
      }
      finiGroundfloor();
   }

	return 0;
}

