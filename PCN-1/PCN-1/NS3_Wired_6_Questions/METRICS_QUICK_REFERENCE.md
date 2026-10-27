# Computer Networks Viva Preparation Guide

## Overview

This repository contains Computer Networks viva notes covering fundamental networking concepts, performance metrics, protocols, routing, network devices and NS-3 wired network simulation.

## 1. Network Performance Metrics

| Concept | Meaning | Unit |
|---|---|---|
| Bandwidth | Maximum configured link transmission capacity | bps, Mbps |
| Throughput | Data successfully delivered per second | bps, Mbps |
| Goodput | Useful application data delivered per second | bps, Mbps |
| Latency | Time taken for data to travel from source to destination | ms |
| Transmission Delay | Time required to transmit packet bits onto a link | s, ms |
| Propagation Delay | Time taken for a signal to travel through a medium | s, ms |
| Queueing Delay | Time spent waiting in a queue | s, ms |
| Processing Delay | Time taken to process a packet | s, ms |
| Packet Loss | Packets that fail to reach the destination | Packets, % |
| Jitter | Variation in packet delay | ms |
| RTT | Round-trip time for a packet and its response | ms |
| BDP | Amount of data that can be in transit over an RTT | bits, bytes |

### Important Formulas

**Transmission delay**

`Transmission Delay = Packet Size in Bits / Bandwidth in bps`

**Propagation delay**

`Propagation Delay = Distance / Propagation Speed`

**Total end-to-end delay**

`Total Delay = Transmission + Propagation + Queueing + Processing`

**Throughput**

`Throughput = Received Bits / Measurement Time`

**Packet loss percentage**

`Loss (%) = (Packets Sent - Packets Received) / Packets Sent × 100`

**Average end-to-end delay**

`Average Delay = Total Recorded Packet Delay / Packets Received`

**Round-trip time**

`RTT = Forward Path Delay + Return Path Delay`

**Bandwidth-delay product**

`BDP = Bandwidth × RTT`

**Simple delay variation**

`Jitter = Absolute Difference Between Successive Packet Delays`

Note: Actual jitter definitions vary by protocol and measurement tool. FlowMonitor reports accumulated jitter statistics.

## 2. Types of Computer Networks

- PAN — Personal Area Network
- LAN — Local Area Network
- WLAN — Wireless Local Area Network
- MAN — Metropolitan Area Network
- WAN — Wide Area Network

## 3. Network Topologies

- Bus topology
- Star topology
- Ring topology
- Mesh topology
- Tree topology
- Point-to-point topology

Topology defines how nodes and links are interconnected.

## 4. OSI and TCP/IP Models

### OSI Model

| Layer | Name | Examples or Responsibilities |
|---|---|---|
| 7 | Application | HTTP, DNS, SMTP |
| 6 | Presentation | Data representation, encoding, encryption functions |
| 5 | Session | Session management |
| 4 | Transport | TCP, UDP |
| 3 | Network | IP addressing and routing |
| 2 | Data Link | Ethernet frames, MAC addresses |
| 1 | Physical | Signals, cables, transmission media |

Memory trick: All People Seem To Need Data Processing.

### TCP/IP Model

1. Application
2. Transport
3. Internet
4. Network Access

## 5. Network Devices

| Device | Main Function |
|---|---|
| Hub | Repeats signals to connected ports |
| Switch | Forwards frames using MAC addresses |
| Router | Forwards packets between IP networks |
| Bridge | Connects LAN segments |
| Gateway | Connects networks or systems and may translate protocols |
| Modem | Modulates/demodulates signals or performs related access-link functions |

## 6. TCP vs UDP

| Feature | TCP | UDP |
|---|---|---|
| Connection | Connection-oriented | Connectionless |
| Reliability | Reliable delivery mechanisms | No built-in delivery guarantee |
| Ordering | Ordered byte stream | No ordering guarantee |
| Acknowledgments | Yes | No built-in ACK mechanism |
| Retransmissions | Supported | Not built in |
| Common uses | File transfer, web traffic, SSH | DNS queries, real-time media, gaming |

### TCP Three-Way Handshake

1. SYN
2. SYN-ACK
3. ACK

### Important TCP Concepts

- Sequence numbers
- Acknowledgments
- Retransmissions
- Sliding window
- Flow control
- Congestion control
- TCP connection termination

**Flow control** protects the receiver from excessive incoming data.

**Congestion control** adjusts sending behavior according to network conditions.

## 7. IP Addressing and Network Services

- IPv4: 32-bit IP addresses.
- IPv6: 128-bit IP addresses.
- MAC address: Link-layer address used for local network delivery.
- Subnet mask: Identifies the network prefix and host portion of an IPv4 address.
- Default gateway: Router used to reach destinations outside the local subnet.
- NAT: Translates network address information.
- ARP: Resolves IPv4 addresses to link-layer addresses on a local network.
- DNS: Resolves domain names and provides naming information.
- DHCP: Automatically configures IP networking.
- ICMP: Network-control and diagnostic messages.
- TTL: IPv4 packet hop limit.

## 8. Application-Layer Protocols

| Protocol | Purpose | Common Port |
|---|---|---|
| HTTP | Web communication | TCP 80 |
| HTTPS | Secure web communication | TCP 443 |
| FTP | File transfer | TCP 21 for control |
| SSH | Secure remote access | TCP 22 |
| DNS | Domain name resolution | Port 53 |
| DHCP | Dynamic host configuration | UDP 67/68 for IPv4 |
| SMTP | Email transfer | TCP 25 for server-to-server transfer |
| POP3 | Email retrieval | TCP 110 |
| IMAP | Email access and synchronization | TCP 143 |
| Telnet | Unencrypted remote terminal | TCP 23 |
| NTP | Time synchronization | UDP 123 |

