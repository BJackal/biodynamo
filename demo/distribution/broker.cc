#include "broker.h"

namespace bdm {

Broker::Broker(zmqpp::context* ctx, const std::string& endpoint,
               const bool verbose)
    : ctx_(ctx), endpoint_(endpoint), verbose_(verbose) {
  this->socket_ = new zmqpp::socket(*ctx_, zmqpp::socket_type::router);
  this->hb_at_ =
      std::chrono::system_clock::now() + duration_ms_t(HEARTBEAT_INTERVAL);
}

Broker::~Broker() {
  if (socket_) {
    delete socket_;
  }
}

void Broker::Bind() {
  socket_->bind(endpoint_);
  std::cout << "I: MDP broker/0.2.0 is active at " << endpoint_ << std::endl;
}

//  This method processes one READY, REPORT, HEARTBEAT or
//  DISCONNECT message sent to the broker by a worker

void Broker::HandleMessageWorker(const std::string& identity,
                                 std::unique_ptr<zmqpp::message> msg) {
  // Message format:
  // Frame 1:     WorkerCommandHeader class (serialized)
  // Frame 2..n:  Application frames
  assert(msg->parts() >= 1);  //  At least, command

  // Frame 1
  std::string* header_str = new std::string(msg->get(0));
  msg->pop_front();

  std::unique_ptr<WorkerCommandHeader> header =
      CommandHeader::FromString<WorkerCommandHeader>(header_str);

  bool worker_ready = (workers_.find(identity) != workers_.end());
  WorkerEntry* worker = GetOrCreateWorker(identity);

  if (header->cmd_ == WorkerProtocolCmd::kReady) {
    if (worker_ready) {
      DeleteWorker(worker);
    } else {
      // Cross-check worker identity
      assert(header->worker_id_ == identity);

      // Monitor worker using heartbeats
      worker->expiry = std::chrono::system_clock::now() + HEARTBEAT_EXPIRY;
      waiting_.insert(worker);

      std::cout << "I: worker " << identity << " created" << std::endl;
    }
  } else if (header->cmd_ == WorkerProtocolCmd::kReport) {
    if (worker_ready) {
      //  Forward worker report message to appropriate client

      // Message format:
      // Frame 1:     client_id (manually; ROUTER socket)
      // Frame 2:     "BDM/0.1C"
      // Frame 3:     ClientCommandHeader class (serialized)
      // Frame 4..n:  Application frames

      // Frame 3
      std::unique_ptr<std::string> c_header =
          ClientCommandHeader(ClientProtocolCmd::kReport,
                              CommunicatorId::kSomeWorker,
                              CommunicatorId::kClient)
              .worker_id(worker->identity)
              .client_id(header->client_id_)
              .ToString();
      msg->push_front(*c_header);

      // Frame 2
      msg->push_front(MDPC_CLIENT);
      // Frame 1
      msg->push_front(header->client_id_);

      if (verbose_) {
        std::cout << "I: broker sends message: " << *msg << std::endl;
      }
      socket_->send(*msg);

    } else {
      DeleteWorker(worker);
    }
  } else if (header->cmd_ == WorkerProtocolCmd::kHeartbeat) {
    if (worker_ready) {
      // Remove and reinsert worker to the waiting
      // queue after updating his expiration time
      waiting_.erase(waiting_.find(worker));
      worker->expiry = std::chrono::system_clock::now() + HEARTBEAT_EXPIRY;
      waiting_.insert(worker);
    } else {
      DeleteWorker(worker);
    }
  } else if (header->cmd_ == WorkerProtocolCmd::kDisconnect) {
    // Delete worker without sending DISCONNECT message
    DeleteWorker(worker, false);
  } else {
    std::cout << "E: invalid input message" << *msg << std::endl;
  }
}

//  Process a request coming from a client.
void Broker::HandleMessageClient(const std::string& sender,
                                 std::unique_ptr<zmqpp::message> msg) {
  // Message format:
  // Frame 1:     ClientCommandHeader class (serialized)
  // Frame 2..n:  Application frames
  assert(msg->parts() >= 1);

  // Frame 1
  std::string* header_str = new std::string(msg->get(0));
  msg->pop_front();

  std::unique_ptr<ClientCommandHeader> header =
      CommandHeader::FromString<ClientCommandHeader>(header_str);

  // Ignore MMI service for now
  // When MMI service is implemented, header->receiver_
  // will point to CommunicatorId::kBroker
  assert(header->receiver_ == CommunicatorId::kSomeWorker);

  if (workers_.find(header->worker_id_) == workers_.end()) {
    // no such worker exist
    std::cout << "E: invalid worker " << header->worker_id_
              << ". Droping message..." << std::endl;

    // Message format:
    // Frame 1:     client_id (manually; ROUTER socket)
    // Frame 2:     "BDM/0.1C"
    // Frame 3:     ClientCommandHeader class (serialized)
    // Frame 4..n:  Application frames

    // Frame 3
    std::unique_ptr<std::string> header =
        ClientCommandHeader(ClientProtocolCmd::kNak, CommunicatorId::kBroker,
                            CommunicatorId::kClient)
            .client_id(sender)
            .ToString();
    msg->push_front(*header);

    // Frame 2
    msg->push_front(MDPC_CLIENT);
    // Frame 1
    msg->push_front(sender);

    if (verbose_) {
      std::cout << "I: sending NAK reply to client: " << *msg << std::endl;
    }

    // send NAK to client
    socket_->send(*msg);
  } else {
    WorkerEntry* worker = workers_[header->worker_id_];

    // Forward the pending messages to the worker
    worker->requests.push_back(msg->copy());

    while (!worker->requests.empty()) {
      zmqpp::message& pending = worker->requests.front();

      // TODO(kkanellis): fix client (what if client is different?)
      worker->Send(socket_, WorkerProtocolCmd::kRequest, &pending, sender);
      worker->requests.pop_front();
    }
  }
}

//  The purge method deletes any idle workers that haven't pinged us in a
//  while. We hold workers from oldest to most recent, so we can stop
//  scanning whenever we find a live worker. This means we'll mainly stop
//  at the first worker, which is essential when we have large numbers of
//  workers (since we call this method in our critical path)

void Broker::Purge() {
  while (!waiting_.empty()) {
    WorkerEntry* worker = *waiting_.begin();
    if (std::chrono::system_clock::now() < worker->expiry) {
      break;
    }

    DeleteWorker(worker, false);
  }
}

//  Lazy constructor that locates a worker by identity, or creates a new
//  worker if there is no worker already with that identity.

WorkerEntry* Broker::GetOrCreateWorker(const std::string& identity) {
  assert(!identity.empty());

  if (workers_.find(identity) == workers_.end()) {
    // Create worker and add him to workers
    WorkerEntry* worker = new WorkerEntry(identity, verbose_);
    workers_[identity] = worker;

    std::cout << "I: registering new worker: " << identity << std::endl;
  }
  return workers_[identity];
}

void Broker::DeleteWorker(WorkerEntry* worker, bool disconnect /* = true */) {
  if (disconnect) {
    worker->Send(socket_, WorkerProtocolCmd::kDisconnect);
  }

  // Remove from waiting & workers list
  waiting_.erase(worker);
  workers_.erase(worker->identity);
  delete worker;

  std::cout << "I: delete expired worker " << worker->identity << std::endl;
}

void Broker::Run() {
  Bind();

  zmqpp::poller poller;
  poller.add(*socket_, zmqpp::poller::poll_in);

  while (true) {
    // Wait till heartbeat duration
    poller.poll(HEARTBEAT_INTERVAL.count());

    if (poller.events(*socket_) & zmqpp::poller::poll_in) {
      auto msg = std::make_unique<zmqpp::message>();
      if (!socket_->receive(*msg)) {
        break;  // Interrupted
      }

      // Message format received (expected)
      // Frame 1:     sender id (auto; DEALER socket)
      // Frame 2:     "BDM/0.1C" || "BDM/0.1W"
      // Frame 3:     *CommandHeader class (serialized)
      // Frame 4..n:  Application frames
      assert(msg->parts() >= 3);

      if (verbose_) {
        std::cout << "I: broker received message: " << *msg << std::endl;
      }

      // Frame 1
      std::string sender = msg->get(0);
      msg->pop_front();

      // Frame 2
      std::string protocol = msg->get(0);
      msg->pop_front();

      if (protocol == MDPC_CLIENT) {
        HandleMessageClient(sender, std::move(msg));
      } else if (protocol == MDPW_WORKER) {
        HandleMessageWorker(sender, std::move(msg));
      } else {
        std::cout << "E: invalid message: " << *msg << std::endl;
      }
    }

    //  Disconnect and delete any expired workers
    //  Send heartbeats to idle workers if needed
    if (std::chrono::system_clock::now() > hb_at_) {
      Purge();
      for (auto worker : waiting_) {
        worker->Send(socket_, WorkerProtocolCmd::kHeartbeat);
      }
      hb_at_ = std::chrono::system_clock::now() + HEARTBEAT_INTERVAL;
    }
  }
}
}  // namespace bdm
