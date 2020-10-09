//
// USPerror.h
//
//  Created on: Jan 23, 2019
//      Author: dmounday
//

#ifndef USPERROR_H_
#define USPERROR_H_

#include "USPMessage.h"

namespace gsagent {

class USP_error : public USPMessage {
public:
  USP_error (std::string& id);
  virtual ~USP_error ();
protected:
  usp::Error *error;
};

} /* namespace gsagent */

#endif /* USPERROR_H_ */
