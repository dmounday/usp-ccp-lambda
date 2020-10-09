//
// Gatespace Networks Inc. Copyright 2018-2019. All Rights Reserved.
// Gatespace Networks, Inc. Confidential Material.
// OnBoardResponse.h
//
//  Created on: Aug 2, 2019
//      Author: dmounday
//

#ifndef SRC_ONBOARDRESPONSE_H_
#define SRC_ONBOARDRESPONSE_H_

#include "NotifyResponse.h"

namespace gsagent {

class OnBoardResponse : public NotifyResponse {
public:
  OnBoardResponse (const std::string id );
  virtual ~OnBoardResponse ();
};

} /* namespace gsagent */

#endif /* SRC_ONBOARDRESPONSE_H_ */
