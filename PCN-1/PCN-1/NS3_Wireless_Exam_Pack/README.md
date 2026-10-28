# NS-3 Wireless Network Simulation — R1 to R4

## 1. Overview

This guide covers four wireless networking experiments using NS-3 and C++.

| Question | Experiment | Main objective |
|---|---|---|
| R1 | Wi-Fi topology and traffic | Create a Wi-Fi network and measure performance |
| R2 | Node mobility | Compare static and moving nodes |
| R3 | AODV routing | Configure routing protocols and examine route behavior |
| R4 | Wireless performance analysis | Vary traffic rate and number of stations |

**Common source file:** `wifi_topology_metrics.cc`

All four experiments can share a common base program if the required command-line options and network behavior are implemented correctly.

---

# 2. Common Algorithm

### Algorithm: Wireless Network Simulation

1. Start the simulation.
2. Create Wi-Fi stations and an access point.
3. Configure Wi-Fi PHY and MAC layers.
4. Configure node mobility.
5. Install the Internet stack and routing protocol.
6. Assign IP addresses to network interfaces.
7. Configure UDP sender and receiver applications.
8. Enable FlowMonitor.
9. Run the simulation.
10. Collect transmitted packets, received packets, delay and received bytes.
11. Calculate throughput, PDR, packet loss and average delay.
12. Print the results, save the FlowMonitor XML file and destroy the simulator.

### Flowchart

```text
Start
  |
Create Wi-Fi nodes and AP
  |
Configure PHY, MAC and mobility
  |
Install Internet stack and routing
  |
Assign IP addresses
  |
Configure UDP sender and receiver
  |
Enable FlowMonitor
  |
Run simulation
  |
Collect packet statistics
  |
Calculate performance metrics
  |
Print results and save XML
  |
Stop
```

---

# 3. R1 — Create a Wi-Fi Topology and Generate Traffic

## Objective

Create a Wi-Fi network with stations and one access point, generate UDP traffic and measure network performance.

## Concepts

- Wi-Fi stations and access point
- PHY and MAC configuration
- IP addressing
- UDP applications
- FlowMonitor
- Throughput, PDR, packet loss and delay

## Algorithm

1. Create the station nodes and AP node.
2. Configure the Wi-Fi standard, channel and PHY.
3. Install station and AP Wi-Fi devices.
4. Configure mobility.
5. Install the Internet stack.
6. Assign IP addresses.
7. Install a UDP server on the AP.
8. Install a UDP client on a station.
9. Run the simulation and collect performance metrics.

## C++ code: Wi-Fi topology

```cpp
NodeContainer stations;
stations.Create(nWifi);

NodeContainer apNode;
apNode.Create(1);

WifiHelper wifi;
wifi.SetStandard(WIFI_STANDARD_80211g);

WifiMacHelper mac;
Ssid ssid = Ssid("ns3-wifi");

YansWifiChannelHelper channel =
    YansWifiChannelHelper::Default();

YansWifiPhyHelper phy;
phy.SetChannel(channel.Create());

// Install Wi-Fi on stations
mac.SetType("ns3::StaWifiMac",
    "Ssid", SsidValue(ssid),
    "ActiveProbing", BooleanValue(false));

NetDeviceContainer staDevices =
    wifi.Install(phy, mac, stations);

// Install Wi-Fi on AP
mac.SetType("ns3::ApWifiMac",
    "Ssid", SsidValue(ssid));

NetDeviceContainer apDevices =
    wifi.Install(phy, mac, apNode);
```

### Explanation

- `NodeContainer` creates the nodes.
- `WifiHelper` configures the Wi-Fi standard.
- `YansWifiChannelHelper` creates the wireless channel.
- `YansWifiPhyHelper` configures the physical layer.
- `WifiMacHelper` configures station and AP MAC behavior.
- `wifi.Install()` installs Wi-Fi devices on the nodes.

