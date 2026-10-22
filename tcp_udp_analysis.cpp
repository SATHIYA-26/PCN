
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

using namespace ns3;

int main(int argc, char *argv[])
{
    // Simulation parameters
    std::string bandwidth = "2Mbps";
    std::string linkDelay = "10ms";
    uint32_t queueSize = 50;
    uint32_t packetSize = 1024;
    std::string udpRate = "4Mbps";
    double simTime = 10.0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("bandwidth", "Link bandwidth", bandwidth);
    cmd.AddValue("linkDelay", "Link delay", linkDelay);
    cmd.AddValue("queueSize", "Queue size in packets", queueSize);
    cmd.AddValue("packetSize", "Packet size in bytes", packetSize);
    cmd.AddValue("udpRate", "UDP data rate", udpRate);
    cmd.AddValue("simTime", "Simulation time in seconds", simTime);
    cmd.Parse(argc, argv);

    if (simTime <= 2.0)
    {
        std::cerr << "Simulation time must be greater than 2 seconds.\n";
        return 1;
    }

    // 1. Create 6 nodes
    NodeContainer nodes;
    nodes.Create(6);

    // 2. Configure point-to-point links
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(bandwidth));
    p2p.SetChannelAttribute("Delay", StringValue(linkDelay));

    p2p.SetQueue("ns3::DropTailQueue<Packet>",
                 "MaxSize",
                 StringValue(std::to_string(queueSize) + "p"));

    // 3. Create the required topology
    // Node 0 -- Node 2
    NetDeviceContainer d02 =
        p2p.Install(nodes.Get(0), nodes.Get(2));

    // Node 1 -- Node 2
    NetDeviceContainer d12 =
        p2p.Install(nodes.Get(1), nodes.Get(2));

    // Node 2 -- Node 3
    NetDeviceContainer d23 =
        p2p.Install(nodes.Get(2), nodes.Get(3));

    // Node 3 -- Node 4
    NetDeviceContainer d34 =
        p2p.Install(nodes.Get(3), nodes.Get(4));

    // Node 3 -- Node 5
    NetDeviceContainer d35 =
        p2p.Install(nodes.Get(3), nodes.Get(5));

    // 4. Install Internet stack on all nodes
    InternetStackHelper internet;
    internet.Install(nodes);

    // 5. Assign a separate IP subnet to each link
    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    address.Assign(d02);

    address.SetBase("10.1.2.0", "255.255.255.0");
    address.Assign(d12);

    address.SetBase("10.1.3.0", "255.255.255.0");
    address.Assign(d23);

    address.SetBase("10.1.4.0", "255.255.255.0");
    address.Assign(d34);

    address.SetBase("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer i35 = address.Assign(d35);

    // Destination IP: Node 5's address on the Node 3--Node 5 link
    Ipv4Address destination = i35.GetAddress(1);

    // 6. Enable routing between nodes
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Applications run between 1 second and simTime - 1 second
    double appStart = 1.0;
    double appStop = simTime - 1.0;
    double measurementTime = appStop - appStart;

    // 7. TCP receiver on Node 5
    uint16_t tcpPort = 5000;

    PacketSinkHelper tcpSinkHelper(
        "ns3::TcpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), tcpPort));

    ApplicationContainer tcpSink =
        tcpSinkHelper.Install(nodes.Get(5));

    tcpSink.Start(Seconds(0.0));
    tcpSink.Stop(Seconds(simTime + 2.0));

    // 8. TCP sender on Node 0
    BulkSendHelper tcpClient(
        "ns3::TcpSocketFactory",
        InetSocketAddress(destination, tcpPort));

    tcpClient.SetAttribute("MaxBytes", UintegerValue(0));
    tcpClient.SetAttribute("SendSize", UintegerValue(packetSize));

    ApplicationContainer tcpApp =
        tcpClient.Install(nodes.Get(0));

    tcpApp.Start(Seconds(appStart));
    tcpApp.Stop(Seconds(appStop));

    // 9. UDP receiver on Node 5
    uint16_t udpPort = 6000;

    PacketSinkHelper udpSinkHelper(
        "ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), udpPort));

    ApplicationContainer udpSink =
        udpSinkHelper.Install(nodes.Get(5));

    udpSink.Start(Seconds(0.0));
    udpSink.Stop(Seconds(simTime + 2.0));

    // 10. UDP sender on Node 0
    OnOffHelper udpClient(
        "ns3::UdpSocketFactory",
        InetSocketAddress(destination, udpPort));

    udpClient.SetAttribute("DataRate", StringValue(udpRate));
    udpClient.SetAttribute("PacketSize", UintegerValue(packetSize));

    udpClient.SetAttribute(
        "OnTime",
        StringValue("ns3::ConstantRandomVariable[Constant=1]"));

    udpClient.SetAttribute(
        "OffTime",
        StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer udpApp =
        udpClient.Install(nodes.Get(0));

    udpApp.Start(Seconds(appStart));
    udpApp.Stop(Seconds(appStop));

    // 11. Install FlowMonitor
    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    // Allow packets in transit to arrive
    Simulator::Stop(Seconds(simTime + 2.0));
    Simulator::Run();

    monitor->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(
            flowHelper.GetClassifier());

    // 12. Open CSV output file
    std::ofstream csv("tcp_udp_results.csv");

    csv << "Protocol,TxPackets,RxPackets,LostPackets,"
           "ThroughputMbps,PacketLossPercent,PDRPercent,"
           "TransmissionRatioPercent,EndToEndDelayMs\n";

    std::cout << std::fixed << std::setprecision(4);

    std::cout << "\n===== TCP vs UDP Performance =====\n";
    std::cout << "Nodes: 6\n";
    std::cout << "Source: Node 0\n";
    std::cout << "Destination: Node 5\n";
    std::cout << "Bandwidth per link: " << bandwidth << "\n";
    std::cout << "Link delay: " << linkDelay << "\n";
    std::cout << "Queue: DropTail, " << queueSize << " packets\n";
    std::cout << "UDP offered rate: " << udpRate << "\n\n";

    // 13. Calculate performance metrics
    for (const auto &flow : monitor->GetFlowStats())
    {
        Ipv4FlowClassifier::FiveTuple tuple =
            classifier->FindFlow(flow.first);

        const FlowMonitor::FlowStats &s = flow.second;

        std::string protocol;

        if (tuple.protocol == 6 &&
            tuple.destinationPort == tcpPort)
        {
            protocol = "TCP";
        }
        else if (tuple.protocol == 17 &&
                 tuple.destinationPort == udpPort)
        {
            protocol = "UDP";
        }
        else
        {
            continue;
        }

        double packetLoss = 0.0;
        double pdr = 0.0;
        double transmissionRatio = 0.0;
        double throughput = 0.0;
        double delayMs = 0.0;

        if (s.txPackets > 0)
        {
            packetLoss =
                100.0 * (s.txPackets - s.rxPackets)
                / s.txPackets;

            pdr = 100.0 * s.rxPackets / s.txPackets;
        }

        // Received IP bytes / transmitted IP bytes
        if (s.txBytes > 0)
        {
            transmissionRatio =
                100.0 * s.rxBytes / s.txBytes;
        }

        // Throughput in Mbps
        if (measurementTime > 0)
        {
            throughput =
                s.rxBytes * 8.0 / measurementTime / 1e6;
        }

        // Average end-to-end delay in milliseconds
        if (s.rxPackets > 0)
        {
            delayMs =
                1000.0 * s.delaySum.GetSeconds()
                / s.rxPackets;
        }

        std::cout << protocol
                  << ": Tx=" << s.txPackets
                  << ", Rx=" << s.rxPackets
                  << ", Lost=" << s.lostPackets
                  << ", Throughput=" << throughput << " Mbps"
                  << ", Packet loss=" << packetLoss << "%"
                  << ", PDR=" << pdr << "%"
                  << ", Transmission ratio="
                  << transmissionRatio << "%"
                  << ", End-to-end delay="
                  << delayMs << " ms\n";

        csv << protocol << ","
            << s.txPackets << ","
            << s.rxPackets << ","
            << s.lostPackets << ","
            << throughput << ","
            << packetLoss << ","
            << pdr << ","
            << transmissionRatio << ","
            << delayMs << "\n";
    }

    csv.close();

    // Save detailed FlowMonitor statistics
    monitor->SerializeToXmlFile(
        "tcp_udp_flowmon.xml", true, true);

    Simulator::Destroy();

    std::cout << "\nSaved: tcp_udp_results.csv\n";
    std::cout << "Saved: tcp_udp_flowmon.xml\n";

    return 0;
}
