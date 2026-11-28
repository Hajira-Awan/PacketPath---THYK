# Computer Networks — Assignment 1

**Course:** Computer Networks
**Due date:** 23 November
**Group members:** Hajira Awan, Taina Ejaz, Yusra Ashfaq, Abdul Rehman
**Environment:** Windows, VS Code integrated terminal (PowerShell), g++ (MinGW-w64), tested with ncat, telnet (Microsoft Telnet Client), PuTTY, curl, and nc (Linux/Termux for cross-device test)

---

## Repository Structure

```
.
├── README.md
├── .gitignore
├── netlab/
│   ├── net.hpp
│   ├── daytime_server.cpp
│   ├── daytime_client.cpp
│   ├── daytime_server2.cpp
│   ├── daytime_client2.cpp
│   ├── echo_server.cpp
│   ├── echo_client.cpp
│   ├── web_server.cpp
│   ├── frame_server.cpp
│   ├── frame_client.cpp
│   ├── thread_server.cpp
│   ├── select_server.cpp
│   ├── chat_server.cpp
│   ├── chat_client.cpp
│   ├── crash_daytime.py
│   ├── crash_web.py
│   └── www/
│       ├── index.html
│       ├── about.html
│       └── style.css
└── screenshots/
    ├── 01_Tutorial1_Daytime_Server/
    ├── 02_Tutorial2_Daytime_Client/
    ├── 03_Tutorial3_Portable_Socket_Layer/
    ├── 04_Tutorial4_Echo_Server_Client/
    ├── 05_Tutorial5_Web_Server/
    ├── 06_Tutorial6_Message_Framing/
    ├── 07_Tutorial8_Thread_Server/
    ├── 08_Tutorial9_Select_Server/
    ├── 09_Assignment_Chat_System/
    ├── 10_Bonus_Crash_Attempt/
    └── 11_GitHub_Push/
```

> **Note:** All build/run commands below assume you're inside the `netlab/` folder (`cd netlab` first).

---

## Part 1: Tutorials (4 marks)

### Tutorial 1 — Daytime Server

**Build:**
```
g++ -std=c++17 -Wall -Wextra -o daytime_server daytime_server.cpp
```
**Run:**
```
./daytime_server 1313
```
**Tested with:** ncat, telnet, PuTTY, `netstat -an | findstr LISTENING`, and a second device on the same network (phone via Termux) connecting over LAN IP.

| Test | Screenshot |
|---|---|
| Build + run, ncat connects, shows date/time | <img src="screenshots/01_Tutorial1_Daytime_Server/ss_01.jpeg" width="420"> |
| Multiple ncat connections, each gets a fresh timestamp | <img src="screenshots/01_Tutorial1_Daytime_Server/ss_02.jpeg" width="420"> |
| PuTTY raw TCP configuration | <img src="screenshots/01_Tutorial1_Daytime_Server/ss_04.jpeg" width="420"> |
| PuTTY showing the server's response | <img src="screenshots/01_Tutorial1_Daytime_Server/ss_05.jpeg" width="420"> |
| Cross-device test: phone (Termux) connecting over LAN via `nc <server-ip> 1313` | <img src="screenshots/01_Tutorial1_Daytime_Server/ss_06.jpeg" width="420"> <img src="screenshots/01_Tutorial1_Daytime_Server/ss_07.jpeg" width="420"> |
| `netstat` confirming the listening socket on port 1313 | <img src="screenshots/01_Tutorial1_Daytime_Server/ss_08.jpeg" width="420"> |

**Observation:** Server accepts one client at a time, sends the current date/time, then closes the connection as per the daytime protocol (RFC 867). Each new connection gets a fresh client port number chosen by the OS.

---

### Tutorial 2 — Daytime Client

**Build:**
```
g++ -std=c++17 -Wall -Wextra -o daytime_client daytime_client.cpp -lws2_32
```
**Run:**
```
./daytime_client 127.0.0.1 1313
./daytime_client localhost 1313
```