## IP addressing

```cpp
InternetStackHelper internet;
internet.Install(stations);
internet.Install(apNode);

Ipv4AddressHelper address;
address.SetBase("10.1.1.0", "255.255.255.0");

Ipv4InterfaceContainer staInterfaces =
    address.Assign(staDevices);

Ipv4InterfaceContainer apInterface =
    address.Assign(apDevices);
```

**Important:** The example assumes the stations and AP are connected through the same infrastructure Wi-Fi network and use a compatible IP subnet. Verify connectivity and routing for the chosen NS-3 version and configuration.

## UDP traffic

```cpp
uint16_t port = 9;

UdpServerHelper server(port);
ApplicationContainer serverApp =
    server.Install(apNode.Get(0));

serverApp.Start(Seconds(1.0));
serverApp.Stop(Seconds(20.0));

UdpClientHelper client(
    apInterface.GetAddress(0), port);

client.SetAttribute("MaxPackets",
    UintegerValue(10000));

client.SetAttribute("Interval",
    TimeValue(MilliSeconds(10)));

client.SetAttribute("PacketSize",
    UintegerValue(512));

ApplicationContainer clientApp =
    client.Install(stations.Get(0));

clientApp.Start(Seconds(2.0));
clientApp.Stop(Seconds(20.0));
```

The UDP client generates packets, and the server receives them. This configuration generates traffic from one station to the AP.

## Run

From the NS-3 root directory:

```bash
./ns3 run scratch/wifi_topology_metrics
```

---

# 4. R2 — Simulate Node Mobility and Evaluate Performance

## Objective

Compare network performance when nodes are stationary and when stations move.

## Algorithm

1. Create the Wi-Fi topology.
2. Configure a static mobility model.
3. Run the simulation and record the metrics.
4. Replace the static model with a RandomWalk mobility model.
5. Run the simulation again.
6. Compare throughput, PDR, delay and packet loss.

## C++ code: Static mobility

```cpp
MobilityHelper mobility;

mobility.SetMobilityModel(
    "ns3::ConstantPositionMobilityModel");

mobility.Install(stations);
mobility.Install(apNode);
```

## C++ code: RandomWalk mobility

```cpp
MobilityHelper mobility;

mobility.SetMobilityModel(
    "ns3::RandomWalk2dMobilityModel",
    "Bounds",
    RectangleValue(
        Rectangle(0, 100, 0, 100)));

mobility.Install(stations);

// Keep the AP stationary
MobilityHelper apMobility;
apMobility.SetMobilityModel(
    "ns3::ConstantPositionMobilityModel");

apMobility.Install(apNode);
```

## Run

**Static baseline:**

```bash
./ns3 run "scratch/wifi_topology_metrics --mode=static"
```

**Moving stations:**

```bash
./ns3 run "scratch/wifi_topology_metrics --mode=randomwalk"
```

## Interpretation

- Mobility can change wireless distances and signal quality.
- Link changes may increase delay or packet loss.
- Throughput may decrease if connectivity deteriorates.
- Actual results depend on the topology, PHY settings, traffic and random seed.

Keep all other simulation parameters constant when comparing static and mobile cases.

---

# 5. R3 — Configure AODV and Examine Routing Behavior

## Objective

Configure wireless routing and understand the difference between AODV and OLSR.

## AODV — Ad hoc On-Demand Distance Vector

AODV is a **reactive routing protocol**. It discovers routes when needed.

Important control messages:

- **RREQ:** Route Request
- **RREP:** Route Reply
- **RERR:** Route Error

### AODV algorithm

1. A source needs a route to a destination.
2. It broadcasts an RREQ.
3. The destination, or an intermediate node with a valid route, can send an RREP.
4. The source sends data using the discovered route.
5. If a link breaks, an RERR can report the failure and a route may need to be rediscovered.

## OLSR — Optimized Link State Routing

