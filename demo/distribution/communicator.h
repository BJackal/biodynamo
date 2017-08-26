#ifndef DEMO_DISTRIBUTION_COMMUNICATOR_H_
#define DEMO_DISTRIBUTION_COMMUNICATOR_H_

#include <string>

#include "common.h"

namespace bdm {

class Communicator {
 public:
  Communicator(DistSharedInfo* info, std::string endpoint,
               CommunicatorId comm_id)
      : info_(info),
        socket_(nullptr),
        endpoint_(endpoint),
        comm_id_(comm_id),
        is_connected_(false) {}

  virtual ~Communicator() {}

  virtual void ReactorTimedOut() {}
  virtual void ReactorServedRequests() {}

  virtual void Connect() = 0;
  virtual void HandleOutgoingMessage(std::unique_ptr<zmqpp::message> msg) = 0;
  virtual void HandleIncomingMessage() = 0;

  virtual CommunicatorId GetCommunicationId() {
    assert(comm_id_ != CommunicatorId::kUndefined);
    return comm_id_;
  }

  virtual zmqpp::socket* GetSocketPtr() { return socket_.get(); }

  virtual bool IsConnected() { return is_connected_; }

  template <typename T>
  void SetSocketOption(const zmqpp::socket_option& option, const T& value) {
    socket_->set(option, value);
  }

  template <typename T>
  T GetSocketOption(const zmqpp::socket_option& option) {
    T value;
    GetSocketOption(option, &value);
    return value;
  }

  template <typename T>
  void GetSocketOption(const zmqpp::socket_option& option, T* value) {
    socket_->get(option, *value);
  }

 protected:
  DistSharedInfo* info_;

  std::unique_ptr<zmqpp::socket> socket_;
  std::string endpoint_;

  CommunicatorId comm_id_;
  bool is_connected_;
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_COMMUNICATOR_H_
