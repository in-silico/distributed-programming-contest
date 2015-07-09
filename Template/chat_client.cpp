
#include "peer_router.h"
#include <iostream>

using namespace std;
using namespace zmqpp;

class ChatClient : public Listener {
  string myname;
  public:
    void recv_message(zmqpp::message& msg, zmqpp::socket& skt) {
      string sender, text;
      msg >> sender >> text;
      cout << sender << ": " << text << endl;
    }

    void other_input(int fd, zmqpp::socket& skt) {
      string text;
      if (fd == 0) {
        //read from stdin
        getline(cin, text);
        message msg;
        if (text == "exit") {
          msg << static_cast<int>(PRCommand::Unregister);
          skt.send(msg);
          finish();
        } else {
          msg << myname << text;
          cout << "Sending: " << text << endl;
          broadcast_peer(skt,msg);
        }
      }
    }

    void setName(const string& s) {
      myname = s;
    }

    string getName() const {
      return myname;
    }
};

int main() {
  ChatClient chat;
  string myname;
  cout << "Write your name here: ";
  getline(cin, myname);
  chat.setName(myname);
  vector<int> filedesc {0};
  start_peer("tcp://localhost:3030",filedesc,chat);
}
