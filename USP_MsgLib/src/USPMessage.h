//
// USPmessage.h
//
//  Created on: Jan 23, 2019
//      Author: dmounday
//

#ifndef USPMESSAGE_H_
#define USPMESSAGE_H_
#include <string>
#include "usp-msg-1-1.pb.h"

namespace gsagent {

class USPMessage {
public:
  USPMessage (const std::string& id, usp::Header_MsgType mtype);
  USPMessage (usp::Header_MsgType mtype);
  virtual ~USPMessage ();
  void encode_header(usp::Msg&);
  inline usp::Msg* get_message(){return &message;}
  inline usp::Body* get_body(){return body;}
protected:
  usp::Msg message;
  usp::Body *body;
};

} /* namespace gsagent */

#endif /* USPMESSAGE_H_ */
