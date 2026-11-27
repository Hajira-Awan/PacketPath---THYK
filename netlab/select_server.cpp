// select_server.cpp - Tutorial 9: echo server, one thread, many clients
#include <iostream>
#include <map>
#include <vector>
#include "net.hpp"

int main(int argc, char** argv) {
  const char* port = (argc > 1) ? argv[1] : "5000";
  try {
    net::Runtime runtime;
    net::Socket listener = net::tcp_listen(port);
    std::cout << "Select server on port " << port << std::endl;

    std::map<socket_t, net::Socket> clients;

    for (;;) {
      // Build the set: listener + every client
      fd_set readset;
      FD_ZERO(&readset);
      FD_SET(listener.get(), &readset);
      socket_t maxfd = listener.get();
      for (auto& kv : clients) {
        FD_SET(kv.first, &readset);
        if (kv.first > maxfd) maxfd = kv.first;
      }

      // Wait until at least one socket is ready
      int ready = select(static_cast<int>(maxfd) + 1, &readset,
                         nullptr, nullptr, nullptr);
      if (ready < 0) net::fail("select");

      // New client?
      if (FD_ISSET(listener.get(), &readset)) {
        sockaddr_storage peer{};
        sock_len peer_len = sizeof peer;
        socket_t fd = accept(listener.get(),
                             reinterpret_cast<sockaddr*>(&peer), &peer_len);
        if (fd != INVALID_SOCK) {
          std::cout << "Client "
                    << net::peer_name(reinterpret_cast<sockaddr*>(&peer), peer_len)
                    << " connected. Active clients: " << clients.size() + 1
                    << std::endl;
          clients.emplace(fd, net::Socket(fd));
        }
      }

      // Data from existing clients?
      std::vector<socket_t> dead;
      char buf[1024];
      for (auto& kv : clients) {
        if (!FD_ISSET(kv.first, &readset)) continue;
        long long n = net::recv_some(kv.first, buf, sizeof buf);
        if (n <= 0 || !net::send_all(kv.first, buf, static_cast<size_t>(n)))
          dead.push_back(kv.first);
      }
      for (socket_t fd : dead) {
        clients.erase(fd);   // Socket destructor closes it
        std::cout << "Client disconnected. Active clients: "
                  << clients.size() << std::endl;
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}