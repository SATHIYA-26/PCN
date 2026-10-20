# PCN-1 C Version

All socket-programming `.py` files have been replaced with C source files (`.c`).
The NS-3 simulation files remain `.cc`, since NS-3 programs are written in C++.

## Compile and run socket programs

Use Linux/Ubuntu or WSL with GCC installed. Open a terminal in the relevant experiment folder.

For a TCP client/server:
```bash
gcc tcp_server.c -o server
gcc tcp_client.c -o client
./server
```
Open a second terminal in the same folder and run `./client`.

For a UDP client/server, compile `udp_server.c` and `udp_client.c` to `server` and `client`; run the server first, then the client.

For concurrent TCP, compile `concurrent_tcp_server.c` and `concurrent_tcp_client.c`. The server may need pthread support:
```bash
gcc concurrent_tcp_server.c -o server -pthread
gcc concurrent_tcp_client.c -o client
```
Run the server, then launch clients in separate terminals.

For reliable UDP, DNS, HTTP, checksum/CRC/Hamming, compile the matching `.c` files:
```bash
gcc filename.c -o program
./program
```
For client/server experiments, compile both sides and run the server first. HTTP download expects a local HTTP server listening on `127.0.0.1:8000` and a page at `/index.html`.

These are POSIX socket programs. They are intended for Linux/Ubuntu/WSL, not native Windows MSVC. NS-3 `.cc` programs must be run through your NS-3 installation.