| Test | Screenshot |
|---|---|
| Client build and first run | <img src="screenshots/02_Tutorial2_Daytime_Client/ss_01.jpeg" width="420"> |
| Client with `localhost` and with `127.0.0.1` | <img src="screenshots/02_Tutorial2_Daytime_Client/ss_02.jpeg" width="420"> <img src="screenshots/02_Tutorial2_Daytime_Client/ss_03.jpeg" width="420"> |
| Server stopped → client shows `Cannot connect ... (error 10061)` | <img src="screenshots/02_Tutorial2_Daytime_Client/ss_04.jpeg" width="420"> |

**Observation:** Client reads in a loop until `recv()` returns 0 (server closed the connection). The Windows-specific error code (10061 = `WSAECONNREFUSED`) confirms error-number differences between platforms as explained in the manual.

---

### Tutorial 3 — Portable Socket Layer (`net.hpp`) + Daytime Server/Client v2

**Build:**
```
g++ -std=c++17 -Wall -Wextra -o daytime_server2 daytime_server2.cpp -lws2_32
g++ -std=c++17 -Wall -Wextra -o daytime_client2 daytime_client2.cpp -lws2_32
```

| Test | Screenshot |
|---|---|
| PuTTY test against `daytime_server2` | <img src="screenshots/03_Tutorial3_Portable_Socket_Layer/ss_01.jpeg" width="420"> |
| Build and run `daytime_server2` | <img src="screenshots/03_Tutorial3_Portable_Socket_Layer/ss_02.jpeg" width="420"> <img src="screenshots/03_Tutorial3_Portable_Socket_Layer/ss_03.jpeg" width="420"> |
| Run `daytime_client2` | <img src="screenshots/03_Tutorial3_Portable_Socket_Layer/ss_04.jpeg" width="420"> |
| Rebuild both after edits | <img src="screenshots/03_Tutorial3_Portable_Socket_Layer/ss_05.jpeg" width="420"> |
| telnet test | <img src="screenshots/03_Tutorial3_Portable_Socket_Layer/ss_06.jpeg" width="420"> |
| `daytime_client2` shows `connect: Connection refused` once server is stopped | <img src="screenshots/03_Tutorial3_Portable_Socket_Layer/ss_07.jpeg" width="420"> |
| `ncat --version` and `ncat` test | <img src="screenshots/03_Tutorial3_Portable_Socket_Layer/ss_08.jpeg" width="420"> <img src="screenshots/03_Tutorial3_Portable_Socket_Layer/ss_09.jpeg" width="420"> |
| PuTTY configuration for `daytime_server2` | <img src="screenshots/03_Tutorial3_Portable_Socket_Layer/ss_10.jpeg" width="420"> |

**Observation:** `net.hpp` removes all the `#ifdef _WIN32` blocks from the application code. `net::Runtime` handles `WSAStartup`/`WSACleanup`, and `net::Socket` (RAII) auto-closes the handle. Error messages are now human-readable text (e.g. "connect: Connection refused") instead of raw platform-specific codes.

---

### Tutorial 4 — TCP Echo Server and Client

**Build:**
```
g++ -std=c++17 -Wall -Wextra -o echo_server echo_server.cpp -lws2_32
g++ -std=c++17 -Wall -Wextra -o echo_client echo_client.cpp -lws2_32
```

| Test | Screenshot |
|---|---|
| Build echo_server / echo_client | <img src="screenshots/04_Tutorial4_Echo_Server_Client/ss_01.jpeg" width="420"> |
| Client sends text, receives the same text echoed back | <img src="screenshots/04_Tutorial4_Echo_Server_Client/ss_02.jpeg" width="420"> <img src="screenshots/04_Tutorial4_Echo_Server_Client/ss_03.jpeg" width="420"> <img src="screenshots/04_Tutorial4_Echo_Server_Client/ss_04.jpeg" width="420"> |
| Client disconnects (Ctrl+Z), server shows `Client disconnected` | <img src="screenshots/04_Tutorial4_Echo_Server_Client/ss_05.jpeg" width="420"> <img src="screenshots/04_Tutorial4_Echo_Server_Client/ss_06.jpeg" width="420"> |
| ncat test | <img src="screenshots/04_Tutorial4_Echo_Server_Client/ss_07.jpeg" width="420"> <img src="screenshots/04_Tutorial4_Echo_Server_Client/ss_08.jpeg" width="420"> |
| telnet test (Microsoft Telnet Client) | <img src="screenshots/04_Tutorial4_Echo_Server_Client/ss_09.jpeg" width="420"> <img src="screenshots/04_Tutorial4_Echo_Server_Client/ss_10.jpeg" width="420"> <img src="screenshots/04_Tutorial4_Echo_Server_Client/ss_11.jpeg" width="420"> <img src="screenshots/04_Tutorial4_Echo_Server_Client/ss_12.jpeg" width="420"> |
| PuTTY test | <img src="screenshots/04_Tutorial4_Echo_Server_Client/ss_13.jpeg" width="420"> |

