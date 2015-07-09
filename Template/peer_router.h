
#ifndef PEER_ROUTER_H
#define PEER_ROUTER_H

#include <zmqpp/zmqpp.hpp>
#include <vector>
#include <unordered_map>
#include <iostream>

enum class PRCommand {
  Register, //Register a peer in the router
  Unregister,
  Send, //Send a message from one peer to other
  Send_Router, //Send a message to the router
  Broadcast, //Send a message to all the other peers
};

class Listener {
  bool _finished = false;
  public:
    virtual void recv_message(zmqpp::message& msg, zmqpp::socket& skt) = 0;
    virtual void other_input(int fileDescriptor, zmqpp::socket& skt) = 0;
    virtual void finish() {
      _finished = true;
    }
    bool finished() const {
      return _finished;
    }
};

/*** Router Code Starts Here ***/

struct router_state {
  std::unordered_map<std::string,int> peer_id;
  std::unordered_map<int,std::string> peer_id_inv;
};

//private method to dispatch peers
void dispatch_peer(zmqpp::socket &peers, zmqpp::message &request,
    router_state& rstate, Listener& listener) {
  std::string id;
  PRCommand cmd;
  int index, icmd;
  zmqpp::message response;
  request >> id >> icmd;
  cmd = static_cast<PRCommand>(icmd);
  switch (cmd) {
    case PRCommand::Register:
      index = rstate.peer_id.size();
      rstate.peer_id[id] = index;
      rstate.peer_id_inv[index] = id;
      break;
    case PRCommand::Unregister:
      index = rstate.peer_id[id];
      rstate.peer_id_inv.erase(index);
      rstate.peer_id.erase(id);
      break;
    case PRCommand::Send:
      request >> index; // read the destination
      response = request.copy();
      response.pop_front(); //id
      response.pop_front(); // op
      response.pop_front(); // dest
      response.push_front(rstate.peer_id_inv[index]);
      peers.send(response);
      break;
    case PRCommand::Send_Router:
      listener.recv_message(request, peers);
      break;
    case PRCommand::Broadcast:
      for (const auto p : rstate.peer_id) {
        if (p.first == id) continue; //don't broadcast to sender
        response = request.copy();
        response.pop_front(); //id
        response.pop_front(); // op

        response.push_front(p.first);
        peers.send(response);
      }
      break;
  }
}

/*
 * Creates a router socket connecting to the given URL (Usually is
 * tcp://localhost:port_number). This function never returns, but call
 * event methods on socket messages or inputs from other file descriptors.
 * Pass a vector with the additional input file descriptors you want to use,
 * for instance you can pass a zero for STDIN.
 */
void start_router(const std::string& url, const std::vector<int>& file_descriptors,
    Listener &listener) {
  zmqpp::context ctx;
  zmqpp::socket peers(ctx, zmqpp::socket_type::router);
  peers.bind(url);

  zmqpp::poller pol;
  pol.add(peers);
  for (auto fd : file_descriptors)
    pol.add(fd);

  router_state rstate;
  zmqpp::message msg;
  while (!listener.finished()) {
    if (pol.poll()) {
      if (pol.has_input(peers)) {
        peers.receive(msg);
        dispatch_peer(peers, msg, rstate, listener);
      }
      for (auto fd : file_descriptors) {
        if (pol.has_input(fd)) {
          listener.other_input(fd, peers);
        }
      }
    }
  }
}


/*** Router code ends here ***/

/*** Peer Code Starts Here ***/

/*
 * Send a message to the peer identified by the id dest.
 */
void send_peer(zmqpp::socket &router, zmqpp::message &me, int dest) {
  me.push_front(dest);
  me.push_front(static_cast<int>(PRCommand::Send));
  router.send(me);
}

void broadcast_peer(zmqpp::socket &router, zmqpp::message &me) {
  me.push_front(static_cast<int>(PRCommand::Broadcast));
  router.send(me);
}

/*
 * Creates a client socket connecting to the given URL (Where the
 * router is supposed to be). This function never returns, but call
 * event methods on socket messages or inputs from other file descriptors.
 * Pass a vector with the additional input file descriptors you want to use,
 * for instance you can pass a zero for STDIN.
 */
void start_peer(const std::string& url, const std::vector<int>& file_descriptors,
    Listener &listener) {
  zmqpp::context ctx;
  zmqpp::socket router(ctx, zmqpp::socket_type::dealer);
  router.connect(url);

  zmqpp::message reg;
  reg << static_cast<int>(PRCommand::Register);
  router.send(reg);

  zmqpp::poller pol;
  pol.add(router);
  for (auto fd : file_descriptors)
    pol.add(fd);

  zmqpp::message msg;
  while (!listener.finished()) {
    if (pol.poll()) {
      if (pol.has_input(router)) {
        router.receive(msg);
        listener.recv_message(msg, router);
      }
      for (auto fd : file_descriptors) {
        if (pol.has_input(fd)) {
          listener.other_input(fd, router);
        }
      }
    }
  }
}

/*** Peer code ends here ***/

#endif