OLSR is a **proactive routing protocol**. It periodically maintains routing information before application traffic needs a route.

Important concepts:

- **HELLO:** Discovers and maintains neighbor information.
- **TC:** Topology-control messages.
- **MPR:** Multipoint Relay, which reduces redundant forwarding of topology-control messages.

### OLSR algorithm

1. Nodes periodically discover their neighbors.
2. They exchange topology-control information.
3. Each node calculates and maintains routes.
4. Application packets use an available route.
5. Routing information is updated as the topology changes.

## AODV versus OLSR

| Feature | AODV | OLSR |
|---|---|---|
| Type | Reactive | Proactive |
| Route discovery | When needed | Maintained in advance |
| Control messages | RREQ, RREP, RERR | HELLO, TC |
| Initial route delay | May be higher | Often lower if a route is ready |
| Control overhead | Often lower when few routes are needed | Periodic control traffic |
| Key mechanism | On-demand discovery | Multipoint Relays |

## C++ code: Select a routing protocol

```cpp
AodvHelper aodv;
OlsrHelper olsr;
Ipv4ListRoutingHelper list;

if (routing == "aodv")
{
    list.Add(aodv, 10);
}
else if (routing == "olsr")
{
    list.Add(olsr, 10);
}

InternetStackHelper internet;
internet.SetRoutingHelper(list);
internet.Install(nodes);
```

Required module headers:

```cpp
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
```

Here, `routing` is the selected command-line string, and `nodes` must contain the nodes participating in the intended routing topology.

## Run

**AODV:**

```bash
./ns3 run "scratch/wifi_topology_metrics --routing=aodv --mode=randomwalk"
```

**OLSR:**

```bash
./ns3 run "scratch/wifi_topology_metrics --routing=olsr --mode=randomwalk"
```

### Important limitation

Selecting a routing helper is not sufficient to prove multi-hop routing. A meaningful AODV/OLSR comparison requires an appropriate ad hoc Wi-Fi topology with intermediate forwarding nodes and a destination several hops away.

The basic station-to-AP infrastructure example demonstrates Wi-Fi traffic, but it is not a strong multi-hop route-discovery experiment.

---

# 6. R4 — Measure Wireless Performance Under Changing Conditions

## Objective

Study how offered traffic, the number of stations and mobility affect wireless performance.

## Algorithm

1. Configure a baseline Wi-Fi network.
2. Run the simulation with a default UDP rate and station count.
3. Reduce the UDP rate and collect the metrics.
4. Increase the UDP rate and collect the metrics.
5. Increase the station count and collect the metrics.
6. Run a mobile-node case if required.
7. Compare the results.

## C++ code: Command-line parameters

```cpp
uint32_t nWifi = 6;
std::string udpRate = "1Mbps";

CommandLine cmd;

cmd.AddValue(
    "nWifi",
    "Number of Wi-Fi stations",
    nWifi);

cmd.AddValue(
    "udpRate",
    "UDP offered data rate",
    udpRate);

cmd.Parse(argc, argv);
```

The program must use `nWifi` when creating stations and use `udpRate` when configuring the UDP application. Merely declaring a command-line argument does not change the simulation by itself.

## Run

**Baseline:**

```bash
./ns3 run "scratch/wifi_topology_metrics --mode=static --nWifi=6 --udpRate=1Mbps"
```

**Lower traffic rate:**

```bash
./ns3 run "scratch/wifi_topology_metrics --mode=static --nWifi=6 --udpRate=500Kbps"
```

**Higher traffic rate:**

```bash
./ns3 run "scratch/wifi_topology_metrics --mode=static --nWifi=6 --udpRate=2Mbps"
```

**More stations:**

```bash
./ns3 run "scratch/wifi_topology_metrics --mode=static --nWifi=10 --udpRate=1Mbps"
```

**Moving stations:**

