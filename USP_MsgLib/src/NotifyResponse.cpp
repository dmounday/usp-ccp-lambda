//
// Gatespace Networks Inc. Copyright 2018-2019. All Rights Reserved.
// Gatespace Networks, Inc. Confidential Material.
// NotifyResponse.cpp
//
//  Created on: Aug 2, 2019
//      Author: dmounday
//

#include "NotifyResponse.h"

namespace gsagent {

NotifyResponse::NotifyResponse (const std::string& id, const std::string& sub_id):
          USPMessage(id, usp::Header_MsgType_NOTIFY_RESP){
  usp::Response* response = get_body()->mutable_response();
  usp::NotifyResp* notify_resp = response->mutable_notify_resp();
  notify_resp->set_subscription_id(sub_id);
}

NotifyResponse::~NotifyResponse () {
// TODO Auto-generated destructor stub
}

} /* namespace gsagent */
