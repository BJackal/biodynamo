#ifndef DEMO_DISTRIBUTION_WORKER_COMM_H_
#define DEMO_DISTRIBUTION_WORKER_COMM_H_

#include <string>

#include "common.h"
#include "communicator.h"

namespace bdm {

class WorkerCommunicator : public Communicator {
 public:
  WorkerCommunicator(DistSharedInfo* info, const std::string& endpoint,
                     CommunicatorId comm_id);
  ~WorkerCommunicator();

  void Connect();
  void HandleOutgoingMessage(zmqpp::message& msg);
  void HandleIncomingMessage();

 private:
  void SendToCoWorker(const std::string& command,
                      zmqpp::message* message = nullptr,
                      const std::string& option = "");

  bool client_;  // Act as client? (aka initiate communication)
  std::string coworker_identity_;

  std::string coworker_str_;
  std::string worker_str_;
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_WORKER_COMM_H_