Ports listed are conventional defaults.

### HTTP Request Components

- Request line
- Headers
- Optional message body

### HTTP Response Components

- Status line
- Headers
- Optional message body

Common status codes: 200, 301, 302, 400, 401, 403, 404 and 500.

## 9. Routing and Congestion

- Routing: Determines how packets reach destination networks.
- Routing table: Stores destination and forwarding information.
- Static routing: Routes configured manually.
- Dynamic routing: Routes learned through routing protocols.
- Hop: One forwarding step between network-layer devices.
- MTU: Maximum transmission unit supported by a link.
- Congestion: Network demand exceeds available resources.
- Bottleneck: Resource that limits end-to-end performance.
- QoS: Mechanisms for managing traffic and performance requirements.
- Queue: Temporarily stores packets awaiting transmission.
- DropTail: Drops arriving packets when the queue is full.

### Effects of Changing Network Parameters

- Increasing bandwidth can improve throughput.
- Increasing propagation delay increases the propagation component of end-to-end delay.
- Increasing queue size may reduce drops but increase queueing delay.
- Increasing offered UDP traffic can increase congestion.
- A congested bottleneck may increase packet loss and delay.

## 10. Error Detection and Correction

| Technique | Purpose |
|---|---|
| Parity | Detects certain bit errors |
| Checksum | Helps detect data corruption |
| CRC | Detects errors using polynomial division |
| Hamming code | Detects and, in suitable forms, corrects bit errors |

For minimum Hamming distance `d_min`:

- Maximum guaranteed detectable errors: `d_min - 1`
- Maximum guaranteed correctable errors: `floor((d_min - 1) / 2)`

## 11. NS-3 Wired Simulation

### Purpose

NS-3 is a discrete-event network simulator used to model network topologies, protocols and traffic, and measure network performance.

### Experiment Workflow

1. Create nodes using `NodeContainer`.
2. Create wired links using `PointToPointHelper`.
3. Configure data rates, channel delays and queue sizes.
4. Install networking protocols using `InternetStackHelper`.
5. Assign IP addresses using `Ipv4AddressHelper`.
6. Configure routes using `Ipv4GlobalRoutingHelper`.
7. Configure TCP/UDP sender and receiver applications.
8. Enable FlowMonitor and PCAP tracing.
9. Run the simulation using `Simulator::Run()`.
10. Collect statistics and save output files.
11. Clean up using `Simulator::Destroy()`.

### Important NS-3 Classes and Methods

| Component | Purpose |
|---|---|
| `NodeContainer` | Creates and stores nodes |
| `PointToPointHelper` | Configures point-to-point links |
| `NetDeviceContainer` | Stores installed network devices |
| `InternetStackHelper` | Installs networking protocols |
| `Ipv4AddressHelper` | Assigns IPv4 addresses |
| `Ipv4GlobalRoutingHelper` | Provides global routing support |
| `OnOffHelper` | Generates application traffic |
| `PacketSinkHelper` | Receives application traffic |
| `FlowMonitorHelper` | Installs and manages flow monitoring |
| `EnablePcap()` | Enables packet capture |
| `Simulator::Run()` | Executes the simulation |
| `Simulator::Destroy()` | Cleans up simulator state |

### W1 — Topology, IP Addressing and Routing

Focus on node creation, point-to-point links, IP subnets and routing tables.

### W2 — TCP/UDP Traffic and Packet Tracing

Focus on sender/receiver applications, traffic generation, PCAP captures and Wireshark.

### W3 — Performance Analysis

Focus on packet counts, throughput, delay, packet loss, jitter and FlowMonitor reports.

### FlowMonitor vs PCAP

- FlowMonitor collects flow-level statistics.
- PCAP captures packets for detailed inspection.
- TCP retransmissions are not directly counted by a dedicated FlowMonitor retransmission counter; inspect TCP traces or PCAP files.

## 12. Common Viva Questions

1. What is bandwidth?
2. What is the difference between bandwidth and throughput?
3. What is latency?
4. What are the four types of delay?
5. What is jitter?
6. How is packet loss calculated?
7. What is RTT?
8. What is the difference between TCP and UDP?
9. Explain the TCP three-way handshake.
10. What is the OSI model?
11. What is the difference between a hub, switch and router?
12. What is an IP address?
13. What is a subnet mask?
14. What is DNS?
15. What is DHCP?
16. What is routing?
17. What is congestion?
18. What is a bottleneck link?
19. What is DropTail?
20. What is FlowMonitor?
21. What is PCAP?
22. Why do we use global routing in NS-3?
23. Why do we assign a different subnet to each point-to-point link?
24. How is throughput calculated in NS-3?
25. Why do we inspect packet captures for TCP retransmissions?

## Study Strategy

1. Learn the definitions and differences between the performance metrics.
2. Memorize the delay, throughput, packet loss and BDP formulas.
3. Revise the OSI model and network devices.
4. Understand TCP, UDP, IP addressing and routing.
5. Learn the NS-3 workflow and the purpose of each helper class.
6. Practise answering viva questions aloud using short, clear explanations.

**Key rule:** For every concept, understand what it means, why it is used, and how it applies to your NS-3 experiment.
