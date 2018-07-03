#ifndef DEMO_DISTRIBUTION_CLIENT_H_
#define DEMO_DISTRIBUTION_CLIENT_H_

#include <string>

#include "common.h"
#include "logger.h"
#include "protocol.h"

namespace bdm {

class Client {
 public:
  Client(zmqpp::context *ctx, const std::string &identity,
         const std::string &broker, LoggingLevel level);
  ~Client();

  void SetTimeout(duration_ms_t timeout);

  template <typename T>
  void SetSocketOption(zmqpp::socket_option option, const T &value);

  template <typename T>
  T GetSocketOption(zmqpp::socket_option option);
  template <typename T>
  void GetSocketOption(zmqpp::socket_option option, T *value);

  void SendToWorker(std::unique_ptr<zmqpp::message> msg,
                    const std::string &worker_id);
  bool Recv(std::unique_ptr<zmqpp::message> *msg_out,
            ClientProtocolCmd *command = nullptr,
            std::string *recv_from = nullptr);

  // RPC calls to broker service
  bool CheckWorker(const std::string &worker_id);
  bool RequestBrokerTermination();

 private:
  void ConnectToBroker();
  void Send(ClientProtocolCmd cmd, CommunicatorId receiver,
            std::unique_ptr<zmqpp::message> msg = nullptr,
            const std::string &worker_id = "");

  zmqpp::context *ctx_;                    //  Our context
  std::unique_ptr<zmqpp::socket> socket_;  //  Socket to broker
  std::string identity_;                   //  Client identity
  std::string broker_endpoint_;            //  Broker address
  duration_ms_t timeout_;                  //  Request timeout

  Logger logger_;
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_CLIENT_H_
