

#include "TelnetConnection.h"
#include <Groundfloor/Materials/GFGarbageCollector.h>

#include <Groundfloor/Materials/GFFunctions.h>
#include "ChatChannel.h"
#include "Account.h"

#include "../Globals.h"

CTelnetConnection::CTelnetConnection( TJRBaseSocket *aSocket ) : TJRThreadedConnection(aSocket) {
   bBinaryMode = false;

   this->gameintf.nickname.set(aSocket->getRemoteAddress()->ip.getValue());

   TGFString welcome("Hi, type /2 text - for chat, /q for quiting, /n nickname - to change your tmpnick, /l user pass - to login\r\n");
   this->send( &welcome );

   enterGlobalChatChannel(this);

   this->setInterval(10);

   this->start();
}

CTelnetConnection::~CTelnetConnection() {
   leaveGlobalChatChannel(this);

   this->stopAndWait();
}

bool CTelnetConnection::inBinaryMode() {
   return this->bBinaryMode;
}

void CTelnetConnection::sendBin(uint32_t command, uint32_t intparam1, uint32_t intparam2, TGFString *s, uint32_t intparam3, uint32_t intparam4) {
   uint32_t strlen = 0;
   if (s != NULL) {
      strlen = s->getLength();
   }

   TGFString tmp;
   tmp.append((command & 0xff000000) >> 24);
   tmp.append((command & 0x00ff0000) >> 16);
   tmp.append((command & 0x0000ff00) >> 8);
   tmp.append((command & 0x000000ff));

   bool bIsStrCommand = ((command & 0x10000000) > 0);
   bool bIntParamCommand = ((command & 0x20000000) > 0);
   bool bExtIntParamCommand = ((command & 0x40000000) > 0);

   if (bIntParamCommand) {
      tmp.append((intparam1 & 0xff000000) >> 24);
      tmp.append((intparam1 & 0x00ff0000) >> 16);
      tmp.append((intparam1 & 0x0000ff00) >> 8);
      tmp.append((intparam1 & 0x000000ff));

      tmp.append((intparam2 & 0xff000000) >> 24);
      tmp.append((intparam2 & 0x00ff0000) >> 16);
      tmp.append((intparam2 & 0x0000ff00) >> 8);
      tmp.append((intparam2 & 0x000000ff));
   }

   if (bExtIntParamCommand) {
      tmp.append((intparam3 & 0xff000000) >> 24);
      tmp.append((intparam3 & 0x00ff0000) >> 16);
      tmp.append((intparam3 & 0x0000ff00) >> 8);
      tmp.append((intparam3 & 0x000000ff));

      tmp.append((intparam4 & 0xff000000) >> 24);
      tmp.append((intparam4 & 0x00ff0000) >> 16);
      tmp.append((intparam4 & 0x0000ff00) >> 8);
      tmp.append((intparam4 & 0x000000ff));
   }

   if (bIsStrCommand) {
      tmp.append((strlen & 0xff000000) >> 24);
      tmp.append((strlen & 0x00ff0000) >> 16);
      tmp.append((strlen & 0x0000ff00) >> 8);
      tmp.append((strlen & 0x000000ff));

      tmp.append(s,s->getLength());
   }

   this->send(&tmp);
}

