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


                        CGameInterface intf;
                        intf.Login(GFDisposableStr("servertest"), GFDisposableStr("muddy"));
                        
                        Global_World()->printf_world_stats(true);

                        TGFString s;
                        intf.interact_getQuestText(1, &s);
                        printf("%s\n", s.getValue());

                        auto c = Global_World()->getNpcByName(GFDisposableStr("npc_mister_b"));
                        if (c!=NULL){
                           TGFVector v;
                           auto i = intf.interact_getQuests(c->WorldId, &v);
                           for (int j = 0; j < i; j++) {
                              auto q = static_cast<CQuest *>(v.elementAt(j));
                              printf("%d: %s\n", q->id, q->title.getValue());
                           }
                        }
                        
                        while ( server->isRunning() ) {
                           server->cleanup();

                           //printf("\nStats\n=====\n");
                           //Global_World()->printf_world_stats(true);

                           GFMillisleep(1500);
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

