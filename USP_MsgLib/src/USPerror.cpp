//
// USPerror.cpp
//
//  Created on: Jan 23, 2019
//      Author: dmounday
//

#include "USPerror.h"

namespace gsagent {

USP_error::USP_error (std::string& id):
                USPMessage(id, usp::Header_MsgType_ERROR){
  error = get_body()->mutable_error();
}

USP_error::~USP_error () {
}

} /* namespace gsagent */
