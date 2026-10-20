# A1 — TCP Client-Server

## Exact exam question
Develop a TCP client-server application. The client sends a message to the server, and the server replies.

## Files
# See `C_VERSION_NOTES.md` for C compilation and run commands.
# See `C_VERSION_NOTES.md` for C compilation and run commands.

## Run on Windows PowerShell
Open two terminals in this folder.

Terminal 1:
```powershell
# See `C_VERSION_NOTES.md` for C compilation and run commands.
```
Terminal 2:
```powershell
# See `C_VERSION_NOTES.md` for C compilation and run commands.
```

## Expected output
Client:
```text
Server: Hello from TCP server
```
Server shows the connection and the client's message.

## How to verify
The client prints the server reply and the server prints the client's message.

## Performance metrics (if asked)
- RTT = response receive time − request send time.
- Throughput (bit/s) = successfully received payload bits / elapsed seconds.
- For a single exchange, explain the measurement method; do not invent a value.

## Viva questions
See `VIVA.md`.
## Metrics printed by the code
TCP client prints application-level RTT and reply throughput.