```bash
./ns3 run "scratch/wifi_topology_metrics --mode=randomwalk --nWifi=6 --udpRate=1Mbps"
```

## Results table

Record actual values printed by your program.

| Run | Mode | Stations | UDP rate | Throughput (Mbps) | PDR (%) | Delay (ms) | Lost packets |
|---|---|---:|---|---:|---:|---:|---:|
| 1 | Static | 6 | 1 Mbps | | | | |
| 2 | Static | 6 | 500 Kbps | | | | |
| 3 | Static | 6 | 2 Mbps | | | | |
| 4 | Static | 10 | 1 Mbps | | | | |
| 5 | RandomWalk | 6 | 1 Mbps | | | | |

### Interpretation

- Higher offered traffic can increase contention and packet loss.
- More stations can increase competition for channel access.
- Mobility can change connectivity and link quality.
- Results depend on the PHY, topology, routing, simulation duration and random seed.

These are expected trends, not guaranteed numerical outcomes.

---

# 7. Common Performance Metrics

All four experiments use the same core metrics.

## 1. Throughput

The amount of data successfully received per unit time.

\[
\text{Throughput (Mbps)}
=
\frac{\text{Received bytes}\times 8}
{\text{Receive interval}\times 10^6}
\]

## 2. Packet Delivery Ratio (PDR)

The percentage of transmitted packets received successfully.

\[
\text{PDR (\%)}
=
\frac{\text{Received packets}}
{\text{Transmitted packets}}\times100
\]

## 3. Packet loss

\[
\text{Lost packets}
=
\text{Transmitted packets}-\text{Received packets}
\]

## 4. Packet loss percentage

\[
\text{Packet loss (\%)}
=
\frac{\text{Lost packets}}
{\text{Transmitted packets}}\times100
\]

## 5. Average end-to-end delay

\[
\text{Average delay}
=
\frac{\text{Total end-to-end delay}}
{\text{Received packets}}
\]

Multiply seconds by 1,000 to obtain milliseconds.

### C++ code: Calculate metrics from FlowMonitor

```cpp
monitor->CheckForLostPackets();

Ptr<Ipv4FlowClassifier> classifier =
    DynamicCast<Ipv4FlowClassifier>(
        flowHelper.GetClassifier());

auto stats = monitor->GetFlowStats();

for (const auto &entry : stats)
{
    const FlowMonitor::FlowStats &s =
        entry.second;

    double pdr = 0.0;
    double throughputMbps = 0.0;
    double avgDelayMs = 0.0;

    if (s.txPackets > 0)
    {
        pdr = 100.0 * s.rxPackets / s.txPackets;
    }

    if (s.rxPackets > 0)
    {
        avgDelayMs =
            1000.0 * s.delaySum.GetSeconds()
            / s.rxPackets;

        double interval =
            (s.timeLastRxPacket -
             s.timeFirstTxPacket).GetSeconds();

        if (interval > 0)
        {
            throughputMbps =
                8.0 * s.rxBytes
                / (interval * 1e6);
        }
    }

    uint32_t lostEstimate =
        s.txPackets - s.rxPackets;

    std::cout
        << "Flow ID: " << entry.first << "\n"
        << "Tx Packets: " << s.txPackets << "\n"
        << "Rx Packets: " << s.rxPackets << "\n"
        << "PDR: " << pdr << "%\n"
        << "Throughput: " << throughputMbps
        << " Mbps\n"
        << "Average Delay: " << avgDelayMs
        << " ms\n"
        << "Tx-Rx Loss Estimate: " << lostEstimate
        << " packets\n";
}
```

### Important measurement notes

- The throughput interval should be defined consistently for each run.
- `txPackets - rxPackets` is a simple loss estimate, not necessarily FlowMonitor's own loss count.
- FlowMonitor's `lostPackets` is based on its own tracking and timeout behavior.
- Avoid dividing by zero when no packets are transmitted or received.
- The sample uses FlowMonitor's `delaySum` to calculate average end-to-end delay.

