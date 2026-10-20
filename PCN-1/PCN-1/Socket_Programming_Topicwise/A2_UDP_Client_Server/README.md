# A2 — UDP Client-Server

## Exact exam question
Develop UDP client-server applications. The client sends a datagram and receives a reply.

## Files
# See `C_VERSION_NOTES.md` for C compilation and run commands.
# See `C_VERSION_NOTES.md` for C compilation and run commands.

## Run on Windows PowerShell
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
Server: Hello from UDP server
```

## How to verify
The server prints the datagram and sender address; the client prints the reply. If no server is running, the client reports a timeout.

## Performance metrics (if asked)
- RTT = reply receive time − datagram send time.
- PDR = received datagrams / sent datagrams × 100%.
- Loss rate = lost datagrams / sent datagrams × 100%.
- Throughput = received payload bits / elapsed seconds.

## Viva questions
See `VIVA.md`.
## Metrics printed by the code
UDP client sends 10 echo packets and prints average RTT, PDR, packet-loss rate and application throughput.
