//
// Gatespace Networks Inc. Copyright 2018-2019. All Rights Reserved.
// Gatespace Networks, Inc. Confidential Material.
// OnBoardResponse.cpp
//
//  Created on: Aug 2, 2019
//      Author: dmounday
//

#include "OnBoardResponse.h"

namespace gsagent {

OnBoardResponse::OnBoardResponse (const std::string id):
            NotifyResponse(id, ""){
}

OnBoardResponse::~OnBoardResponse () {
}

} /* namespace gsagent */
