#ifndef DEMO_DISTRIBUTION_WORKER_ENTRY_H__
#define DEMO_DISTRIBUTION_WORKER_ENTRY_H__

#include <chrono>
#include <list>
#include <string>

#include "common.h"

namespace bdm {

class WorkerEntry {
 public:
  explicit WorkerEntry(const std::string& identity) {
    this->identity = identity;
    this->expiry = std::chrono::system_clock::now() + HEARTBEAT_EXPIRY;
  }
  ~WorkerEntry() {}

  void Send(zmqpp::socket* sock, const std::string& command,
            zmqpp::message* message, bool verbose,
            const std::string& option = "") const {
    auto msg = message ? message->copy() : zmqpp::message();

    //  Stack protocol envelope to start of message
    if (!option.empty()) {
      msg.push_front(option);
    }
    msg.push_front(command);
    msg.push_front(MDPW_WORKER);

    //  Stack routing envelope to start of message
    msg.push_front("");
    msg.push_front(identity);

    if (verbose) {
      std::cout << "I: sending " << command << " to worker: " << msg
                << std::endl;
    }

    sock->send(msg);
  }

  std::list<zmqpp::message> requests;  // Pending requests
  std::string identity;                // Worker (printable) identity
  time_point_t expiry;                 // When to send HEARTBEAT
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_WORKER_ENTRY_H__