bool CTelnetConnection::decodeNextBinMessageInBuffer(uint32_t *command, uint32_t *intparam1, uint32_t *intparam2, TGFString *s, uint32_t *intparam3, uint32_t *intparam4) {
   unsigned long iBufLen = buffer.getLength();

   if (iBufLen < 4) {
      return false;
   }

   unsigned char *arrBuffer = reinterpret_cast<unsigned char *>( buffer.getValue() );

   uint32_t iCommand = 0;

   iCommand |= static_cast<uint32_t>(arrBuffer[0]) << 24;
   iCommand |= static_cast<uint32_t>(arrBuffer[1]) << 16;
   iCommand |= static_cast<uint32_t>(arrBuffer[2]) << 8;
   iCommand |= static_cast<uint32_t>(arrBuffer[3]);

   bool bIsStrCommand = ((iCommand & 0x10000000) > 0);
   bool bIntParamCommand = ((iCommand & 0x20000000) > 0);
   bool bExtIntParamCommand = ((iCommand & 0x40000000) > 0);

   // void = 4
   // int = 12 (cmd 4 + int1 4 + int2 4)
   // str = 4+4 = 8
   // int+str = 12 + 4 = 16

   if (!(bIsStrCommand || bIntParamCommand || bExtIntParamCommand)) {
      *command = iCommand;

      buffer.remove(0, 3);

      return true;
   } else {
      int iMin = 0;

      if (bIsStrCommand) {
         iMin += 4;
      }
      if (bIntParamCommand) {
         iMin += 8;
      }
      if (bExtIntParamCommand) {
         iMin += 8;
      }

      if (iBufLen < iMin) {
         return false;
      }
   }

   unsigned long i = 4;
   if (bIntParamCommand) {
      uint32_t iIntParam1 = 0;
      uint32_t iIntParam2 = 0;

      iIntParam1 |= static_cast<uint32_t>(arrBuffer[i]) << 24;
      iIntParam1 |= static_cast<uint32_t>(arrBuffer[i+1]) << 16;
      iIntParam1 |= static_cast<uint32_t>(arrBuffer[i+2]) << 8;
      iIntParam1 |= static_cast<uint32_t>(arrBuffer[i+3]);

      *intparam1 = iIntParam1;

      i += 4;

      iIntParam2 |= static_cast<uint32_t>(arrBuffer[i]) << 24;
      iIntParam2 |= static_cast<uint32_t>(arrBuffer[i+1]) << 16;
      iIntParam2 |= static_cast<uint32_t>(arrBuffer[i+2]) << 8;
      iIntParam2 |= static_cast<uint32_t>(arrBuffer[i+3]);

      *intparam2 = iIntParam2;

      i += 4;
   }

   if (bExtIntParamCommand) {
      uint32_t iIntParam3 = 0;
      uint32_t iIntParam4 = 0;

      iIntParam3 |= static_cast<uint32_t>(arrBuffer[i]) << 24;
      iIntParam3 |= static_cast<uint32_t>(arrBuffer[i+1]) << 16;
      iIntParam3 |= static_cast<uint32_t>(arrBuffer[i+2]) << 8;
      iIntParam3 |= static_cast<uint32_t>(arrBuffer[i+3]);

      *intparam3 = iIntParam3;

      i += 4;

      iIntParam4 |= static_cast<uint32_t>(arrBuffer[i]) << 24;
      iIntParam4 |= static_cast<uint32_t>(arrBuffer[i+1]) << 16;
      iIntParam4 |= static_cast<uint32_t>(arrBuffer[i+2]) << 8;
      iIntParam4 |= static_cast<uint32_t>(arrBuffer[i+3]);

      *intparam4 = iIntParam4;

      i += 4;
   }

   uint32_t iStrLen = 0;
   if (bIsStrCommand) {

      iStrLen |= static_cast<uint32_t>(arrBuffer[i]) << 24;
      iStrLen |= static_cast<uint32_t>(arrBuffer[i+1]) << 16;
      iStrLen |= static_cast<uint32_t>(arrBuffer[i+2]) << 8;
      iStrLen |= static_cast<uint32_t>(arrBuffer[i+3]);

      i += 4;

      if ((i + iStrLen) < iBufLen) {
         return false;
      }

      s->setValue(buffer.getPointer(i), iStrLen);
   }

   buffer.remove(0, i + iStrLen - 1);
   
   *command = iCommand;

   return true;
}

void CTelnetConnection::inform_currentroom() {
   TGFString tmp;
   BYTE envtype = this->gameintf.GetRoomInfo(&tmp);

   if (!bBinaryMode) {
      if (tmp.getLength() > 0) {
         tmp.append_ansi("\r\n");
         this->send(&tmp);
      }
   } else {
      this->sendBin(c_response_roominfo, envtype, 0, &tmp);
   }
}

void CTelnetConnection::inform_lastaction() {
   TGFString tmp;

   uint32_t iWorldId = this->gameintf.GetLastActionInfo(&tmp);

   if (tmp.getLength() > 0) {
      if (!bBinaryMode) {
         tmp.append_ansi("\r\n");
         this->send(&tmp);
      } else {
         this->sendBin(c_response_lastactioninfo, iWorldId, 0, &tmp);
      }
   }
}

void CTelnetConnection::inform_map() {
   TGFString tmp;
   this->gameintf.GetTinyMap(&tmp);

   if (!bBinaryMode) {
      if (tmp.getLength() > 0) {
         tmp.append_ansi("\r\n");
         this->send(&tmp);
      }
   } else {
      this->sendBin(c_response_asciimap, 0, 0, &tmp);
   }
}

void CTelnetConnection::inform_questtitle(uint32_t iQuestId, TGFString *s) {
   TGFString tmp(s);

   if (!bBinaryMode) {
      if (tmp.getLength() > 0) {
         tmp.append_ansi("\r\n");
         this->send(&tmp);
      }
   } else {
      this->sendBin(c_response_questtitle, iQuestId, 0, &tmp);
   }
}

void CTelnetConnection::inform_questtext(uint32_t iQuestId, TGFString *s, long rewards_xp) {
   TGFString tmp(s);

   if (!bBinaryMode) {
      if (tmp.getLength() > 0) {
         tmp.append_ansi("\r\n");

         if (rewards_xp > 0) {
            tmp.append_ansi("Will reward ");
            TGFBValue v;
            v.setInteger(rewards_xp);
            tmp.append(v.asString());
            tmp.append_ansi(" XP on completion\r\n");
         }

         this->send(&tmp);
      }
   } else {
      this->sendBin(c_response_questtext, iQuestId, rewards_xp, &tmp);
   }
}

