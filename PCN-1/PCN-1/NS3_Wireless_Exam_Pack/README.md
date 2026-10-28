# NS-3 Wireless Network Simulation — R1 to R4

## Overview

This guide covers four wireless network simulation experiments using **NS-3 and C++**:

- **R1:** Create a Wi-Fi topology and generate UDP traffic.
- **R2:** Simulate node mobility and evaluate performance.
- **R3:** Configure AODV and examine routing behavior.
- **R4:** Measure wireless performance under changing network conditions.

All four experiments use the same base source file:

`wifi_topology_metrics.cc`

The source file must implement the command-line options used in each experiment.

---

## Requirements

- NS-3 installed and working
- C++ compiler supported by your NS-3 version
- Basic knowledge of Wi-Fi, UDP, IP addressing, routing and network performance metrics

Copy `wifi_topology_metrics.cc` into the `scratch/` directory of your NS-3 installation.

## R1 — Create a Wi-Fi Topology and Generate Traffic

### Objective

Create a wireless network with Wi-Fi stations and an access point (AP), configure the Wi-Fi PHY/MAC, assign IP addresses, generate UDP traffic and measure network performance.

### Concepts covered

- Wi-Fi stations and access point
- Wi-Fi PHY and MAC configuration
- IP addressing and routing helper
- UDP client/server traffic
- FlowMonitor and performance measurement

### Execution

From the NS-3 root directory:

```bash
./ns3 run scratch/wifi_topology_metrics
```

### Expected output

The program should report metrics such as:

- Transmitted packets
- Received packets
- Packet loss
- Packet Delivery Ratio (PDR)
- Throughput
- Average end-to-end delay

### Topology note

In the basic example, a Wi-Fi station sends UDP traffic to the AP itself. This demonstrates Wi-Fi connectivity and traffic measurement, but not station-to-station forwarding or multi-hop routing.

---

## R2 — Simulate Node Mobility and Evaluate Performance

### Objective

Study how node movement affects wireless network performance by comparing a static network with a mobile network.

### Concepts covered

- Static mobility
- RandomWalk mobility
- Changes in wireless connectivity
- Throughput, delay, PDR and packet loss comparison

### Execution

**Static baseline:**

```bash
./ns3 run "scratch/wifi_topology_metrics --mode=static"
```

**Moving nodes:**

```bash
./ns3 run "scratch/wifi_topology_metrics --mode=randomwalk"
```

### Procedure

1. Run the static simulation.
2. Record throughput, PDR, average delay and packet loss.
3. Run the simulation with RandomWalk mobility.
4. Record the same metrics.
5. Compare the two results.

Keep the number of stations, UDP rate, simulation duration and routing configuration constant.

### Interpretation

Movement can change distances, signal quality and connectivity. Depending on the topology and radio settings, mobility may increase delay or packet loss and reduce throughput. These changes are not guaranteed in every simulation.

**Important:** If the source accepts a `--speed` argument but does not connect it to the mobility model, changing that argument will not change node speed. Update the source before relying on it.

---

## R3 — Configure AODV and Examine Routing Behavior

### Objective

Configure a wireless routing protocol and understand how AODV and OLSR differ in route discovery and maintenance.

### AODV — Ad hoc On-Demand Distance Vector

AODV is a **reactive routing protocol**. It discovers routes when required.

Important messages:

- **RREQ:** Route Request
- **RREP:** Route Reply
- **RERR:** Route Error

AODV can reduce unnecessary route-maintenance traffic when routes are not needed, but route discovery may introduce initial delay.

### OLSR — Optimized Link State Routing

OLSR is a **proactive routing protocol**. It periodically maintains routing information before application traffic requires a route.

Important concepts:

- **HELLO messages:** Discover and maintain neighbor information.
- **TC messages:** Distribute topology-control information.
- **MPR:** Multipoint Relay, used to reduce redundant forwarding of control messages.

OLSR may provide lower initial route delay when a valid route is already available, at the cost of periodic control traffic.

### Execution

**AODV:**

```bash
./ns3 run "scratch/wifi_topology_metrics --routing=aodv --mode=randomwalk"
```

**OLSR:**

```bash
./ns3 run "scratch/wifi_topology_metrics --routing=olsr --mode=randomwalk"
```

### Comparison

| Feature | AODV | OLSR |
|---|---|---|
| Routing type | Reactive | Proactive |
| Route discovery | When needed | Routes maintained in advance |
| Main messages | RREQ, RREP, RERR | HELLO, TC |
| Initial route delay | May be higher | May be lower if a route is available |
| Control overhead | Often lower when few routes are needed | Periodic control traffic |
| Key mechanism | On-demand route discovery | Multipoint Relays (MPRs) |

### Important limitation

The source code must actually install the selected routing protocol. Merely accepting `--routing=aodv` or `--routing=olsr` does not guarantee that the protocol is configured.

The basic station-to-AP example is not a strong demonstration of multi-hop AODV route discovery. For meaningful route discovery and route-failure analysis, extend the simulation to an **ad hoc Wi-Fi topology with multiple hops** and trace routing events.

---

## R4 — Measure Wireless Performance Under Changing Conditions

### Objective

Evaluate how offered traffic, the number of Wi-Fi stations and mobility affect network performance.

### Execution

**Baseline:**

```bash
./ns3 run "scratch/wifi_topology_metrics --mode=static --nWifi=6 --udpRate=1Mbps"
```

