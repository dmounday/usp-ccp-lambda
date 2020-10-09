//
// USPmessage.cpp
//
//  Created on: Jan 23, 2019
//      Author: dmounday
//
#include "USPMessage.h"

#include "usp-msg-1-1.pb.h"

namespace gsagent {
static unsigned usp_next_id = 1000;

USPMessage::USPMessage (const std::string& id, usp::Header_MsgType mtype){
  usp::Header *header = message.mutable_header();
  header->set_msg_type(mtype);
  header->set_msg_id(id);
  body = message.mutable_body();
}
USPMessage::USPMessage (usp::Header_MsgType mtype){
  usp::Header *header = message.mutable_header();
  header->set_msg_type(mtype);
  header->set_msg_id(std::to_string(++usp_next_id));
  body = message.mutable_body();
}
USPMessage::~USPMessage () {
}
void USPMessage::encode_header(usp::Msg& msg){
  return;
}
} /* namespace gsagent */
