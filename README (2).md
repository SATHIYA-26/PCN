# PCN Lab — Question to File

Use the question keywords to choose the folder and source file.

## C Socket Programming
**Root:** `Socket_Programming_Topicwise`

| Question keywords | Folder | File(s) |
|---|---|---|
| TCP client-server | `A1_TCP_Client_Server` | `tcp_client.c`, `tcp_server.c` |
| UDP client-server | `A2_UDP_Client_Server` | `udp_client.c`, `udp_server.c` |
| Multiple/concurrent TCP clients | `A3_Concurrent_TCP_Multiple_Clients` | `concurrent_tcp_client.c`, `concurrent_tcp_server.c` |
| HTTP GET / download webpage | `A4_HTTP_Webpage_Download_Local_Server` | `http_client.c` |
| Reliable UDP / ACK / timeout / retransmission | `A5_Reliable_UDP_Timers_ACK_Retransmission` | `reliable_udp_client.c`, `reliable_udp_server.c` |
| DNS / hostname-to-IP resolution | `A6_DNS_Simulation` | `dns_client.c`, `dns_server.c` |
| Checksum error detection | `A7_Checksum_Error_Detection` | `error_client.c`, `error_server.c` |
| CRC / polynomial division / XOR | `A8_CRC_Error_Detection` | `error_client.c`, `error_server.c` |
| Hamming code / parity bits / single-bit correction | `A9_Hamming_Code_Error_Correction` | `error_client.c`, `error_server.c` |

## NS-3 Wired
**Root:** `NS3_Wired_6_Questions`

| Question keywords | Folder | File |
|---|---|---|
| Nodes, links, IP addresses, wired topology | `W1_Multi_Node_Topology` | `wired_simulation.cc` |
| TCP/UDP traffic, PCAP, packet tracing | `W2_TCP_UDP_Packet_Tracing` | `wired_simulation.cc` |
| Throughput, delay, packet loss, PDR | `W3_Performance_Metrics` | `wired_simulation.cc` |
| Vary bandwidth, link delay, queue size | `W4_Vary_Network_Parameters` | `wired_simulation.cc` |
| Routing, link failure, route recovery | `W5_Routing_Link_Failure` | `routing_failure.cc` |
| TCP congestion control, throughput, fairness | `W6_TCP_Congestion_Control` | `tcp_congestion.cc` |

## NS-3 Wireless
**Root:** `NS3_Wireless_Exam_Pack`

| Question keywords | Folder | File |
|---|---|---|
| Wi-Fi topology, stations/AP, traffic | `C1_WiFi_Topology_Traffic` | `wifi_topology_metrics.cc` |
| Moving nodes / mobility / performance | `C2_Mobility_Performance` | `wifi_topology_metrics.cc` |
| AODV / wireless routing changes | `C3_AODV_Routing_Changes` | `wifi_topology_metrics.cc` |
| Change wireless conditions / compare metrics | `C4_Wireless_Conditions_Sweep` | `wifi_topology_metrics.cc` |

## Networking Tools
| Question asks about | Tool |
|---|---|
| Active connections / listening ports | `netstat` |
| IP configuration | `ifconfig` / `ip addr` |
| Wireless interface | `iwconfig` |
| Connectivity / RTT / packet loss | `ping` |
| Routing table | `route` / `ip route` |
| Host/port scan | `nmap` |
| Terminal packet capture | `tcpdump` |
| DNS query | `dig` |
| Route to destination | `traceroute` |
| Continuous route/latency/loss | `mtr` |
| Capture and inspect packets | Wireshark |

## Wireshark Display Filters
- TCP: `tcp`
- UDP: `udp`
- DNS: `dns`
- HTTP: `http`
- ICMP: `icmp`
- Specific IP: `ip.addr == 192.168.1.10`
- TCP SYN: `tcp.flags.syn == 1`
- TCP port 80: `tcp.port == 80`

**Note:** DNS files in this ZIP are named `dns_client.c` and `dns_server.c`; there are no separate `dns_tcp_*` or `dns_udp_*` filenames. Verify the transport protocol in the code before selecting it. Check the selected source file against any extra requirement in the question.