**Lower UDP rate:**

```bash
./ns3 run "scratch/wifi_topology_metrics --mode=static --nWifi=6 --udpRate=500Kbps"
```

**Higher UDP rate:**

```bash
./ns3 run "scratch/wifi_topology_metrics --mode=static --nWifi=6 --udpRate=2Mbps"
```

**More stations:**

```bash
./ns3 run "scratch/wifi_topology_metrics --mode=static --nWifi=10 --udpRate=1Mbps"
```

**Moving nodes:**

```bash
./ns3 run "scratch/wifi_topology_metrics --mode=randomwalk --nWifi=6 --udpRate=1Mbps"
```

### Results table

Record the actual output from each run.

| Run | Mode | Stations | UDP rate | Throughput (Mbps) | PDR (%) | Avg. delay (ms) | Lost packets |
|---|---|---:|---|---:|---:|---:|---:|
| 1 | Static | 6 | 1 Mbps | | | | |
| 2 | Static | 6 | 500 Kbps | | | | |
| 3 | Static | 6 | 2 Mbps | | | | |
| 4 | Static | 10 | 1 Mbps | | | | |
| 5 | RandomWalk | 6 | 1 Mbps | | | | |

### Interpretation

- Higher offered traffic can increase contention and packet loss.
- More stations can increase competition for wireless channel access.
- Mobility can change link quality and connectivity.
- The actual outcome depends on PHY settings, topology, traffic, routing, simulation duration and random seed.

These are expected tendencies, not guaranteed numerical results.

---

## Common Performance Metrics

All four experiments use the same core metrics.

### 1. Throughput

Measures the amount of data successfully received per unit time.

\[
\text{Throughput (Mbps)}
=\frac{\text{Received bytes}\times 8}
{\text{Receive interval}\times 10^6}
\]

### 2. Packet Delivery Ratio (PDR)

Measures the percentage of transmitted packets received successfully.

\[
\text{PDR (\%)}
=\frac{\text{Received packets}}
{\text{Transmitted packets}}\times100
\]

### 3. Packet Loss

\[
\text{Lost packets}
=\text{Transmitted packets}-\text{Received packets}
\]

### 4. Packet Loss Percentage

\[
\text{Packet loss (\%)}
=\frac{\text{Lost packets}}
{\text{Transmitted packets}}\times100
\]

### 5. Average End-to-End Delay

\[
\text{Average delay}
=\frac{\text{Total end-to-end delay}}
{\text{Received packets}}
\]

Convert seconds to milliseconds by multiplying by 1,000.

**Measurement note:** Use consistent packet counts and a clearly defined measurement interval. FlowMonitor's loss accounting may differ from a simple transmitted-minus-received estimate, particularly depending on when packets are observed and when the simulation ends. Avoid dividing by zero if no packets are received or transmitted.

---

## Common NS-3 Workflow

The usual simulation workflow is:

1. Create nodes.
2. Configure Wi-Fi devices, PHY and MAC.
3. Configure mobility if required.
4. Install the Internet stack.
5. Configure the routing protocol.
6. Assign IP addresses.
7. Configure UDP applications.
8. Enable FlowMonitor and tracing if needed.
9. Run the simulation.
10. Collect metrics and save results.
11. Destroy the simulator.

The exact order of some helper configuration and installation calls depends on the NS-3 version and the chosen Wi-Fi setup.

---

## Viva Questions and Answers

**1. What is NS-3?**

NS-3 is a discrete-event network simulator used to study network protocols and performance.

**2. What is an access point?**

An access point provides wireless stations with a point of connection to a Wi-Fi network.

**3. What is UDP?**

UDP is a connectionless transport protocol that does not provide built-in delivery guarantees, ordering or retransmissions.

**4. What is throughput?**

The amount of data successfully received per unit time.

**5. What is PDR?**

The percentage of transmitted packets that reach the destination successfully.

**6. What is node mobility?**

Node mobility describes how nodes move during a simulation.

**7. What is AODV?**

A reactive routing protocol that discovers routes when needed.

**8. What is OLSR?**

A proactive routing protocol that periodically maintains routing information.

**9. What is the difference between AODV and OLSR?**

AODV discovers routes on demand, whereas OLSR maintains routes in advance.

**10. What is FlowMonitor?**

FlowMonitor collects flow-level statistics such as transmitted packets, received packets, delay and packet-loss information.

**11. Why compare static and mobile nodes?**

To study how movement and changing wireless links affect network performance.

**12. Why vary the UDP rate?**

To evaluate how offered traffic and channel contention affect throughput, delay and packet loss.

**13. Does FlowMonitor directly measure every TCP retransmission?**

No. Detailed TCP retransmission analysis generally requires suitable TCP traces or packet captures.

**14. Does station-to-AP traffic prove multi-hop AODV routing?**

No. A genuine multi-hop routing experiment requires an appropriate topology and routing-event observations.

---

## Quick Revision

| Question | Main task | Main modification |
|---|---|---|
| R1 | Wi-Fi topology and UDP traffic | Basic setup |
| R2 | Mobility and performance | `--mode=static` / `--mode=randomwalk` |
| R3 | Routing behavior | `--routing=aodv` / `--routing=olsr` |
| R4 | Performance under changing conditions | `--udpRate`, `--nWifi`, and mobility |

**Remember:** One common source file can support all four questions, provided the implementation correctly supports the required options and experiment objectives.
