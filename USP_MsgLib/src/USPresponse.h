//
// USPresponse.h
//
//  Created on: Jan 26, 2019
//      Author: dmounday
//

#ifndef USPRESPONSE_H_
#define USPRESPONSE_H_

#include "USPMessage.h"

namespace gsagent {

class USPResponse : public USPMessage {
public:
  USPResponse (std::string, usp::Header_MsgType);
  virtual ~USPResponse ();
  usp::Response *response;
  inline usp::Response *get_response(){return response;}
};

} /* namespace gsagent */

#endif /* USPRESPONSE_H_ */
