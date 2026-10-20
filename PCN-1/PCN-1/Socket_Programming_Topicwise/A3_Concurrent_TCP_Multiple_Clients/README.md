# A3 — Concurrent TCP Server for Multiple Clients

## Exact exam question
Modify a TCP server so that it can handle multiple clients concurrently.

## Files
# See `C_VERSION_NOTES.md` for C compilation and run commands.
# See `C_VERSION_NOTES.md` for C compilation and run commands.

## Run on Windows PowerShell
Terminal 1:
```powershell
# See `C_VERSION_NOTES.md` for C compilation and run commands.
```
Then open Terminal 2 and Terminal 3 and run this in each:
```powershell
# See `C_VERSION_NOTES.md` for C compilation and run commands.
```
Enter a different message in each client.

## Expected output
Each client receives:
```text
Server: Echo: <your message>
```

## How to verify
Keep the server running and launch two clients from separate terminals. The server should report both connections and both messages.

## Performance metrics (if asked)
- Concurrent clients handled = number of clients that successfully connect and receive replies.
- Response time = reply receive time − request send time.
- Throughput = total successfully delivered payload bits / elapsed time.

## Viva questions
See `VIVA.md`.
## Metrics printed by the code
Each client prints response time and application throughput; server prints the cumulative number of clients handled. Run several client terminals to demonstrate concurrency.