**Observation:** Server reads whatever bytes arrive and immediately writes them back with `send_all()`. Tested successfully with three independent tools (ncat, telnet, PuTTY) in addition to the custom client — confirming the protocol is tool-agnostic, plain TCP.

---

### Tutorial 5 — Static Web Server (HTTP/1.0)

**Build:**
```
g++ -std=c++17 -Wall -Wextra -o web_server web_server.cpp -lws2_32
```
**Run (from the folder that holds `www`):**
```
./web_server 8080 www
```

| Test | Screenshot |
|---|---|
| Browser: `index.html` with link to `about.html` | <img src="screenshots/05_Tutorial5_Web_Server/ss_01.jpeg" width="420"> <img src="screenshots/05_Tutorial5_Web_Server/ss_10.jpeg" width="420"> |
| Browser: `about.html` navigation | <img src="screenshots/05_Tutorial5_Web_Server/ss_11.jpeg" width="420"> |
| Browser: `style.css` served with correct `Content-Type` | <img src="screenshots/05_Tutorial5_Web_Server/ss_14.jpeg" width="420"> |
| Server log: `/favicon.ico → 404`, idle client → `400` | <img src="screenshots/05_Tutorial5_Web_Server/ss_02.jpeg" width="420"> |
| `curl -i` → 404 Not Found for missing file | <img src="screenshots/05_Tutorial5_Web_Server/ss_05.jpeg" width="420"> |
| `curl -I` → HEAD request, headers only | <img src="screenshots/05_Tutorial5_Web_Server/ss_06.jpeg" width="420"> |
| `curl -i -X POST` → 405 Method Not Allowed | <img src="screenshots/05_Tutorial5_Web_Server/ss_07.jpeg" width="420"> |
| `curl --path-as-is -i ".../../web_server.cpp"` → 400 Bad Request (path traversal blocked) | <img src="screenshots/05_Tutorial5_Web_Server/ss_08.jpeg" width="420"> |
| `curl --path-as-is -i ".../%2e%2e/web_server.cpp"` → 400 Bad Request (decoded first, then blocked) | <img src="screenshots/05_Tutorial5_Web_Server/ss_16.jpeg" width="420"> |
| `curl -v` showing the full raw HTTP request/response | <img src="screenshots/05_Tutorial5_Web_Server/ss_18.jpeg" width="420"> |
| Raw request typed via `ncat` (`GET / HTTP/1.0`) | <img src="screenshots/05_Tutorial5_Web_Server/ss_17.jpeg" width="420"> |
| Query string handled (`/?name=test` → 200) | <img src="screenshots/05_Tutorial5_Web_Server/ss_15.jpeg" width="420"> |
| Full consolidated server access log (every test in one view) | <img src="screenshots/05_Tutorial5_Web_Server/ss_13.jpeg" width="420"> |

**Observation:** The server correctly serves static files with the right MIME type, returns 404 for missing files, 405 for unsupported methods, and 400 for any request that tries path traversal — both with a literal `../` and with the URL-encoded `%2e%2e` form (decoded first, then rejected, as required by Section 7.2.3 of the manual). An idle client that sends nothing is dropped after the 5-second `select()` timeout with a 400 response, confirming the server does not hang indefinitely on a non-responsive connection.

---

### Tutorial 6 — Message Framing (Length-Prefixed Messages)

**Build:**
```
g++ -std=c++17 -Wall -Wextra -o frame_server frame_server.cpp -lws2_32
g++ -std=c++17 -Wall -Wextra -o frame_client frame_client.cpp -lws2_32
```
**Run:**
```
./frame_server 5001
./frame_client 127.0.0.1 5001 one two three
```

