// chat_client.cpp - Assignment: chat client
// usage: chat_client HOST PORT
#include <iostream>
#include <string>
#include <thread>
#include "net.hpp"

static void receive_loop(socket_t s, bool& running) {
  char buf[1024];
  std::string inbuf;
  while (running) {
    long long n = net::recv_some(s, buf, sizeof buf);
    if (n <= 0) {
      std::cout << "\nDisconnected from server.\n";
      running = false;
      break;
    }
    inbuf.append(buf, static_cast<size_t>(n));
    for (;;) {
      size_t pos = inbuf.find('\n');
      if (pos == std::string::npos) break;
      std::string line = inbuf.substr(0, pos);
      inbuf.erase(0, pos + 1);
      if (!line.empty() && line.back() == '\r') line.pop_back();
      std::cout << line << std::endl;
    }
  }
}

int main(int argc, char** argv) {
  const char* host = (argc > 1) ? argv[1] : "127.0.0.1";
  const char* port = (argc > 2) ? argv[2] : "6000";

  try {
    net::Runtime runtime;
    net::Socket s = net::tcp_connect(host, port);
    std::cout << "Connected to " << host << ":" << port
              << ". Type /quit to leave.\n";

    bool running = true;
    std::thread receiver(receive_loop, s.get(), std::ref(running));

    std::string line;
    while (running && std::getline(std::cin, line)) {
      std::string msg = line + "\n";
      if (!net::send_all(s.get(), msg.data(), msg.size())) break;
      if (line == "/quit") break;
    }
    running = false;
    net::shutdown_send(s.get());  // nudges the receiver thread out of recv()
    receiver.join();
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}