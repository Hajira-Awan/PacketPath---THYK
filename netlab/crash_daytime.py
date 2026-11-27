"""
crash_daytime.py - bonus: probe daytime_server.cpp (Tutorial 1) for a real weakness.

Target weakness: the server is single-threaded and BLOCKING.
  accept() -> send() -> close()  one client at a time, no timeout.

send() normally returns right away, because the daytime message is only
a few dozen bytes. But if we open MANY connections at once without ever
reading from them, the operating system can only let the server accept
new connections as fast as the backlog (5, in this server's listen() call)
allows. Once the backlog is full, new clients get "connection refused" and
the server LOOKS dead to everyone else, even though the process is alive.
Think of it as a denial-of-service, not a memory-corruption crash.

Run this on the SAME machine as the server, against your OWN server only.
Usage:
    python3 crash_daytime.py 127.0.0.1 1313
"""
import socket
import sys
import time

HOST = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
PORT = int(sys.argv[2]) if len(sys.argv) > 2 else 1313
N = 200          # how many connections to open and hold
TIMEOUT = 5      # seconds to wait for a normal client afterwards


def try_normal_client():
    """A normal, well-behaved client: connect, read, close."""
    try:
        with socket.create_connection((HOST, PORT), timeout=TIMEOUT) as s:
            data = s.recv(200)
            return True, data
    except Exception as e:
        return False, str(e)


def main():
    print(f"Opening {N} connections to {HOST}:{PORT} and holding them open...")
    held = []
    refused = 0
    for i in range(N):
        try:
            s = socket.create_connection((HOST, PORT), timeout=1)
            held.append(s)
        except Exception as e:
            refused += 1
    print(f"  {len(held)} connections accepted, {refused} refused.")

    print("Checking whether a brand-new, well-behaved client can still be served...")
    ok, info = try_normal_client()
    if ok:
        print(f"  Still works: server replied with {info!r}")
        print("  The server survived. Tutorial 1's server sends and closes fast enough")
        print("  that a small flood did not deny service. Try raising N, or see")
        print("  crash_web.py instead, which targets the web server's request parser.")
    else:
        print(f"  FAILED to get a reply: {info}")
        print("  This is the bonus finding: a flood of connections exhausted the")
        print("  listen backlog (5, set in listen(listener, 5)) and/or the single")
        print("  accept() thread, so a normal client could not be served. That is")
        print("  a denial-of-service against the single-threaded, blocking design.")

    for s in held:
        try:
            s.close()
        except Exception:
            pass


if __name__ == "__main__":
    main()