| Test | Screenshot |
|---|---|
| Build frame_server / frame_client | <img src="screenshots/06_Tutorial6_Message_Framing/ss_01.jpeg" width="420"> |
| Client sends `one two three`, server replies in upper case | <img src="screenshots/06_Tutorial6_Message_Framing/ss_02.jpeg" width="420"> |

**Observation:** Output confirms `Reply: ONE`, `Reply: TWO`, `Reply: THREE` — each argument sent as a separate length-prefixed message is received intact by the server as a distinct message, demonstrating that `send_msg`/`recv_msg` correctly solve the TCP byte-stream framing problem (no merging/splitting of messages).

---

### Tutorial 7 & 8 — One Thread per Client

**Build:**
```
g++ -std=c++17 -Wall -Wextra -pthread -o thread_server thread_server.cpp -lws2_32
```

| Test | Screenshot |
|---|---|
| Two `echo_client` instances connect simultaneously, `Active clients: 1` → `2` | <img src="screenshots/07_Tutorial8_Thread_Server/ss_01.jpeg" width="420"> |
| Both clients echoed independently and concurrently | <img src="screenshots/07_Tutorial8_Thread_Server/ss_02.jpeg" width="420"> |
| Clients disconnect, `Active clients: 1` → `0` | <img src="screenshots/07_Tutorial8_Thread_Server/ss_04.jpeg" width="420"> |

**Observation:** A new `std::thread` is spawned per accepted connection, each with its own `Socket` moved into it. An atomic counter tracks `Active clients` correctly as clients join and leave, with no clients blocking one another.

---

### Tutorial 9 — `select()` for Many Clients

**Build:**
```
g++ -std=c++17 -Wall -Wextra -o select_server select_server.cpp -lws2_32
```

| Test | Screenshot |
|---|---|
| Three `ncat` clients connect at once, `Active clients: 1 → 2 → 3` | <img src="screenshots/08_Tutorial9_Select_Server/ss_01.jpeg" width="420"> |
| All three echoed independently in the same single-threaded server | <img src="screenshots/08_Tutorial9_Select_Server/ss_02.jpeg" width="420"> |
| All three disconnect, counter goes back down to `0` | <img src="screenshots/08_Tutorial9_Select_Server/ss_03.jpeg" width="420"> |

**Observation:** A single thread using `select()` serves all three clients concurrently with no threads or locks, confirming the event-driven design from Section 10 of the manual.

---

## Part 2: Multi-User Chat System (5 marks)

**Build:**
```
g++ -std=c++17 -Wall -Wextra -o chat_server chat_server.cpp -lws2_32
g++ -std=c++17 -Wall -Wextra -o chat_client chat_client.cpp -lws2_32
```
**Run:**
```
./chat_server 6000
./chat_client 127.0.0.1 6000
```

### Requirements checklist

