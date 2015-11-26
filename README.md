# P2P-Over-MiddleBoxes-Demo
A simple demo of P2P communication over middle boxes such as NAT

##compile:

    g++ -g -lboost_thread -lboost_system  client.cpp -o client
    g++ -g -lboost_thread -lboost_system  server.cpp -o server

##run:

   ./server \[port\]
   ./client \[host\] \[port\]
