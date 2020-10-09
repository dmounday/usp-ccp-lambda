//
// USPresponse.cpp
//
//  Created on: Jan 26, 2019
//      Author: dmounday
//

#include "USPresponse.h"

namespace gsagent {

USPResponse::USPResponse (std::string id, usp::Header_MsgType mtype):
                        USPMessage(id, mtype)
{
  response = get_body()->mutable_response();
}

USPResponse::~USPResponse () {
}

} /* namespace gsagent */
