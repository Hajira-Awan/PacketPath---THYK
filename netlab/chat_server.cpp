// chat_server.cpp - Assignment: multi-user chat server
#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "net.hpp"

constexpr size_t MAX_LINE = 1024;
constexpr int MAX_CLIENTS = 60;

struct Client {
  net::Socket sock;
  std::string name;
  std::string inbuf;
};

static bool valid_name(const std::string& n) {
  if (n.empty() || n.size() > 16) return false;
  for (char c : n) {
    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') return false;
  }
  return true;
}

static void send_line(socket_t s, const std::string& text) {
  std::string line = text + "\n";
  net::send_all(s, line.data(), line.size());
}

static void broadcast(std::map<socket_t, Client>& clients, const std::string& text,
                       socket_t except = INVALID_SOCK) {
  for (auto& kv : clients) {
    if (kv.first == except) continue;
    send_line(kv.first, text);
  }
}

// Handle one complete line from one client. May remove the client from "clients"
// (on /quit), so the caller must not use "c" after this returns true.
// Returns true if the client should be dropped.
static bool handle_line(std::map<socket_t, Client>& clients, socket_t fd,
                         std::string line) {
  while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
  if (line.empty()) return false;

  Client& c = clients.at(fd);

  if (line == "/quit") {
    return true;
  }

  if (line.rfind("/nick ", 0) == 0) {
    std::string new_name = line.substr(6);
    if (!valid_name(new_name)) {
      send_line(fd, "Error: name must be 1-16 letters, digits, or _");
      return false;
    }
    for (auto& kv : clients) {
      if (kv.first != fd && kv.second.name == new_name) {
        send_line(fd, "Error: that name is in use");
        return false;
      }
    }
    std::string old_name = c.name;
    c.name = new_name;
    broadcast(clients, old_name + " is now " + new_name);
    return false;
  }

  if (line == "/who") {
    std::string list = "Users:";
    for (auto& kv : clients) list += " " + kv.second.name;
    send_line(fd, list);
    return false;
  }

  // Ordinary chat text
  broadcast(clients, c.name + ": " + line, fd);
  return false;
}

int main(int argc, char** argv) {
  const char* port = (argc > 1) ? argv[1] : "6000";
  try {
    net::Runtime runtime;
    net::Socket listener = net::tcp_listen(port);
    std::cout << "Chat server on port " << port << std::endl;

    std::map<socket_t, Client> clients;
    int next_user_number = 1;

    for (;;) {
      fd_set readset;
      FD_ZERO(&readset);
      FD_SET(listener.get(), &readset);
      socket_t maxfd = listener.get();
      for (auto& kv : clients) {
        FD_SET(kv.first, &readset);
        if (kv.first > maxfd) maxfd = kv.first;
      }

      int ready = select(static_cast<int>(maxfd) + 1, &readset, nullptr, nullptr, nullptr);
      if (ready < 0) net::fail("select");

      // New connection
      if (FD_ISSET(listener.get(), &readset)) {
        socket_t fd = accept(listener.get(), nullptr, nullptr);
        if (fd != INVALID_SOCK) {
          if (static_cast<int>(clients.size()) >= MAX_CLIENTS) {
            send_line(fd, "Error: server is full");
            net::close_socket(fd);
          } else {
            std::string name = "user" + std::to_string(next_user_number++);
            clients.emplace(fd, Client{net::Socket(fd), name, std::string()});
            send_line(fd, "Welcome, you are " + name);
            broadcast(clients, name + " joined", fd);
            std::cout << name << " joined. Active clients: " << clients.size() << std::endl;
          }
        }
      }

      // Data from existing clients
      std::vector<socket_t> dead;
      char buf[1024];
      for (auto& kv : clients) {
        socket_t fd = kv.first;
        if (!FD_ISSET(fd, &readset)) continue;

        long long n = net::recv_some(fd, buf, sizeof buf);
        if (n <= 0) { dead.push_back(fd); continue; }

        kv.second.inbuf.append(buf, static_cast<size_t>(n));
        if (kv.second.inbuf.size() > MAX_LINE * 4) { dead.push_back(fd); continue; }

        for (;;) {
          size_t pos = kv.second.inbuf.find('\n');
          if (pos == std::string::npos) break;
          std::string line = kv.second.inbuf.substr(0, pos);
          kv.second.inbuf.erase(0, pos + 1);
          if (handle_line(clients, fd, line)) { dead.push_back(fd); break; }
        }
      }

      for (socket_t fd : dead) {
        auto it = clients.find(fd);
        if (it == clients.end()) continue;
        std::string name = it->second.name;
        clients.erase(it);
        broadcast(clients, name + " left");
        std::cout << name << " left. Active clients: " << clients.size() << std::endl;
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}