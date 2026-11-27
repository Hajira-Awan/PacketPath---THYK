// thread_server.cpp - Tutorial 8: one thread for each client
// An echo server that serves many clients at the same time.
// Each client gets its own std::thread. The server shows how many are active.

#include <atomic>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include "net.hpp"

static std::atomic<int> g_active{0};  // number of clients that are connected now
static std::mutex g_log_mutex;        // one thread writes to the console at a time

static void log_line(const std::string& text) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    std::cout << text << std::endl;
}

// This function runs in its own thread. It owns the client socket.
static void serve_client(net::Socket client, std::string who) {
    int now = ++g_active;
    log_line(who + " connected. Active clients: " + std::to_string(now));

    char buf[1024];
    for (;;) {
        long long n = net::recv_some(client.get(), buf, sizeof buf);
        if (n <= 0) break;  // 0 = the client closed, < 0 = error
        if (!net::send_all(client.get(), buf, static_cast<size_t>(n))) break;
    }

    now = --g_active;
    log_line(who + " disconnected. Active clients: " + std::to_string(now));
    // The Socket destructor closes the socket when the thread ends.
}

int main(int argc, char** argv) {
    const char* port = (argc > 1) ? argv[1] : "5000";
    try {
        net::Runtime runtime;
        net::Socket listener = net::tcp_listen(port);
        log_line(std::string("Thread server on port ") + port + ". Press Ctrl+C to stop.");

        for (;;) {
            sockaddr_storage peer{};
            sock_len peer_len = sizeof peer;
            net::Socket client(accept(listener.get(), reinterpret_cast<sockaddr*>(&peer), &peer_len));
            if (!client.valid()) continue;
            std::string who = net::peer_name(reinterpret_cast<sockaddr*>(&peer), peer_len);

            // Give the socket to a new thread. detach(): nobody waits for it.
            std::thread(serve_client, std::move(client), who).detach();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}