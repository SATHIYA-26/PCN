# NS-3 Wired Network Simulation — W1, W2 & W3

## Overview

This project simulates a wired computer network using **NS-3 and C++**. It demonstrates network topology creation, IP addressing, routing, TCP/UDP communication, packet tracing and network performance analysis.

## Network Configuration

- **Nodes:** 6 (4 end hosts and 2 routers)
- **Point-to-point links:** 5
- **Access link rate:** 10 Mbps
- **Access link delay:** 2 ms
- **Bottleneck link rate:** 2 Mbps
- **Bottleneck link delay:** 10 ms
- **Queue discipline:** DropTail
- **Queue size:** 50 packets
- **TCP packet size:** 1024 bytes
- **UDP packet size:** 512 bytes
- **UDP data rate:** 1 Mbps
- **Simulation duration:** 20 seconds

## Node Roles

| Node | Role |
|---|---|
| Node 0 | TCP sender |
| Node 1 | UDP sender |
| Node 2 | UDP receiver |
| Node 3 | TCP receiver |
| Node 4 | Router |
| Node 5 | Router |

## Workflow

### 1. Create Nodes

Create six nodes using `NodeContainer`.

```cpp
NodeContainer nodes;
nodes.Create(6);
```

### 2. Create Point-to-Point Links

Use `PointToPointHelper` to connect the required pairs of nodes.

```cpp
PointToPointHelper p2p;

p2p.SetDeviceAttribute(
    "DataRate", StringValue("10Mbps"));

p2p.SetChannelAttribute(
    "Delay", StringValue("2ms"));
```

Install the links using `p2p.Install(nodeA, nodeB)`. Repeat for all five connections, configuring the bottleneck link separately.

### 3. Configure the Bottleneck and Queue

```cpp
PointToPointHelper bottleneck;

bottleneck.SetDeviceAttribute(
    "DataRate", StringValue("2Mbps"));

bottleneck.SetChannelAttribute(
    "Delay", StringValue("10ms"));

bottleneck.SetQueue(
    "ns3::DropTailQueue<Packet>",
    "MaxSize", QueueSizeValue(QueueSize("50p")));
```

### 4. Install the Internet Stack

Install networking protocols on all six nodes.

```cpp
InternetStackHelper internet;
internet.Install(nodes);
```

### 5. Assign IP Addresses

Use `Ipv4AddressHelper` to assign a different subnet to each point-to-point link.

```cpp
Ipv4AddressHelper address;

address.SetBase(
    "10.1.1.0", "255.255.255.0");

NetDeviceContainer devices = p2p.Install(
    nodes.Get(0), nodes.Get(4));

Ipv4InterfaceContainer interfaces =
    address.Assign(devices);
```

Repeat the addressing process for each link, using a unique subnet each time.

### 6. Configure Global Routing

Populate the routing tables so packets can travel across connected networks.

```cpp
Ipv4GlobalRoutingHelper::PopulateRoutingTables();
```

Nodes 4 and 5 act as routers by forwarding packets between their connected networks.

### 7. Configure TCP and UDP Applications

Configure sender and receiver applications using helpers such as:

- `PacketSinkHelper` — receives traffic.
- `OnOffHelper` — generates traffic.
- `InetSocketAddress` — specifies the destination IP address and port.

Use TCP for traffic from Node 0 to Node 3 and UDP for traffic from Node 1 to Node 2. Configure packet sizes, data rates, ports and application start/stop times.

### 8. Enable FlowMonitor and PCAP

FlowMonitor collects flow-level statistics.

```cpp
FlowMonitorHelper flowHelper;

Ptr<FlowMonitor> monitor =
    flowHelper.InstallAll();
```

Enable packet capture on the required bottleneck devices using `EnablePcap()`.

### 9. Run the Simulation

```cpp
Simulator::Stop(Seconds(20.0));
Simulator::Run();
```

### 10. Analyze Results

Use FlowMonitor statistics to analyze:

- Transmitted and received packets
- Estimated packet loss
- Throughput
- Average end-to-end delay
- Jitter sum

Save the FlowMonitor report as XML and inspect PCAP files using Wireshark to examine individual packets and TCP retransmissions.

### 11. Destroy the Simulator

```cpp
Simulator::Destroy();
```

## Performance Metrics

**Throughput**

Throughput is the amount of successfully received data per unit of time.

\[
\text{Throughput (Mbps)}
=
\frac{\text{Received bytes}\times 8}
{\text{Measurement time}\times 10^6}
\]

**Packet Loss Percentage**

\[
\text{Loss (\%)}
=
\frac{Tx-Rx}{Tx}\times100
\]

This simple formula is an estimate when transmitted and received packet counts are comparable for the same flow and measurement interval.

**Average End-to-End Delay**

\[
\text{Average Delay}
=
\frac{\text{Total packet delay}}
{\text{Received packets}}
\]

## Run the Program

Place `wired_simulation.cc` in the NS-3 `scratch/` directory.

From the NS-3 root directory, run:

```bash
./ns3 run scratch/wired_simulation
```

Example parameter variations, if supported by the source code:

```bash
./ns3 run "scratch/wired_simulation --bottleneckRate=5Mbps"

./ns3 run "scratch/wired_simulation --queueSize=100p"

./ns3 run "scratch/wired_simulation --bottleneckDelay=20ms"

./ns3 run "scratch/wired_simulation --udpRateMbps=2"
```

## Output Files

- `wired-flowmon.xml` — FlowMonitor statistics.
- `wired-bottleneck-*.pcap` — packet captures, if tracing is enabled.
- Terminal output — flow summaries and performance metrics.

## Experiment Mapping

- **W1:** Multi-node topology, IP addressing and routing.
- **W2:** TCP/UDP traffic generation and packet-level tracing.
- **W3:** Throughput, delay, packet loss and FlowMonitor analysis.

A common implementation can cover all three experiments when it includes the required features.

## Notes

- The topology and link connections must match the experiment specification.
- Use unique IP subnets for each point-to-point link.
- TCP retransmissions are not directly counted by a dedicated FlowMonitor retransmission counter; inspect PCAP or TCP traces.
- Actual performance results depend on the topology, traffic settings and NS-3 version.
- The code snippets above illustrate the workflow; integrate them with the complete source file and verify helper APIs against your installed NS-3 version.