void CTelnetConnection::inform_npcinfo(uint32_t iWorldId, TGFString *s) {
   TGFString tmp(s);

   if (!bBinaryMode) {
      if (tmp.getLength() > 0) {
         tmp.append_ansi("\r\n");
         this->send(&tmp);
      }
   } else {
      this->sendBin(c_response_npcinfo, iWorldId, 0, &tmp);
   }
}

void CTelnetConnection::inform_npcdialog(uint32_t iWorldId, TGFString *s) {
   TGFString tmp(s);

   if (!bBinaryMode) {
      if (tmp.getLength() > 0) {
         tmp.append_ansi("\r\n");
         this->send(&tmp);
      }
   } else {
      this->sendBin(c_response_dialog, iWorldId, 0, &tmp);
   }
}

void CTelnetConnection::inform_earnxp(long xp, long totalxp) {
   TGFString tmp("");

   if (!bBinaryMode) {
      tmp.append_ansi("You earn ");
      TGFBValue v;
      v.setInteger(xp);
      tmp.append(v.asString());
      tmp.append_ansi(" XP\r\n");
      this->send(&tmp);
   } else {
      this->sendBin(c_event_earnsxp, xp, totalxp, NULL);
   }
}

void CTelnetConnection::inform_combatevent(uint32_t iSourceWorldId, uint32_t iTargetWorldId, int eventtype, int amount, TGFString *combatmsg) {
   TGFString tmp("");

   if (!bBinaryMode) {
      // note: already transmitted through combat chat channel
   } else {
      this->sendBin(c_event_combatmsg, iSourceWorldId, iTargetWorldId, combatmsg, eventtype, amount);
   }
}

void CTelnetConnection::informAboutAllStats(CCharacter *cAbout) {
   TGFString tmp("");

   CBaseCombatStats *stats = cAbout->getCurrentStats();

   if (!bBinaryMode) {
      tmp.setLength(1024);

      sprintf(tmp.getPointer(0), "Stats for %s: level %d, %d XP, %d HP\r\n\0\0\0\0",
         cAbout->name.get(),
         cAbout->level.get(),
         cAbout->xp.get(),
         cAbout->currenthealthpool.get()
      );

      tmp.setLength(strlen(tmp.getValue()));

      this->send(&tmp);
   } else {
      this->sendBin(c_event_statinfo_level, cAbout->WorldId, cAbout->level.get(), NULL);
      this->sendBin(c_event_statinfo_totalxp, cAbout->WorldId, cAbout->xp.get(), NULL);
      this->sendBin(c_event_statinfo_hp, cAbout->WorldId, cAbout->currenthealthpool.get(), NULL);
      
      if (stats != NULL) {
         this->sendBin(c_event_statinfo_strength, cAbout->WorldId, stats->strength.get(), NULL);
         this->sendBin(c_event_statinfo_energy, cAbout->WorldId, stats->energy.get(), NULL);
         this->sendBin(c_event_statinfo_protection, cAbout->WorldId, stats->protection.get(), NULL);
      }
   }
}

