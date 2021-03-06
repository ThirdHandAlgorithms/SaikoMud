

#include "TelnetConnection.h"
#include <Groundfloor/Materials/GFGarbageCollector.h>

#include <Groundfloor/Materials/GFFunctions.h>
#include "ChatChannel.h"
#include "Account.h"

#include "../Globals.h"

CTelnetConnection::CTelnetConnection( TJRBaseSocket *aSocket ) : TJRThreadedConnection(aSocket) {
   bBinaryMode = false;

   iLastTimeSentMap = 0;

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

void CTelnetConnection::sendBin2(uint32_t command, std::vector<uint32_t> *intarray, TGFStringVector *strarray) {
   TGFString tmp;
   tmp.append((command & 0xff000000) >> 24);
   tmp.append((command & 0x00ff0000) >> 16);
   tmp.append((command & 0x0000ff00) >> 8);
   tmp.append((command & 0x000000ff));

   uint32_t intcount = intarray->size();

   tmp.append((intcount & 0xff000000) >> 24);
   tmp.append((intcount & 0x00ff0000) >> 16);
   tmp.append((intcount & 0x0000ff00) >> 8);
   tmp.append((intcount & 0x000000ff));

   uint32_t strcount = strarray->size();

   tmp.append((strcount & 0xff000000) >> 24);
   tmp.append((strcount & 0x00ff0000) >> 16);
   tmp.append((strcount & 0x0000ff00) >> 8);
   tmp.append((strcount & 0x000000ff));

   for(std::vector<uint32_t>::iterator it = intarray->begin(); it != intarray->end(); ++it) {
      uint32_t a = *it;

      tmp.append((a & 0xff000000) >> 24);
      tmp.append((a & 0x00ff0000) >> 16);
      tmp.append((a & 0x0000ff00) >> 8);
      tmp.append((a & 0x000000ff));
   }

   for (unsigned int i = 0; i < strcount; i++) {
      TGFString *s = strarray->getChunk(i);

      uint32_t len = s->getLength();
      tmp.append((len & 0xff000000) >> 24);
      tmp.append((len & 0x00ff0000) >> 16);
      tmp.append((len & 0x0000ff00) >> 8);
      tmp.append((len & 0x000000ff));

      tmp.append(s);
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

   bool bIntStrArrays = ((iCommand & 0x80000000) > 0);

   std::vector<uint32_t> intarray;
   TGFStringVector strarray;

   // void = 4
   // int = 12 (cmd 4 + int1 4 + int2 4)
   // str = 4+4 = 8
   // int+str = 12 + 4 = 16
   if (bIntStrArrays) {
      unsigned long i = 4;

      if (iBufLen < 12) {
         return false;
      }

      uint32_t intcount = 0;
      intcount |= (uint32_t)(arrBuffer[i] << 24);
      intcount |= (uint32_t)(arrBuffer[i + 1] << 16);
      intcount |= (uint32_t)(arrBuffer[i + 2] << 8);
      intcount |= (uint32_t)(arrBuffer[i + 3]);
      i += 4;

      uint32_t strcount = 0;
      strcount |= (uint32_t)(arrBuffer[i] << 24);
      strcount |= (uint32_t)(arrBuffer[i + 1] << 16);
      strcount |= (uint32_t)(arrBuffer[i + 2] << 8);
      strcount |= (uint32_t)(arrBuffer[i + 3]);
      i += 4;

      if (iBufLen < i + intcount * 4 + strcount * 4) {
         return false;
      }

      uint32_t y;
      for (int x = 0; x < intcount; x++) {
         y = 0;
         y |= (uint32_t)(arrBuffer[i] << 24);
         y |= (uint32_t)(arrBuffer[i + 1] << 16);
         y |= (uint32_t)(arrBuffer[i + 2] << 8);
         y |= (uint32_t)(arrBuffer[i + 3]);

         i += 4;

         intarray.push_back(y);
      }

      for (int x = 0; x < strcount; x++) {
         y = 0;
         y |= (uint32_t)(arrBuffer[i] << 24);
         y |= (uint32_t)(arrBuffer[i + 1] << 16);
         y |= (uint32_t)(arrBuffer[i + 2] << 8);
         y |= (uint32_t)(arrBuffer[i + 3]);

         i += 4;

         strarray.addChunk(new TGFString(buffer.getPointer(i), y));

         i += (int)y;
      }

      buffer.remove(0, i - 1);

      *command = iCommand;


   } else {
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

         if ((i + iStrLen) > iBufLen) {
            return false;
         }

         if (iStrLen > 0) {
            s->setValue(buffer.getPointer(i), iStrLen);
         }
      }

      buffer.remove(0, i + iStrLen - 1);
   
      *command = iCommand;
   }

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

void CTelnetConnection::inform_map(uint32_t extrainfo, uint32_t extrasize) {
   TGFString tmp;
   uint32_t x, y;
   this->gameintf.GetTinyMap(&tmp, &x, &y, extrasize);

   if (!bBinaryMode) {
      if (tmp.getLength() > 0) {
         tmp.append_ansi("\r\n");
         this->send(&tmp);
      }
   } else {
      this->sendBin(c_response_asciimap, x, y, &tmp, extrainfo, extrasize);
   }
}

bool CTelnetConnection::inform_iteminfo(uint32_t iItemId) {
   CItem *item = Global_World()->getItem(iItemId);
   if (item != NULL) {
      TGFString tmp(&item->name);
      tmp.append_ansi("|");
      tmp.append(&(item->description));

      if (!bBinaryMode) {
         if (tmp.getLength() > 0) {
            tmp.append_ansi("\r\n");
            this->send(&tmp);
         }
      } else {
         this->sendBin(c_response_iteminfo, item->id, 0, &tmp, item->type, item->charslot_id);
      }

      if (item->equipable) {
         // if item is equipable, then it's very likely the client wants to know the stats that are on the item
         this->inform_itemstats(iItemId);
      }
   } else {
      // todo: item doesn't exist, give the client a hard time about things he shouldn't be doing...
      return false;
   }

   return true;
}

bool CTelnetConnection::inform_itemstats(uint32_t iItemId) {
   CBaseCombatStats *stats = Global_World()->getItemStats(iItemId);
   if (stats != NULL) {
      TGFString tmp("");

      if (!bBinaryMode) {
         if (tmp.getLength() > 0) {
            tmp.append_ansi("\r\n");
            this->send(&tmp);
         }
      } else {
         this->sendBin(c_response_itemstats, iItemId, stats->strength.get(), &tmp, stats->energy.get(), stats->protection.get());
      }
   } else {
      // todo: item/stats don't exist, give the client a hard time about things he shouldn't be doing...
      return false;
   }

   return true;
}

bool CTelnetConnection::inform_spellinfo(uint32_t iSpellId) {
   CSpell *spell = Global_World()->getSpell(iSpellId);
   if (spell != NULL) {
      TGFString tmp(&(spell->name));

      if (!bBinaryMode) {
         if (tmp.getLength() > 0) {
            tmp.append_ansi("\r\n");
            this->send(&tmp);
         }
      } else {
         this->sendBin(c_response_spellinfo, iSpellId, spell->basedamage, &tmp, spell->casttime, spell->cooldown);
      }
   } else {
      return false;
   }

   return true;
}

bool CTelnetConnection::inform_playerspells(uint32_t iWorldId) {
   std::vector<uint32_t> intarr;
   TGFStringVector strarr;

   CCharacter *c = NULL;
   if (iWorldId == 0) {
      c = Global_World()->getCharacter( this->gameintf.getPlayerWorldId() );
   } else {
      c = Global_World()->getCharacter(iWorldId);
   }

   if (c != NULL) {
      intarr.push_back(c->WorldId);
      strarr.addChunk(new TGFString(c->name.link()));

      std::vector<unsigned long> spells = c->getSpells();

      TGFVector v;
      v.autoClear = true;
      bool b = true;
      for (std::vector<unsigned long>::iterator it = spells.begin(); it != spells.end(); ++it) {
         if (b) {
            // skip first 
            b = false;
            continue;
         }

         if (*it == 0) {
            intarr.push_back(0);
            strarr.addChunk( new TGFString("") );
         } else {
            CSpell *spell = Global_World()->getSpell(*it);
            if (spell != NULL) {
               intarr.push_back(spell->getId());
               strarr.addChunk( new TGFString(&spell->name) );
            } else {
               intarr.push_back(0);
               strarr.addChunk( new TGFString("") );
            }
         }
      }

      this->sendBin2(c_response_spells, &intarr, &strarr);
   }

   return true;
}

bool CTelnetConnection::inform_gearslots(uint32_t iWorldId) {
   std::vector<uint32_t> intarr;
   TGFStringVector strarr;

   CCharacter *c = NULL;
   if (iWorldId == 0) {
      c = Global_World()->getCharacter( this->gameintf.getPlayerWorldId() );
   } else {
      c = Global_World()->getCharacter(iWorldId);
   }

   if (c != NULL) {
      intarr.push_back(c->WorldId);
      strarr.addChunk(new TGFString(c->name.link()));

      TGFVector v;
      v.autoClear = true;
      int x = c->getItemsInSlots(&v);
      for (int i = 0; i < 5; i++) {
         CItem *item = static_cast<CItem *>(v.elementAt(i));
         if (item != NULL) {
            intarr.push_back(item->id);
            strarr.addChunk( new TGFString(&item->name) );
         } else {
            intarr.push_back(0);
            strarr.addChunk( new TGFString("") );
         }
      }

      this->sendBin2(c_response_gearslots, &intarr, &strarr);
   }

   return true;
}

bool CTelnetConnection::inform_self_bagslots() {
   std::vector<uint32_t> intarr;
   TGFStringVector strarr;

   TGFString tmp;

   TGFVector v;
   int c = this->gameintf.getOwnBagSlots(&v);
   if (c != -1) {
      for (int i = 0; i < c; i++) {
         CItem *item = static_cast<CItem *>(v.elementAt(i));
         if (item != NULL) {
            intarr.push_back(item->id);
            strarr.addChunk( new TGFString(&item->name) );

            tmp.append(&item->name);
            tmp.append_ansi("\r\n");
         } else {
            intarr.push_back(0);
            strarr.addChunk( new TGFString("") );
         }
      }

      if (!bBinaryMode) {
         if (tmp.getLength() > 0) {
            tmp.append_ansi("\r\n");
            this->send(&tmp);
         }
      } else {
         this->sendBin2(c_response_bagslots, &intarr, &strarr);
      }

      return true;
   }


   return false;
}

void CTelnetConnection::inform_questtitle(uint32_t iQuestId, TGFString *s, bool bCanComplete) {
   TGFString tmp(s);

   if (!bBinaryMode) {
      if (tmp.getLength() > 0) {
         tmp.append_ansi("\r\n");
         this->send(&tmp);
      }
   } else {
      uint32_t args = 0;
      if (bCanComplete) {
         args |= 0x01;
      }
      this->sendBin(c_response_questtitle, iQuestId, args, &tmp);
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

void CTelnetConnection::inform_questitemrequired(uint32_t iQuestId, uint32_t item_id, uint32_t iNumberRequired, TGFString *sItemName) {
   TGFString tmp(sItemName);
   TGFBValue v;
   v.setInteger(iNumberRequired);
   tmp.append_ansi(" ");
   tmp.append(v.asString());
   tmp.append_ansi("x");

   if (!bBinaryMode) {
      if (tmp.getLength() > 0) {
         tmp.append_ansi("\r\n");

         this->send(&tmp);
      }
   } else {
      this->sendBin(c_response_questitemrequired, iQuestId, item_id, &tmp, iNumberRequired);
   }
}

void CTelnetConnection::inform_npcinfo(uint32_t iWorldId, TGFString *s, uint32_t x, uint32_t y) {
   TGFString tmp(s);

   if (!bBinaryMode) {
      if (tmp.getLength() > 0) {
         tmp.append_ansi("\r\n");
         this->send(&tmp);
      }
   } else {
      this->sendBin(c_response_npcinfo, iWorldId, 0, &tmp, x, y);
   }
}

void CTelnetConnection::inform_playerinfo(CCharacter *c) {
   TGFString tmp(c->name.link());

   if (!bBinaryMode) {
      if (tmp.getLength() > 0) {
         tmp.append_ansi("\r\n");
         this->send(&tmp);
      }
   } else {
      this->sendBin(c_response_playerinfo, c->WorldId, 0, &tmp, c->x.get(), c->y.get());
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

void CTelnetConnection::sendChatMessage(uint32_t iChannelNr, TGFString *sMsg) {
   if (!bBinaryMode) {
      this->send(sMsg);
   } else {
      this->sendBin(c_response_chatmessage, iChannelNr, 0, sMsg);
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
            this->inform_map(0);
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
            this->inform_map(0);
            bActionOk = true;
         } else if ( copy.startsWith_ansi("/lo") ) {
            this->gameintf.Logout();
            bActionOk = true;
         } else if ( copy.startsWith_ansi("/ps") ) {
            if (this->gameintf.IsAdmin() ) {
               Global_World()->printf_world_stats(true);
            }
         } else if ( copy.startsWith_ansi("/tp ") ) {
            if (this->gameintf.IsAdmin()) {
               TGFVector *v = GFsplit( &copy, " " );
               if ( v->size() == 4 ) {
                  TGFBValue pv;
                  pv.setString(static_cast<TGFString *>(v->elementAt(1)));
                  long iWorldId = pv.asInteger();
                  pv.setString(static_cast<TGFString *>(v->elementAt(2)));
                  long x = pv.asInteger();
                  pv.setString(static_cast<TGFString *>(v->elementAt(3)));
                  long y = pv.asInteger();

                  bActionOk = this->gameintf.admin_teleport_player( iWorldId, x, y );
               } else {
                  bActionOk = false;
               }
            } else {
               bActionOk = false;
            }
         } else if ( copy.startsWith_ansi("/bags") ) {
            bActionOk = this->inform_self_bagslots();
         } else if ( copy.startsWith_ansi("/spells") ) {
            bActionOk = this->inform_playerspells(0);
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

         //printf("start decodeNextBinMessageInBuffer\n");
         while ( decodeNextBinMessageInBuffer(&command, &intparam1, &intparam2, &s, &intparam3, &intparam4) )  {
           // printf("decodeNextBinMessageInBuffer -> %08x\n", command);

            bool bMovementActionOk = false;
            bool bNoRoomInfo = false;
            bool bExtraMapInfo = false;

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

            } else if (command == c_check_walkforward) {
               bActionOk = this->gameintf.check_walkforward();
               bExtraMapInfo = true;
            } else if (command == c_check_walkbackwards) {
               bActionOk = this->gameintf.check_walkbackwards();
               bExtraMapInfo = true;
            } else if (command == c_check_walkright) {
               bActionOk = this->gameintf.check_walkright();
               bExtraMapInfo = true;
            } else if (command == c_check_walkleft) {
               bActionOk = this->gameintf.check_walkleft();
               bExtraMapInfo = true;

            } else if (command == c_attack_start) {
               bActionOk = this->gameintf.attack_start(intparam1);
               bMovementActionOk = bActionOk;
            } else if (command == c_attack_stop) {
               //this->gameintf.attack_stop();
            } else if (command == c_cast_spell) {
               bActionOk = this->gameintf.cast_spell(intparam1, intparam2);
               bMovementActionOk = bActionOk;
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

                     int sx = c->x.get();
                     int sy = c->y.get();

                     this->inform_npcinfo( c->WorldId, c->name.link(), (uint32_t)sx, (uint32_t)sy );
                  }
               }
            } else if (command == c_radar_getnearbyplayers) {
               // ...
               TGFVector v;
               v.autoClear = false;
               int c = this->gameintf.radar_getNearbyPlayers(&v);
               if (c > 0) {
                  for (int i = 0; i < c; i++) {
                     CCharacter *c = static_cast<CCharacter *>(v.elementAt(i));

                     this->inform_playerinfo(c);
                  }
               }
            } else if (command == c_radar_getmap) {
               bActionOk = true;
               bNoRoomInfo = true;
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
                     bool b = this->gameintf.canCompleteQuest(q);
                     this->inform_questtitle(q->id, &(q->title), q->autocomplete || b);
                  }
               }
               
               bActionOk = (c != -1);
            } else if (command == c_interact_getquesttext) {
               TGFString s;

               long rewards_xp = 0;
               bActionOk = this->gameintf.interact_getQuestText(intparam1, &s, &rewards_xp);
               
               if (bActionOk) {
                  this->inform_questtext(intparam1, &s, rewards_xp);

                  // send item requirement information
                  CQuest *q = Global_World()->getQuest(intparam1);
                  if (q != NULL) {
                     // i hate std:vectors... why am I doing it this way.......
                     std::vector<CQuestItemRequired> items = q->getRequiredItems();
                     CQuestItemRequired *it;
                     for (std::vector<CQuestItemRequired>::iterator it = items.begin(); it != items.end(); ++it) {
                        CItem *item = Global_World()->getItem(it->item_id);
                        if (item != NULL) {
                           this->inform_questitemrequired(q->id, it->item_id, it->amountrequired, &(item->name));
                        }
                     }

                     bool b = this->gameintf.canCompleteQuest(q);
                     if (b) {
                        if (this->gameintf.completeQuest(q) > 0) {
                           // huray!
                        }
                     }
                  }
               }
            } else if (command == c_self_getallstats) {
               bActionOk = this->gameintf.inform_SelfAboutAllStats();
            } else if (command == c_info_getiteminfo) {
               bActionOk = this->inform_iteminfo(intparam1);
            } else if (command == c_info_getspellinfo) {
               bActionOk = this->inform_spellinfo(intparam1);
            } else if (command == c_self_getspells) {
               bActionOk = this->inform_playerspells(0);
            } else if (command == c_info_getitemstats) {
               bActionOk = this->inform_itemstats(intparam1);
            } else if (command == c_info_getgearslots) {
               bActionOk = this->inform_gearslots(intparam1);
            } else if (command == c_self_getbagslots) {
               bActionOk = this->inform_self_bagslots();
            } else if (command == c_interact_getquestitemsrequired) {
               CQuest *q = Global_World()->getQuest(intparam1);
               if (q != NULL) {
                  std::vector<CQuestItemRequired> items = q->getRequiredItems();
                  CQuestItemRequired *it;
                  for (std::vector<CQuestItemRequired>::iterator it = items.begin(); it != items.end(); ++it) {
                     CItem *item = Global_World()->getItem(it->item_id);
                     if (item != NULL) {
                        this->inform_questitemrequired(q->id, it->item_id, it->amountrequired, &(item->name));
                     }
                  }
               }
            } else if (command == c_info_equipitem) {
               bActionOk = this->gameintf.equip_itemfrombags(intparam1);
               if (bActionOk) {
                  // does this work... ?
                  this->inform_gearslots(0);
                  this->inform_self_bagslots();
                  this->gameintf.inform_SelfAboutAllStats();
               }
            } else if (command == c_info_dequipitem) {
               bActionOk = this->gameintf.dequip_item(intparam1);
            }

            if (bExtraMapInfo) {
               if (bActionOk) {
                  this->inform_map(command,1);
               } else {
                  this->inform_map(command,0);
               }
            } else {
               this->inform_lastaction();

               if (bActionOk || bMovementActionOk) {
                  if (bMovementActionOk) {
                     this->inform_map(command, 0);
                  } else if (iLastTimeSentMap + 1 <= GFGetTimestamp()) {
                     this->inform_map(0);

                     iLastTimeSentMap = GFGetTimestamp();
                  }
               }

               if (bMovementActionOk) {
                  if (!bNoRoomInfo) {
                     this->inform_currentroom();
                  }

                  TGFVector v;
                  v.autoClear = false;
                  int c = this->gameintf.radar_getNearbyPlayers(&v);
                  if (c > 0) {
                     for (int i = 0; i < c; i++) {
                        CCharacter *c = static_cast<CCharacter *>(v.elementAt(i));

                        this->inform_playerinfo(c);
                     }
                  }
               }
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
