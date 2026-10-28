#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/aodv-module.h"
#include "ns3/output-stream-wrapper.h"
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("DeptRoutingCompare");

// ---------------- globals for metrics ----------------
static std::map<uint32_t, std::set<std::string>> g_prev;
static uint32_t g_updTotal = 0, g_updAfterFail = 0;
static bool g_baseline = false;
static double g_failTime = 20.0, g_simTime = 60.0;
static std::vector<double> g_rxTimes;       // F1 (p1->p3) receive timestamps
static std::vector<uint64_t> g_binBytes;    // F1 bytes per 1-s bin
static std::vector<uint32_t> g_binPkts;

// ---------------- routing table snapshot / update counting ----------------
static std::set<std::string>
ParseTable(const std::string& s)
{
    std::set<std::string> out;
    std::istringstream in(s);
    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty() || line.find("Node:") != std::string::npos ||
            line.rfind("Destination", 0) == 0)
            continue;
        std::istringstream ls(line);
        std::string a, b, c, d;
        ls >> a >> b >> c >> d;   // dest, gateway, iface/mask, flag  (ignores timers/metrics)
        if (a.empty()) continue;
        out.insert(a + " " + b + " " + c + " " + d);
    }
    return out;
}

static void
SampleTables(NodeContainer nodes)
{
    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        Ptr<Node> n = nodes.Get(i);
        std::ostringstream oss;
        Ptr<OutputStreamWrapper> w = Create<OutputStreamWrapper>(&oss);
        n->GetObject<Ipv4>()->GetRoutingProtocol()->PrintRoutingTable(w, Time::S);
        std::set<std::string> cur = ParseTable(oss.str());
        std::set<std::string>& prev = g_prev[n->GetId()];
        if (g_baseline)
        {
            uint32_t d = 0;
            for (auto& s : cur) if (!prev.count(s)) d++;   // added / modified entry
            for (auto& s : prev) if (!cur.count(s)) d++;   // removed / modified entry
            g_updTotal += d;
            if (Simulator::Now().GetSeconds() >= g_failTime) g_updAfterFail += d;
        }
        prev = cur;
    }
    g_baseline = true;
    if (Simulator::Now().GetSeconds() < g_simTime)
        Simulator::Schedule(Seconds(0.1), &SampleTables, nodes);
}

// ---------------- link failure helpers ----------------
static void SetLinkBroken(Ptr<RateErrorModel> a, Ptr<RateErrorModel> b, bool broken)
{
    if (broken) { a->Enable(); b->Enable(); }
    else        { a->Disable(); b->Disable(); }
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "s: link r1-r3 " << (broken ? "FAILED" : "RESTORED"));
}
static void SetIfState(Ptr<Ipv4> ip1, uint32_t if1, Ptr<Ipv4> ip2, uint32_t if2, bool up)
{   // used only for the OSPF (global routing) proxy = "dead interval expired"
    if (up) { ip1->SetUp(if1); ip2->SetUp(if2); }
    else    { ip1->SetDown(if1); ip2->SetDown(if2); }
}

static void RxF1(Ptr<const Packet> p, const Address&)
{
    double t = Simulator::Now().GetSeconds();
    g_rxTimes.push_back(t);
    uint32_t bin = (uint32_t)t;
    if (bin < g_binBytes.size()) { g_binBytes[bin] += p->GetSize(); g_binPkts[bin]++; }
}