// todo: this function is way too long for a command-parser, need to buffer messages in queue and process 1 by 1
void CTelnetConnection::newMessageReceived( const TGFString *sMessage ) {
   bool bActionOk = false;

   buffer.append(sMessage);

   try {

      TGFString copy(sMessage);

      if (!bBinaryMode) {

         if ( copy.startsWith_ansi("/binarymode_on\r\n") ) {
            bBinaryMode = true;
            buffer.setLength(0);

            this->inform_lastaction();
            this->inform_map();
            this->inform_currentroom();

            return;
         } else if ( copy.startsWith_ansi("/2 ") ) {
            copy.rtrim_ansi();
            sendToGlobalChatChannel( this, copy.getPointer(3) );
         } else if ( copy.startsWith_ansi("/q") ) {
            this->gameintf.Logout();

            this->socket->disconnect();
            this->stop();
         } else if ( copy.startsWith_ansi("/n ") ) {
            copy.rtrim_ansi();
            this->gameintf.nickname.set( copy.getPointer(3) );
         } else if ( copy.startsWith_ansi("/l ") ) {
            copy.rtrim_ansi();
            TGFVector *v = GFsplit( &copy, " " );
            if ( v->size() == 3 ) {
               bActionOk = this->gameintf.Login( static_cast<TGFString *>(v->elementAt(1)), static_cast<TGFString *>(v->elementAt(2)) );
            }
            delete v;
         } else if ( copy.startsWith_ansi("/wr") ) {
            this->gameintf.ReloadWorld();
            bActionOk = false;   // no character room info
         } else if ( copy.startsWith_ansi("/combatdummy") ) {
            this->gameintf.StartCombatDummy();
            bActionOk = true;
         } else if ( copy.startsWith_ansi("/tinymap") ) {
            this->inform_map();
            bActionOk = true;
         } else if ( copy.startsWith_ansi("/lo") ) {
            this->gameintf.Logout();
            bActionOk = true;
         }

         if ( copy.startsWith_ansi("w") ) {
            bActionOk = this->gameintf.run_walkforward();
         } else if ( copy.startsWith_ansi("a") ) {
            bActionOk = this->gameintf.run_walkleft();
         } else if ( copy.startsWith_ansi("d") ) {
            bActionOk = this->gameintf.run_walkright();
         } else if ( copy.startsWith_ansi("s") ) {
            bActionOk = this->gameintf.run_walkbackwards();
         }

         if (bActionOk) {
            this->inform_lastaction();
            this->inform_currentroom();
         } else {
            this->inform_lastaction();
         }

      } else {

         uint32_t command;
         uint32_t intparam1, intparam2, intparam3, intparam4;
         TGFString s;

         printf("start decodeNextBinMessageInBuffer\n");
         while ( decodeNextBinMessageInBuffer(&command, &intparam1, &intparam2, &s, &intparam3, &intparam4) )  {
            printf("decodeNextBinMessageInBuffer -> %08x\n", command);

            bool bMovementActionOk = false;

            if (command == c_run_walkbackwards) {
               bActionOk = this->gameintf.run_walkbackwards();
               bMovementActionOk = bActionOk;
            } else if (command == c_run_walkforward) {
               bActionOk = this->gameintf.run_walkforward();
               bMovementActionOk = bActionOk;
            } else if (command == c_run_walkleft) {
               bActionOk = this->gameintf.run_walkleft();
               bMovementActionOk = bActionOk;
            } else if (command == c_run_walkright) {
               bActionOk = this->gameintf.run_walkright();
               bMovementActionOk = bActionOk;
            } else if (command == c_attack_start) {
               bActionOk = this->gameintf.attack_start(intparam1);
               bMovementActionOk = bActionOk;
            } else if (command == c_attack_stop) {
               //this->gameintf.attack_stop();
            } else if (command == c_chat_say) {
               if (intparam1 == 2) {
                  sendToGlobalChatChannel( this, &s );
               }
            } else if (command == c_radar_getnearbynpcs) {
               TGFVector v;
               v.autoClear = false;
               int c = this->gameintf.radar_getNearbyNpcs(&v);
               if (c > 0) {
                  for (int i = 0; i < c; i++) {
                     CCharacter *c = static_cast<CCharacter *>(v.elementAt(i));

                     this->inform_npcinfo( c->WorldId, c->name.link() );
                  }
               }
            } else if (command == c_radar_getnearbyplayers) {
               // ...
               bActionOk = false;
            } else if (command == c_interact_greet) {
               TGFString s;
               bActionOk = this->gameintf.interact_greet(intparam1, &s);
               if (bActionOk) {
                  this->inform_npcdialog(intparam1, &s);
               }
            } else if (command == c_interact_getquesttitles) {
               TGFVector v;
               v.autoClear = false;

               int c = this->gameintf.interact_getQuests(intparam1, &v);

               if ( c != -1 ) {
                  for (int i = 0; i < c; i++) {
                     CQuest *q = static_cast<CQuest *>(v.elementAt(i));
                     this->inform_questtitle(q->id, &(q->title));
                  }
               }
               
               bActionOk = (c != -1);
            } else if (command == c_interact_getquesttext) {
               TGFString s;

               long rewards_xp = 0;
               bActionOk = this->gameintf.interact_getQuestText(intparam1, &s, &rewards_xp);
               
               if (bActionOk) {
                  this->inform_questtext(intparam1, &s, rewards_xp);
               }
            } else if (command == c_self_getallstats) {
               bActionOk = this->gameintf.inform_SelfAboutAllStats();
            }

            this->inform_lastaction();

            if (bActionOk) {
               // todo: ... what to do?
            }

            if (bMovementActionOk) {
               this->inform_currentroom();
               this->inform_map();
            }
         }
      }
   } catch (...) {
      this->inform_lastaction();
   }
}

void CTelnetConnection::send( const TGFString *s ) {
   TGFCommReturnData errData;
   this->socket->send( s, &errData );
}

bool CTelnetConnection::matchWithCharacterRef( void *ref ) {
   return this->gameintf.MatchWithCharacterRef(ref);
}

void CTelnetConnection::append_nickname(TGFString *s) {
   s->append_ansi( gameintf.nickname.get() );
}
