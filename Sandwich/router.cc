#include <unordered_map>
#include <vector>
#include <chrono>
#include <fstream>

#include <zmqpp/zmqpp.hpp>
#include <sodium.h>

using namespace std;
using namespace zmqpp;

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::milliseconds ms;
typedef std::chrono::duration<float> fsec;

// State
std::chrono::time_point<std::chrono::system_clock> tstart, tend;
unordered_map<string, int> peer_id;
vector<string> peer_id_inv;
vector<pair<int,int>> tc;
int tc_id;
string key;

void next_testcase(socket& peers) {
  if ((size_t)++tc_id > tc.size()) return;
  tstart = Time::now();
  for (const auto &it : peer_id) {
    message m;
    m << it.first << "start" << it.second << (int)peer_id.size();
    m << key;
    m << (int)tc_id;
//cout << "started id=" << it.second << " nnodes=" << peer_id.size() << " tc_id=" << tc_id << " key=" << key << endl;
    m << tc[tc_id-1].first << tc[tc_id-1].second;
    peers.send(m);
  }
}

void dispatch_peer(socket &peers, message &request) {
  string id, command;
  request >> id >> command;
  if (peer_id.count(id) == 0) {
    peer_id_inv.push_back(id);
    peer_id[id] = peer_id_inv.size() - 1;
  }
  if (command == "send") {
    int dest;
    request >> dest;
    message response = request.copy();
    response.pop_front(); // id
    response.pop_front(); // op
    response.pop_front(); // dest
    response.push_front(peer_id_inv[dest]);
    //cout << "from " << peer_id[id] << " to " << dest << endl;
    peers.send(response);
  } else if (command == "ans") {
    tend = Time::now();
    fsec fs = tend - tstart;
    string command;
    int curr_tc;
    uint64_t answer;
    request >> curr_tc >> answer;
    std::cout << "Time of test case " << curr_tc << ": " << fs.count() << "s\n";
    cout << answer << endl;
    next_testcase(peers);
  }
}

void show() {
  for (const auto &it : peer_id) {
    cout << it.second << " : " << it.first << endl;
  }
}

void start(socket &peers, const char *fname) {
  ifstream in(fname);
  int x;

  key.clear();
  tc.clear();
  for (int i = 0; i < 32; ++i) {
    in >> x;
    key.push_back((unsigned char)x);
  }
  int N, maxTaste;
  while (in >> N >> maxTaste) {
    tc.push_back(make_pair(N,maxTaste));
  }
  tc_id = 0;
  next_testcase(peers);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    cout << "usage " << argv[0] << " endpoint in-file" << endl;
    return 1;
  }
  if (sodium_init() == -1) return 1;

  context ctx;
  socket peers(ctx, socket_type::router);

  peers.bind(argv[1]);

  poller pol;
  pol.add(peers);
  pol.add(0);

  message request;

  while (true) {
    if (pol.poll()) {
      if (pol.has_input(peers)) {
        peers.receive(request);
        dispatch_peer(peers, request);
      }
      if (pol.has_input(0)) {
        string tmp;
        cin >> tmp;
        if (tmp == "show")
          show();
        if (tmp == "start")
          start(peers, argv[2]);
      }
    }
  }

  return 0;
}
