#include "peer_router.h"
#include <iostream>

using namespace std;
using namespace zmqpp;

class ChatServer : public Listener {

  public:
    void recv_message(zmqpp::message& msg, zmqpp::socket& skt) {
    }

    void other_input(int fd, zmqpp::socket& skt) {
    }

};

int main() {
  ChatServer chat;
  vector<int> filedesc {0};
  start_router("tcp://*:3030",filedesc,chat);
}
