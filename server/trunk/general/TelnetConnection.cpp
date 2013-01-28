

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

void CTelnetConnection::sendBin(DWORD32 command, DWORD32 intparam1, DWORD32 intparam2, TGFString *s) {
   DWORD32 strlen = s->getLength();

   TGFString tmp;
   tmp.append((command & 0xff000000) >> 24);
   tmp.append((command & 0x00ff0000) >> 16);
   tmp.append((command & 0x0000ff00) >> 8);
   tmp.append((command & 0x000000ff));

   bool bIsStrCommand = ((command & 0x10000000) > 0);
   bool bIntParamCommand = ((command & 0x20000000) > 0);

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

   if (bIsStrCommand) {
      tmp.append((strlen & 0xff000000) >> 24);
      tmp.append((strlen & 0x00ff0000) >> 16);
      tmp.append((strlen & 0x0000ff00) >> 8);
      tmp.append((strlen & 0x000000ff));

      tmp.append(s,s->getLength());
   }

   this->send(&tmp);
}

bool CTelnetConnection::decodeNextBinMessageInBuffer(DWORD32 *command, DWORD32 *intparam1, DWORD32 *intparam2, TGFString *s) {
   long iBufLen = buffer.getLength();

   if (iBufLen < 4) {
      return false;
   }

   unsigned char *arrBuffer = reinterpret_cast<unsigned char *>( buffer.getValue() );

   DWORD32 iCommand = 0;

   iCommand |= static_cast<DWORD32>(arrBuffer[0]) << 24;
   iCommand |= static_cast<DWORD32>(arrBuffer[1]) << 16;
   iCommand |= static_cast<DWORD32>(arrBuffer[2]) << 8;
   iCommand |= static_cast<DWORD32>(arrBuffer[3]);

   bool bIsStrCommand = ((iCommand & 0x10000000) > 0);
   bool bIntParamCommand = ((iCommand & 0x20000000) > 0);

   if (!(bIsStrCommand || bIntParamCommand)) {
      *command = iCommand;

      buffer.remove(0, 3);

      return true;
   } else if ((bIsStrCommand || bIntParamCommand) && (iBufLen < 16) ) {
      return false;
   } else if (bIntParamCommand && bIsStrCommand && (iBufLen < 20)) {
      return false;
   }

   long i = 4;
   if (bIntParamCommand) {
      DWORD32 iIntParam1 = 0;
      DWORD32 iIntParam2 = 0;

      iIntParam1 |= static_cast<DWORD32>(arrBuffer[i]) << 24;
      iIntParam1 |= static_cast<DWORD32>(arrBuffer[i+1]) << 16;
      iIntParam1 |= static_cast<DWORD32>(arrBuffer[i+2]) << 8;
      iIntParam1 |= static_cast<DWORD32>(arrBuffer[i+3]);

      *intparam1 = iIntParam1;

      i += 4;

      iIntParam2 |= static_cast<DWORD32>(arrBuffer[i]) << 24;
      iIntParam2 |= static_cast<DWORD32>(arrBuffer[i+1]) << 16;
      iIntParam2 |= static_cast<DWORD32>(arrBuffer[i+2]) << 8;
      iIntParam2 |= static_cast<DWORD32>(arrBuffer[i+3]);

      *intparam1 = iIntParam1;

      i += 4;
   }

   DWORD32 iStrLen = 0;
   if (bIsStrCommand) {

      iStrLen |= static_cast<DWORD32>(arrBuffer[i]) << 24;
      iStrLen |= static_cast<DWORD32>(arrBuffer[i+1]) << 16;
      iStrLen |= static_cast<DWORD32>(arrBuffer[i+2]) << 8;
      iStrLen |= static_cast<DWORD32>(arrBuffer[i+3]);

      i += 4;

      if ((i + iStrLen) < iBufLen) {
         return false;
      }

      s->setValue(buffer.getPointer(i), iStrLen);
   }

   buffer.remove(0, i + iStrLen - 1);

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
   this->gameintf.GetLastActionInfo(&tmp);

   if (!bBinaryMode) {
      if (tmp.getLength() > 0) {
         tmp.append_ansi("\r\n");
         this->send(&tmp);
      }
   } else {
      this->sendBin(c_response_lastactioninfo, 0, 0, &tmp);
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
            bActionOk = true;
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

         // todo: make server understand binary speak

         //copy.startsWith_ansi("/binarymode_on\r\n")

         DWORD32 command;
         DWORD32 intparam1;
         DWORD32 intparam2;
         TGFString s;

         if ( decodeNextBinMessageInBuffer(&command, &intparam1, &intparam2, &s) )  {
            if (command == c_run_walkbackwards) {
               bActionOk = this->gameintf.run_walkbackwards();
            } else if (command == c_run_walkforward) {
               bActionOk = this->gameintf.run_walkforward();
            } else if (command == c_run_walkleft) {
               bActionOk = this->gameintf.run_walkleft();
            } else if (command == c_run_walkright) {
               bActionOk = this->gameintf.run_walkright();
            } else if (command == c_attack_start) {
               bActionOk = this->gameintf.attack_start(&s);
            } else if (command == c_attack_stop) {
               //this->gameintf.attack_stop();
            } else if (command == c_chat_say) {
               if (intparam1 == 2) {
                  sendToGlobalChatChannel( this, &s );
               }
            }

            if (bActionOk) {
               this->inform_lastaction();
               this->inform_currentroom();
               this->inform_map();
            } else {
               this->inform_lastaction();
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
