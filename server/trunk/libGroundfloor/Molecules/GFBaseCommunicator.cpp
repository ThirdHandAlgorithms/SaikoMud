
#include "GFBaseCommunicator.h"

TGFCommReturnData::TGFCommReturnData() : TGFFreeable() {
   eof = false;
   error = false;
   errorcode = 0;
   affected = 0;
}

TGFCommReturnData::~TGFCommReturnData() {
}

TGFBaseCommunicator::TGFBaseCommunicator() : TGFFreeable() {
   bConnected = false;
}

TGFBaseCommunicator::~TGFBaseCommunicator() {
}

bool TGFBaseCommunicator::isConnected() {
   return bConnected;
}