---

# 8. FlowMonitor and Simulation Execution

## C++ code

```cpp
FlowMonitorHelper flowHelper;

Ptr<FlowMonitor> monitor =
    flowHelper.InstallAll();

Simulator::Stop(Seconds(20.0));

Simulator::Run();

monitor->CheckForLostPackets();

monitor->SerializeToXmlFile(
    "wireless-flowmon.xml",
    true,
    true);

Simulator::Destroy();
```

### Explanation

- `InstallAll()` enables FlowMonitor on the relevant nodes.
- `Simulator::Run()` executes the discrete-event simulation.
- `CheckForLostPackets()` updates FlowMonitor's loss accounting.
- `SerializeToXmlFile()` saves flow statistics.
- `Simulator::Destroy()` cleans up the simulation state.

The metric-calculation loop must execute after `Simulator::Run()` and before `Simulator::Destroy()`.

---

# 9. Common NS-3 Workflow

The general implementation sequence is:

1. Create nodes.
2. Configure Wi-Fi devices, PHY and MAC.
3. Configure mobility.
4. Configure routing and install the Internet stack.
5. Assign IP addresses.
6. Configure UDP applications.
7. Install FlowMonitor and optional tracing.
8. Run the simulation.
9. Collect and print metrics.
10. Save results and destroy the simulator.

Exact helper configuration and installation order can depend on the NS-3 version and topology.

---

# 10. Viva Questions and Answers

**1. What is NS-3?**

NS-3 is a discrete-event network simulator used to study networking protocols and performance.

**2. What is a Wi-Fi access point?**

An AP provides a connection point for wireless stations in an infrastructure Wi-Fi network.

**3. What is UDP?**

UDP is a connectionless transport protocol without built-in delivery guarantees, ordering or retransmission.

**4. What is throughput?**

The amount of data successfully received per unit time.

**5. What is PDR?**

The percentage of transmitted packets received successfully.

**6. What is node mobility?**

Node mobility describes how network nodes move during a simulation.

**7. What is AODV?**

A reactive routing protocol that discovers routes when needed.

**8. What is OLSR?**

A proactive routing protocol that periodically maintains routing information.

**9. What is the main difference between AODV and OLSR?**

AODV discovers routes on demand, while OLSR maintains routes in advance.

**10. What is FlowMonitor?**

FlowMonitor collects flow-level statistics such as packet counts, delay and received bytes.

**11. Why compare static and moving nodes?**

To study how movement and changing wireless links affect performance.

**12. Why vary the UDP rate?**

To study how offered traffic and channel contention affect throughput, delay and packet loss.

**13. Does FlowMonitor directly measure every TCP retransmission?**

No. Detailed TCP retransmission analysis generally requires TCP traces or packet captures.

**14. Does station-to-AP traffic prove multi-hop routing?**

No. A multi-hop experiment requires intermediate forwarding nodes and suitable routing-event observations.

---

# 11. Final Revision

| Question | What to change | What to explain |
|---|---|---|
| R1 | Basic Wi-Fi setup and UDP traffic | Topology, addressing and metrics |
| R2 | Mobility model | Static versus mobile performance |
| R3 | AODV or OLSR | Reactive versus proactive routing |
| R4 | UDP rate or station count | Performance under changing conditions |

**Viva summary:**

“I create the Wi-Fi topology, configure PHY/MAC, install the Internet stack, assign IP addresses and configure UDP traffic. I use FlowMonitor to collect packet statistics and calculate throughput, PDR, packet loss and average delay. For R2, I change mobility. For R3, I configure routing in a suitable multi-hop topology. For R4, I vary the UDP rate or number of stations and compare the results.”

**Note:** The code snippets illustrate the main components and need to be integrated into a complete, version-compatible NS-3 source file. Verify that every command-line option is implemented before using the run commands.