// ---------------- CBR/UDP application ----------------
static Ptr<Application>
AddCbr(Ptr<Node> src, Ptr<Node> dst, Ipv4Address dstAddr, uint16_t port,
       const std::string& rate, double start, double stop, double simTime)
{
    PacketSinkHelper sinkH("ns3::UdpSocketFactory",
                           InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer s = sinkH.Install(dst);
    s.Start(Seconds(0)); s.Stop(Seconds(simTime));

    OnOffHelper on("ns3::UdpSocketFactory", InetSocketAddress(dstAddr, port));
    on.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    on.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    on.SetAttribute("DataRate", StringValue(rate));
    on.SetAttribute("PacketSize", UintegerValue(1000));
    ApplicationContainer a = on.Install(src);
    a.Start(Seconds(start)); a.Stop(Seconds(stop));
    return s.Get(0);
}

int main(int argc, char* argv[])
{
    std::string proto = "rip";          // rip | ospf | aodv
    std::string scenario = "normal";    // normal | congestion | failure
    std::string outDir = "results";
    double simTime = 60.0, restoreTime = 40.0;
    double appStart = 5.0, appStop = 58.0;
    double ospfDead = 4.0, ospfHello = 1.0;   // OSPF proxy timers
    double ripUpdate = 5.0, ripTimeout = 15.0;
    double aodvHello = 1.0; uint32_t aodvLoss = 2;

    CommandLine cmd(__FILE__);
    cmd.AddValue("proto", "rip|ospf|aodv", proto);
    cmd.AddValue("scenario", "normal|congestion|failure", scenario);
    cmd.AddValue("simTime", "Simulation time (s)", simTime);
    cmd.AddValue("failTime", "Link failure time (s)", g_failTime);
    cmd.AddValue("restoreTime", "Link restore time (s), <0 = never", restoreTime);
    cmd.AddValue("outDir", "Output directory", outDir);
    cmd.Parse(argc, argv);
    g_simTime = simTime;
    g_binBytes.assign((uint32_t)simTime + 1, 0);
    g_binPkts.assign((uint32_t)simTime + 1, 0);

    Config::SetDefault("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue(true));

    // ---------------- nodes ----------------
    NodeContainer all; all.Create(6);
    Ptr<Node> p1 = all.Get(0), p2 = all.Get(1), p3 = all.Get(2);
    Ptr<Node> r1 = all.Get(3), r2 = all.Get(4), r3 = all.Get(5);
    NodeContainer hosts(p1, p2, p3), routers(r1, r2, r3);

    auto mk = [](Ptr<Node> a, Ptr<Node> b, std::string rate, std::string delay) {
        PointToPointHelper p;
        p.SetDeviceAttribute("DataRate", StringValue(rate));
        p.SetChannelAttribute("Delay", StringValue(delay));
        return p.Install(a, b);
    };
    NetDeviceContainer d1 = mk(p1, r1, "100Mbps", "2ms");
    NetDeviceContainer d2 = mk(r1, r2, "10Mbps", "5ms");
    NetDeviceContainer d3 = mk(r2, r3, "10Mbps", "5ms");
    NetDeviceContainer d4 = mk(r3, p3, "100Mbps", "2ms");
    NetDeviceContainer d5 = mk(r2, p2, "100Mbps", "2ms");
    NetDeviceContainer d6 = mk(r1, r3, "5Mbps", "5ms");     // bottleneck + failing link

    // Receive error models used to "cut" r1-r3 (disabled until failure)
    Ptr<RateErrorModel> em1 = CreateObject<RateErrorModel>();
    Ptr<RateErrorModel> em2 = CreateObject<RateErrorModel>();
    for (auto em : {em1, em2})
    {
        em->SetAttribute("ErrorUnit", EnumValue(RateErrorModel::ERROR_UNIT_PACKET));
        em->SetAttribute("ErrorRate", DoubleValue(1.0));
        em->Disable();
    }
    d6.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em1));
    d6.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em2));

    // ---------------- routing protocol + stack ----------------
    // Interface indices on routers (assignment order below): 
    // r1: if1=p1, if2=r2, if3=r3 | r2: if1=r1, if2=r3, if3=p2 | r3: if1=r2, if2=p3, if3=r1
    Ipv4StaticRoutingHelper staticRh;
    if (proto == "rip")
    {
        RipHelper rip;
        rip.Set("UnsolicitedRoutingUpdate", TimeValue(Seconds(ripUpdate)));
        rip.Set("TimeoutDelay", TimeValue(Seconds(ripTimeout)));
        rip.Set("GarbageCollectionDelay", TimeValue(Seconds(10)));
        rip.Set("StartupDelay", TimeValue(Seconds(0.5)));
        rip.Set("MinTriggeredCooldown", TimeValue(Seconds(0.5)));
        rip.Set("MaxTriggeredCooldown", TimeValue(Seconds(1.0)));
        rip.Set("SplitHorizon", EnumValue(RipNg::POISON_REVERSE));
        rip.ExcludeInterface(r1, 1);   // host-facing interfaces
        rip.ExcludeInterface(r2, 3);
        rip.ExcludeInterface(r3, 2);
        Ipv4ListRoutingHelper list;
        list.Add(staticRh, 0);
        list.Add(rip, 10);
        InternetStackHelper rs; rs.SetRoutingHelper(list); rs.Install(routers);
        InternetStackHelper hs; hs.Install(hosts);
    }
    else if (proto == "aodv")
    {
        AodvHelper aodv;
        aodv.Set("HelloInterval", TimeValue(Seconds(aodvHello)));
        aodv.Set("AllowedHelloLoss", UintegerValue(aodvLoss));
        InternetStackHelper s; s.SetRoutingHelper(aodv); s.Install(all);
    }
    else // ospf proxy: global (link-state / SPF) routing
    {
        InternetStackHelper s; s.Install(all);
    }

    // ---------------- IP addressing ----------------
    Ipv4AddressHelper a;
    a.SetBase("10.0.1.0", "255.255.255.0"); Ipv4InterfaceContainer i1 = a.Assign(d1);
    a.SetBase("10.0.2.0", "255.255.255.0"); Ipv4InterfaceContainer i2 = a.Assign(d2);
    a.SetBase("10.0.3.0", "255.255.255.0"); Ipv4InterfaceContainer i3 = a.Assign(d3);
    a.SetBase("10.0.4.0", "255.255.255.0"); Ipv4InterfaceContainer i4 = a.Assign(d4);
    a.SetBase("10.0.5.0", "255.255.255.0"); Ipv4InterfaceContainer i5 = a.Assign(d5);
    a.SetBase("10.0.6.0", "255.255.255.0"); Ipv4InterfaceContainer i6 = a.Assign(d6);
    NS_ASSERT(i1.Get(1).second == 1 && i5.Get(0).second == 3 && i4.Get(0).second == 2);

    if (proto == "rip")   // hosts use default gateway = their router
    {
        staticRh.GetStaticRouting(p1->GetObject<Ipv4>())->SetDefaultRoute(i1.GetAddress(1), i1.Get(0).second);
        staticRh.GetStaticRouting(p2->GetObject<Ipv4>())->SetDefaultRoute(i5.GetAddress(0), i5.Get(1).second);
        staticRh.GetStaticRouting(p3->GetObject<Ipv4>())->SetDefaultRoute(i4.GetAddress(0), i4.Get(1).second);
    }
    if (proto == "ospf")
        Simulator::Schedule(Seconds(0.01), &Ipv4GlobalRoutingHelper::PopulateRoutingTables);

    // ---------------- applications ----------------
    Ipv4Address ip_p3 = i4.GetAddress(1), ip_p2 = i5.GetAddress(1);
    Ptr<Application> sinkF1 = AddCbr(p1, p3, ip_p3, 9001, "1Mbps",   appStart,        appStop, simTime); // F1
    AddCbr(p2, p3, ip_p3, 9002, "1Mbps",   appStart + 0.01, appStop, simTime);                           // F2
    AddCbr(p1, p2, ip_p2, 9003, "500kbps", appStart + 0.02, appStop, simTime);                           // F3
    sinkF1->TraceConnectWithoutContext("Rx", MakeCallback(&RxF1));

    if (scenario == "congestion")
    {
        for (int k = 0; k < 4; ++k)   // extra p1->p3 flows: saturate 5 Mbps bottleneck r1-r3
            AddCbr(p1, p3, ip_p3, 9004 + k, "1500kbps", appStart + 0.03 + 0.01 * k, appStop, simTime);
        for (int k = 0; k < 2; ++k)   // extra p2->p3 flows
            AddCbr(p2, p3, ip_p3, 9008 + k, "2Mbps", appStart + 0.08 + 0.01 * k, appStop, simTime);
    }

    // ---------------- link failure ----------------
    if (scenario == "failure")
    {
        Simulator::Schedule(Seconds(g_failTime), &SetLinkBroken, em1, em2, true);
        if (restoreTime > 0)
            Simulator::Schedule(Seconds(restoreTime), &SetLinkBroken, em1, em2, false);
        if (proto == "ospf")  // link-state detects via dead interval, then SPF reruns automatically
        {
            Ptr<Ipv4> ip1 = r1->GetObject<Ipv4>(), ip3 = r3->GetObject<Ipv4>();
            Simulator::Schedule(Seconds(g_failTime + ospfDead), &SetIfState, ip1, i6.Get(0).second, ip3, i6.Get(1).second, false);
            if (restoreTime > 0)
                Simulator::Schedule(Seconds(restoreTime + ospfHello), &SetIfState, ip1, i6.Get(0).second, ip3, i6.Get(1).second, true);
        }
    }

    // ---------------- monitoring ----------------
    FlowMonitorHelper fmh;
    Ptr<FlowMonitor> mon = fmh.InstallAll();
    Simulator::Schedule(Seconds(0.0), &SampleTables, all);

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    // ---------------- results ----------------
    mon->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> cls = DynamicCast<Ipv4FlowClassifier>(fmh.GetClassifier());
    double tx = 0, rx = 0, rxBytes = 0, delaySum = 0, jitSum = 0, jitN = 0;
    double ctrlPkts = 0, ctrlBytes = 0, f1tx = 0, f1rx = 0, f1delay = 0;
    for (auto& kv : mon->GetFlowStats())
    {
        Ipv4FlowClassifier::FiveTuple t = cls->FindFlow(kv.first);
        uint16_t dp = t.destinationPort;
        if (dp == 520 || dp == 654)       // RIP / AODV control traffic
        {
            ctrlPkts += kv.second.txPackets; ctrlBytes += kv.second.txBytes; continue;
        }
        if (dp < 9001 || dp > 9099) continue;
        tx += kv.second.txPackets; rx += kv.second.rxPackets; rxBytes += kv.second.rxBytes;
        delaySum += kv.second.delaySum.GetSeconds();
        if (kv.second.rxPackets > 1) { jitSum += kv.second.jitterSum.GetSeconds(); jitN += kv.second.rxPackets - 1; }
        if (dp == 9001)
        {
            f1tx = kv.second.txPackets; f1rx = kv.second.rxPackets;
            f1delay = f1rx > 0 ? kv.second.delaySum.GetSeconds() * 1000 / f1rx : 0;
        }
    }
    double pdr = tx > 0 ? 100.0 * rx / tx : 0;
    double avgDelay = rx > 0 ? delaySum * 1000 / rx : 0;
    double avgJit = jitN > 0 ? jitSum * 1000 / jitN : 0;
    double thr = rxBytes * 8 / (appStop - appStart) / 1000.0;
    double f1pdr = f1tx > 0 ? 100.0 * f1rx / f1tx : 0;

    double initConv = -1, maxGap = simTime;
    if (!g_rxTimes.empty())
    {
        initConv = g_rxTimes.front() - appStart;
        maxGap = 0;
        for (size_t i = 1; i < g_rxTimes.size(); ++i)
            maxGap = std::max(maxGap, g_rxTimes[i] - g_rxTimes[i - 1]);
    }

    std::string csv = outDir + "/results.csv";
    bool exists = std::ifstream(csv).good();
    std::ofstream o(csv, std::ios::app);
    if (!exists)
        o << "protocol,scenario,tx_pkts,rx_pkts,lost_pkts,pdr_pct,avg_delay_ms,avg_jitter_ms,"
             "throughput_kbps,f1_pdr_pct,f1_delay_ms,route_updates,route_updates_after_failtime,"
             "ctrl_pkts,ctrl_bytes,initial_conv_s,max_gap_s\n";
    o << proto << "," << scenario << "," << tx << "," << rx << "," << (tx - rx) << "," << pdr << ","
      << avgDelay << "," << avgJit << "," << thr << "," << f1pdr << "," << f1delay << ","
      << g_updTotal << "," << g_updAfterFail << "," << ctrlPkts << "," << ctrlBytes << ","
      << initConv << "," << maxGap << "\n";
    o.close();

    std::ofstream ts(outDir + "/ts_" + proto + "_" + scenario + ".csv");
    ts << "time_s,f1_rx_pkts,f1_throughput_kbps\n";
    for (uint32_t b = 0; b < (uint32_t)simTime; ++b)
        ts << b << "," << g_binPkts[b] << "," << g_binBytes[b] * 8 / 1000.0 << "\n";

    std::cout << "[" << proto << "/" << scenario << "] PDR=" << pdr << "% delay=" << avgDelay
              << "ms jitter=" << avgJit << "ms thr=" << thr << "kbps routeUpd=" << g_updTotal
              << " ctrlPkts=" << ctrlPkts << " maxGap=" << maxGap << "s\n";

    Simulator::Destroy();
    return 0;
}