| Req | Description | Status | Evidence |
|---|---|---|---|
| R1 | Server accepts up to 60 clients | ✅ (by design, `select()`-based; not individually stress-tested to 60) | — |
| R2 | One thread + `select()` | ✅ | <img src="screenshots/09_Assignment_Chat_System/ss_01.jpeg" width="420"> |
| R3 | Default names `user1`, `user2`, ... | ✅ | <img src="screenshots/09_Assignment_Chat_System/ss_01.jpeg" width="420"> |
| R4 | Chat line broadcast with sender's name | ✅ | <img src="screenshots/09_Assignment_Chat_System/ss_02.jpeg" width="420"> |
| R5 | `/nick NAME`, duplicate name rejected | ✅ | <img src="screenshots/09_Assignment_Chat_System/ss_02.jpeg" width="420"> |
| R6 | `/who` lists connected users | ✅ | <img src="screenshots/09_Assignment_Chat_System/ss_02.jpeg" width="420"> |
| R7 | `/quit` disconnects cleanly | ✅ | <img src="screenshots/09_Assignment_Chat_System/ss_02.jpeg" width="420"> |
| R8 | Join/leave broadcast to others | ✅ | <img src="screenshots/09_Assignment_Chat_System/ss_01.jpeg" width="420"> <img src="screenshots/09_Assignment_Chat_System/ss_02.jpeg" width="420"> |
| R9 | Partial/merged lines handled correctly | ✅ | <img src="screenshots/09_Assignment_Chat_System/ss_03.jpeg" width="420"> |
| R10 | Client shows incoming text while typing | ✅ (text interleaving visible as per the manual's own noted limitation, Section 11.8) | <img src="screenshots/09_Assignment_Chat_System/ss_04.jpeg" width="420"> |
| R11 | Only `net.hpp` used, no `#ifdef` in app files | ✅ (verify in source — `chat_server.cpp`/`chat_client.cpp` only `#include "net.hpp"`) | <img src="screenshots/09_Assignment_Chat_System/ss_05.jpeg" width="420"> |

### Test walkthrough

1. Two clients connect → get `user1`, `user2` by default.
2. `user1` runs `/nick Anna`, broadcast: `user1 is now Anna`.
3. `Anna` sends `Hello user2` → `user2` sees `Anna: Hello user2`.
4. `user2` runs `/who` → `Users: Anna user2`.
5. `user2` tries `/nick Anna` → `Error: that name is in use`.
6. `user2` runs `/quit` → disconnects; `Anna`'s terminal shows `user2 left`.

**Split-line stress test (R9):** using Git Bash / MINGW64 on the server machine:
```bash
nc 127.0.0.1 6000
```
and in a second terminal:
```bash
(printf 'Hel'; sleep 2; printf 'lo\n') | nc 127.0.0.1 6000
```
The first terminal shows nothing for 2 seconds after the new user joins, then shows `user3: Hello` — confirming the server buffers partial data (`Hel`) until the rest of the line (`lo\n`) arrives, exactly as described in Section 11.6 of the manual. See `screenshots/09_Assignment_Chat_System/ss_03.jpeg`.

<img src="screenshots/09_Assignment_Chat_System/ss_03.jpeg" width="420">

---

## Part 3: GitHub Upload (1 mark)

Code and report pushed to a public repository:

**Repo:** `https://github.com/Hajira-Awan/PacketPath---THYK`

See `screenshots/11_GitHub_Push/ss_01.jpeg` for the push confirmation.

<img src="screenshots/11_GitHub_Push/ss_01.jpeg" width="420">

---

## Bonus — Crash Attempts

Two scripts were written and tested against the Tutorial 1 and Tutorial 5 servers.

### 1. `crash_daytime.py` — flood the daytime server

Opens 200 simultaneous connections to the daytime server and holds them open, then checks whether a fresh, well-behaved client can still be served.

**Result:** The daytime server **survived** the attempt — see `screenshots/10_Bonus_Crash_Attempt/ss_01.jpeg`. Because Tutorial 1's daytime server sends its response and closes the connection immediately per client (it does not hold state or wait on slow clients), a connection flood alone was not enough to deny service.

<img src="screenshots/10_Bonus_Crash_Attempt/ss_01.jpeg" width="420">

### 2. `crash_web.py` — target the web server's request parser

Sends 12 malformed and edge-case raw HTTP requests to the web server one at a time (bad `%` encoding, empty request line, null byte in the path, negative or non-numeric `Content-Length`, a header line with no colon, a 9000-byte header over the 8192 cap, and so on). After each request it checks that the server is still alive and answering. The idea: one bad request that makes the server throw an uncaught exception would take the whole process down.

**Result:** Also did **not** crash or deny service to the web server. Every malformed request got a proper status code (400, 404 or 405, or a normal 200) and the server stayed alive after each one.

<img src="screenshots/10_Bonus_Crash_Attempt/ss_02.jpeg" width="600">

**Conclusion:** Neither script succeeded in crashing the respective server. Both Tutorial 1 and Tutorial 5 servers handle a connection flood and malformed requests gracefully within the scope of what was tested — this is a reasonable (if unsuccessful) attempt at the bonus requirement, and the scripts themselves are included in the repository (`netlab/crash_daytime.py`, `netlab/crash_web.py`) as evidence of the approach taken.

---

## Known Limitations

- The web server (Tutorial 5, single-threaded version) serves one client at a time; a slow or idle client delays everyone else until the 5-second timeout.
- The chat server's `send_all()` is blocking — a client that never reads can stall the whole server.
- No encryption or authentication anywhere in this project (plain TCP only), as intended for a learning exercise.
- IPv4 only, as per the manual's design choice.
- The chat client shows incoming text and the user's own typed input mixed together on the same line (Section 11.8 of the manual notes this is an accepted limitation of the two-thread design, not a bug).
