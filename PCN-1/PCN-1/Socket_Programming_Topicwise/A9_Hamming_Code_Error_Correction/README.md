# A9 — Hamming Code Error Correction Using Sockets

## Exact exam question
Implement Hamming code to detect and correct a single-bit error, and exchange the codeword through sockets.

## Files
# See `C_VERSION_NOTES.md` for C compilation and run commands.
# See `C_VERSION_NOTES.md` for C compilation and run commands.

These files are shared with checksum and CRC. Restart the server before each test.

## Run on Windows PowerShell
Terminal 1:
```powershell
# See `C_VERSION_NOTES.md` for C compilation and run commands.
```
Terminal 2:
```powershell
# See `C_VERSION_NOTES.md` for C compilation and run commands.
```
Choose `3`, enter data bits such as `1011`, and enter a bit position to flip (for example, `3`). Enter `0` to leave the codeword unchanged.

## Expected output
The receiver reports a syndrome, corrected codeword and extracted data bits.

## How to verify
Enter a valid data bit sequence, flip one bit, and check that the receiver corrects it and extracts the original data.

## Viva questions
See `VIVA.md`.
## Metrics printed by the code
Client prints processing/round-trip time and reports the Hamming correction result; the demo corrects a single-bit error.
