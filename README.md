# distributed-programming-contest

This repository aims to give you the tools to run a distributed programming contest environment,
something quite similar to the [DGC](http://code.google.com/codejam/distributed_index.html).

## dependencies

  - [ZeroMQ](http://zeromq.org/)
  - [ZMQPP](https://github.com/zeromq/zmqpp)
  - [Libsodium](https://github.com/jedisct1/libsodium)
  - c++11 compliant compiler (g++ >= 4.8)

## Examples.

----

### Sandwich

This is a problem taken from the examples of distributed google code jam
[see here](https://code.google.com/codejam/distributed_faq.html)

#### Router

Is in charge of send the instruction "start" to all the other peers, also works forwarding messages
from one node to other node.

#### Peer

Each peer is is charge to solve some part of the main task, this code must be included/modified for
each specific task.

---

### API

Soon.

____

brought to you by in-silico.
