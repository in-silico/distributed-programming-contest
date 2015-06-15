#include <zmqpp/zmqpp.hpp>
#include <bits/stdc++.h>
#include <sodium.h>

using namespace std;
using namespace zmqpp;

typedef long long int LL;

struct Sandwich {
  int N;
  LL nonce;
  unsigned char key[32];
  int maxTaste;
  int tmp1[16], tmp2;

  Sandwich(int N, LL start_nonce, const unsigned char* key) : N(N), nonce(start_nonce) {
    copy(key, key+32, this->key);
    tmp2=-1; maxTaste = 1000000000;
  }

  int getTaste(int i) {
    int base = i/16;
    if (base != tmp2) {
      LL tmp3 = nonce + base;
      crypto_stream_salsa208((unsigned char*)tmp1,sizeof(tmp1),
          (unsigned char*)&tmp3, key);
      tmp2 = base;
    }
    return (tmp1[i % 16] % maxTaste);
  }

  int getN() const {
    return N;
  }
};

void send(socket &router, message &me, int dest) {
  me.push_front(dest);
  me.push_front("send");
  router.send(me);
}

int myid, num_nodes;

void solve(Sandwich& s, socket &router) {
  int64_t minv=0, ssum=0, tsum=0;
  int64_t minpsum=0, maxpsum=-s.maxTaste;
  int block_size = (s.getN()-1) / num_nodes + 1;
  for (int i=myid*block_size; i<min((myid+1)*block_size,s.getN()); i++) {
    int elem = s.getTaste(i);
    //cout << elem << " ";
    minpsum = min(minpsum, tsum);
    ssum += -elem; tsum += -elem;
    maxpsum = max(maxpsum, tsum);
    minv = max(minv, ssum);
    if (ssum < 0) ssum=0;
  }
  message ans;
  ans << "join" << myid << minv << minpsum << maxpsum << tsum;
  send(router, ans, 0);
}

struct Entry {
  int64_t ans, minsum, maxsum, sum;
};

int main(int argc, char **argv) {
  if (argc < 2) {
    cout << "usage " << argv[0] << " endpoint" << endl;
    return 0;
  }
  context ctx;
  socket router(ctx, socket_type::dealer);

  router.connect(argv[1]);

  message m;
  m << "awake";
  router.send(m);

  poller pol;
  pol.add(router);
  pol.add(0);

  message request;
  string command, key;
  int N, maxTaste, tc_id;
  vector<Entry> summary;
  int num_filled=0;

  while (true) {
    if (pol.poll()) {
      if (pol.has_input(router)) {
        router.receive(request);
        request >> command;
        if (command == "start") {
          request >> myid >> num_nodes >> key;
          summary.resize(num_nodes);
          request >> tc_id;
          //cout << "started id=" << myid << " nnodes=" << num_nodes << " tc_id=" << tc_id << " key=" << key << endl;
          request >> N >> maxTaste;
          Sandwich s(N,((LL)tc_id)<<30,(unsigned char*)key.c_str());
          s.maxTaste = maxTaste;
          num_filled = 0;
          solve(s,router);
        } else if (command == "join") {
          int from;
          request >> from;
          request >> summary[from].ans >> summary[from].minsum >> summary[from].maxsum >> summary[from].sum;
          num_filled++;
          if (num_filled == num_nodes) {
            message m;
            int64_t ans = 0, sum = 0, minsum=0;
            for (int i=0; i<num_nodes; i++) {
              //cout << "node=" << i <<  "ans= " << summary[i].ans << "sum=" << summary[i].sum << " minsum=" << summary[i].minsum << " maxsum=" << summary[i].maxsum << endl;
              ans = max(ans, summary[i].ans);
              ans = max(ans, summary[i].maxsum + sum - minsum);
              minsum = min(minsum, sum + summary[i].minsum);
              sum += summary[i].sum;
            }
            m << "ans" << tc_id << ans - sum;
            router.send(m);
          }
        }
      }
      if (pol.has_input(0)) {
        /*int dest;
        cin >> dest;
        m << "send" << dest << "cad1" << 2343 << "cad2";
        router.send(m);*/
      }
    }
  }

  return 0;
}
