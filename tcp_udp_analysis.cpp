
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

#include <fstream>
#include <iostream>
#include <iomanip>

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

    // Create nodes
    NodeContainer nodes;
    nodes.Create(2);

    // Configure link and DropTail queue
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(bandwidth));
    p2p.SetChannelAttribute("Delay", StringValue(linkDelay));
    p2p.SetQueue("ns3::DropTailQueue<Packet>",
                 "MaxSize",
                 StringValue(std::to_string(queueSize) + "p"));

    NetDeviceContainer devices = p2p.Install(nodes);

    // Install Internet stack and IP addresses
    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    double appStart = 1.0;
    double appStop = simTime - 1.0;

    if (appStop <= appStart)
    {
        std::cerr << "Simulation time must be greater than 2 seconds.\n";
        return 1;
    }

    // TCP receiver
    uint16_t tcpPort = 5000;

    PacketSinkHelper tcpSinkHelper(
        "ns3::TcpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), tcpPort));

    ApplicationContainer tcpSink = tcpSinkHelper.Install(nodes.Get(1));
    tcpSink.Start(Seconds(0.0));
    tcpSink.Stop(Seconds(simTime));

    // TCP sender
    BulkSendHelper tcpClient(
        "ns3::TcpSocketFactory",
        InetSocketAddress(interfaces.GetAddress(1), tcpPort));

    tcpClient.SetAttribute("MaxBytes", UintegerValue(0));
    tcpClient.SetAttribute("SendSize", UintegerValue(packetSize));

    ApplicationContainer tcpApp = tcpClient.Install(nodes.Get(0));
    tcpApp.Start(Seconds(appStart));
    tcpApp.Stop(Seconds(appStop));

    // UDP receiver
    uint16_t udpPort = 6000;

    PacketSinkHelper udpSinkHelper(
        "ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), udpPort));

    ApplicationContainer udpSink = udpSinkHelper.Install(nodes.Get(1));
    udpSink.Start(Seconds(0.0));
    udpSink.Stop(Seconds(simTime));

    // UDP sender
    OnOffHelper udpClient(
        "ns3::UdpSocketFactory",
        InetSocketAddress(interfaces.GetAddress(1), udpPort));

    udpClient.SetAttribute("DataRate", StringValue(udpRate));
    udpClient.SetAttribute("PacketSize", UintegerValue(packetSize));
    udpClient.SetAttribute(
        "OnTime",
        StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    udpClient.SetAttribute(
        "OffTime",
        StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer udpApp = udpClient.Install(nodes.Get(0));
    udpApp.Start(Seconds(appStart));
    udpApp.Stop(Seconds(appStop));

    // Monitor flows
    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    // Leave time for queued packets to arrive
    Simulator::Stop(Seconds(simTime + 2.0));
    Simulator::Run();

    monitor->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());

    std::ofstream csv("tcp_udp_results.csv");

    csv << "Protocol,TxPackets,RxPackets,LostPackets,"
           "ThroughputMbps,PacketLossPercent,PDRPercent,"
           "TransmissionRatioPercent,EndToEndDelayMs\n";

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\nTCP vs UDP Performance Results\n";
    std::cout << "Bandwidth: " << bandwidth
              << " | Link delay: " << linkDelay
              << " | Queue: DropTail, " << queueSize << " packets\n\n";

    for (const auto &flow : monitor->GetFlowStats())
    {
        Ipv4FlowClassifier::FiveTuple tuple =
            classifier->FindFlow(flow.first);

        const FlowMonitor::FlowStats &s = flow.second;

        std::string protocol;

        if (tuple.protocol == 6 && tuple.destinationPort == tcpPort)
            protocol = "TCP";
        else if (tuple.protocol == 17 && tuple.destinationPort == udpPort)
            protocol = "UDP";
        else
            continue;

        double packetLoss = 0.0;
        double pdr = 0.0;
        double transmissionRatio = 0.0;
        double throughput = 0.0;
        double delayMs = 0.0;

        if (s.txPackets > 0)
        {
            packetLoss =
                100.0 * (s.txPackets - s.rxPackets) / s.txPackets;

            pdr = 100.0 * s.rxPackets / s.txPackets;
        }

        // Byte transmission ratio: received IP bytes / transmitted IP bytes
        if (s.txBytes > 0)
        {
            transmissionRatio = 100.0 * s.rxBytes / s.txBytes;
        }

        // Measure throughput over the common application interval
        double measurementTime = appStop - appStart;
        throughput = s.rxBytes * 8.0 / measurementTime / 1e6;

        if (s.rxPackets > 0)
        {
            delayMs =
                1000.0 * s.delaySum.GetSeconds() / s.rxPackets;
        }

        std::cout << protocol
                  << ": Tx=" << s.txPackets
                  << ", Rx=" << s.rxPackets
                  << ", Lost=" << s.lostPackets
                  << ", Throughput=" << throughput << " Mbps"
                  << ", Packet loss=" << packetLoss << "%"
                  << ", PDR=" << pdr << "%"
                  << ", Transmission ratio=" << transmissionRatio << "%"
                  << ", End-to-end delay=" << delayMs << " ms\n";

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

    monitor->SerializeToXmlFile("tcp_udp_flowmon.xml", true, true);

    Simulator::Destroy();

    std::cout << "\nSaved: tcp_udp_results.csv\n";
    std::cout << "Saved: tcp_udp_flowmon.xml\n";

    return 0;
}
