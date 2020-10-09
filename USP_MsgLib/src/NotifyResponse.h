//
// Gatespace Networks Inc. Copyright 2018-2019. All Rights Reserved.
// Gatespace Networks, Inc. Confidential Material.
// NotifyResponse.h
//
//  Created on: Aug 2, 2019
//      Author: dmounday
//

#ifndef SRC_NOTIFYRESPONSE_H_
#define SRC_NOTIFYRESPONSE_H_

#include "USPMessage.h"

namespace gsagent {

class NotifyResponse : public USPMessage {
public:
  NotifyResponse (const std::string& id, const std::string& sup_id );
  virtual ~NotifyResponse ();
};

} /* namespace gsagent */

#endif /* SRC_NOTIFYRESPONSE_H_ */
