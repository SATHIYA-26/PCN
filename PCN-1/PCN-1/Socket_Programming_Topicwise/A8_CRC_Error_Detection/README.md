# A8 — CRC Error Detection Using Sockets

## Exact exam question
Implement CRC-based error detection and exchange the CRC codeword through sockets.

## Files
# See `C_VERSION_NOTES.md` for C compilation and run commands.
# See `C_VERSION_NOTES.md` for C compilation and run commands.

These files are shared with checksum and Hamming. Restart the server before each test.

## Run on Windows PowerShell
Terminal 1:
```powershell
# See `C_VERSION_NOTES.md` for C compilation and run commands.
```
Terminal 2:
```powershell
# See `C_VERSION_NOTES.md` for C compilation and run commands.
```
Choose `2`; enter data bits such as `1101` and generator bits such as `1011`.

## Expected output
The client displays the CRC codeword. The receiver checks the remainder and reports `NO ERROR DETECTED` when the remainder is all zero.

## How to verify
For an error test, flip one bit in the transmitted codeword before the receiver checks it. Many bit errors will produce a nonzero remainder.

## Viva questions
See `VIVA.md`.
## Metrics printed by the code
Client prints processing/round-trip time and whether CRC validation succeeded or corruption was detected.
