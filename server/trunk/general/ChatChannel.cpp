

#include "ChatChannel.h"

CChatChannel *Global_Chat_Channel = NULL;

class CClientChatter: public TGFFreeable {
public:
   CTelnetConnection *c;
   TGFString s;
};

CChatChannel::CChatChannel() : TGFThread() {
   this->channelnr = 2;

   this->start();
}

CChatChannel::~CChatChannel() {
   this->stopAndWait();
}

void CChatChannel::addClient( CTelnetConnection *pClient ) {
   this->clients.addElement( pClient );
}

void CChatChannel::delClient( CTelnetConnection *pClient ) {
   this->clients.removeElement(pClient);
}

void CChatChannel::execute() {

   CClientChatter *cc = static_cast<CClientChatter *>( this->msgqueue.pop() );

   if ( cc != NULL ) {
      unsigned int c = this->clients.size();
      for ( unsigned i = 0; i < c; i++ ) {
         CTelnetConnection *c = static_cast<CTelnetConnection *>( this->clients.elementAt(i) );
         if ( c != NULL ) {
            if ( c->isRunning() ) {
               if ( c->inBinaryMode() ) {
                  c->sendBin( c_response_chatmessage, this->channelnr, 0, &cc->s );
               } else {
                  c->send( &cc->s );
               }
            } else {
               this->delClient(c);
            }
         }
      }

      delete cc;
   }
}

void CChatChannel::messageToAll( CTelnetConnection *pFrom, const TGFString *sMsg ) {
   TGFString *sFullMsg = new TGFString();
   if (pFrom != NULL) {
      sFullMsg->append_ansi("<");
      pFrom->append_nickname(sFullMsg);
      sFullMsg->append_ansi(">: ");
   }
   sFullMsg->append( sMsg );
   sFullMsg->append( char(13) );
   sFullMsg->append( char(10) );

   CClientChatter *cc = new CClientChatter();
   cc->c = pFrom;
   cc->s.setValue( sFullMsg );

   this->msgqueue.push( cc );
}

void CChatChannel::messageToAll_ansi( CTelnetConnection *pFrom, const char *sMsg ) {
   TGFString tmp(sMsg);

   this->messageToAll(pFrom, &tmp);
}

bool initGlobalChatChannel() {
   Global_Chat_Channel = new CChatChannel();
   Global_Chat_Channel->start();

   return true;
}

void finiGlobalChatChannel() {
   Global_Chat_Channel->stopAndWait();
   delete  Global_Chat_Channel;

   Global_Chat_Channel = NULL;
}

void enterGlobalChatChannel(CTelnetConnection *pClient ) {
   Global_Chat_Channel->addClient(pClient);
}
void leaveGlobalChatChannel(CTelnetConnection *pClient ) {
   Global_Chat_Channel->delClient(pClient);
}

void sendToGlobalChatChannel( CTelnetConnection *pFrom, const char *sMsg ) {
   Global_Chat_Channel->messageToAll_ansi( pFrom, sMsg );
}

void sendToGlobalChatChannel( CTelnetConnection *pFrom, const TGFString *sMsg ) {
   Global_Chat_Channel->messageToAll( pFrom, sMsg );
}
