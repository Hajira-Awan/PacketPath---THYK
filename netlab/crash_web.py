"""
crash_web.py - bonus: probe web_server.cpp (Tutorial 5/8) for a real weakness.

Target weakness: exception safety. The whole server loop in main() is
usually wrapped in ONE try/catch. If a single client's request makes any
function the server calls THROW an uncaught C++ exception (for example
std::string::substr with a bad position, or std::stoi on bad input), that
exception is not caught per-connection -- it propagates out of main()'s
loop entirely, and the WHOLE SERVER PROCESS EXITS. One bad request from
one client then denies service to every other client.

This script sends a batch of unusual, malformed raw HTTP requests one at
a time, and after each one checks whether the server process is still
alive and answering. The one that makes it stop answering is your bonus
finding -- note which request it was.

Run this on the SAME machine as the server, against your OWN server only.
Usage:
    python3 crash_web.py 127.0.0.1 8080
"""
import socket
import sys
import time

HOST = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
PORT = int(sys.argv[2]) if len(sys.argv) > 2 else 8080

# Each entry: (label, raw request bytes sent exactly as written, no extra \r\n added)
CASES = [
    ("lone percent at end of path",
     b"GET /% HTTP/1.1\r\nHost: x\r\n\r\n"),
    ("percent with only one hex digit",
     b"GET /%4 HTTP/1.1\r\nHost: x\r\n\r\n"),
    ("percent with invalid hex digits",
     b"GET /%zz HTTP/1.1\r\nHost: x\r\n\r\n"),
    ("empty request line",
     b"\r\n\r\n"),
    ("request line with no target",
     b"GET\r\n\r\n"),
    ("method only, no version",
     b"GET /\r\n\r\n"),
    ("huge number of query params",
     b"GET /?" + b"a=1&" * 2000 + b" HTTP/1.1\r\nHost: x\r\n\r\n"),
    ("negative-looking content-length",
     b"POST / HTTP/1.1\r\nHost: x\r\nContent-Length: -1\r\n\r\n"),
    ("non-numeric content-length",
     b"POST / HTTP/1.1\r\nHost: x\r\nContent-Length: abc\r\n\r\n"),
    ("header line with no colon",
     b"GET / HTTP/1.1\r\nHost\r\n\r\n"),
    ("extremely long single header (9000 bytes, over the 8192 cap)",
     b"GET / HTTP/1.1\r\nHost: " + b"a" * 9000 + b"\r\n\r\n"),
    ("null byte in the path",
     b"GET /\x00etc/passwd HTTP/1.1\r\nHost: x\r\n\r\n"),
]


def send_raw(data, read_timeout=3):
    with socket.create_connection((HOST, PORT), timeout=read_timeout) as s:
        s.sendall(data)
        s.settimeout(read_timeout)
        try:
            reply = s.recv(300)
        except socket.timeout:
            reply = b"<no reply / timed out>"
        return reply


def server_alive():
    try:
        r = send_raw(b"GET / HTTP/1.0\r\n\r\n", read_timeout=2)
        return True, r[:60]
    except Exception as e:
        return False, str(e)


def main():
    alive, info = server_alive()
    print(f"Before any test: server alive = {alive} ({info!r})")
    if not alive:
        print("Start web_server.exe first, then run this script again.")
        return

    for label, raw in CASES:
        print(f"\n--- sending: {label} ---")
        try:
            reply = send_raw(raw)
            print(f"  reply: {reply[:80]!r}")
        except Exception as e:
            print(f"  connection error while sending: {e}")

        time.sleep(0.3)
        alive, info = server_alive()
        print(f"  server still alive? {alive}  ({info!r})")
        if not alive:
            print(f"\n*** FOUND IT: the request '{label}' killed the server. ***")
            print("That is your bonus finding. Screenshot this terminal output,")
            print("and the server's own terminal, which should show it exited")
            print("or printed an uncaught exception / crash message.")
            return

    print("\nNone of these cases killed the server in this run.")
    print("That is a fine result too: it means this server handles malformed")
    print("requests safely. You can report that you tested these cases and")
    print("the server stayed up, which is itself worth a line in the report.")


if __name__ == "__main__":
    main()