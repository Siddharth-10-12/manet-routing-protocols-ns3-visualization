#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/olsr-module.h"
#include "ns3/applications-module.h"
#include <fstream>
#include "ns3/netanim-module.h"

using namespace ns3;

std::ofstream outFile;

void TxTrace(Ptr<const Packet> p) {
    outFile << Simulator::Now().GetSeconds() << ",Tx," << p->GetSize() << std::endl;
}

void RxTrace(Ptr<const Packet> p) {
    outFile << Simulator::Now().GetSeconds() << ",Rx," << p->GetSize() << std::endl;
}

int main() {
    outFile.open("packets_olsr.csv");
    outFile << "Time,Type,Size" << std::endl;

    NodeContainer nodes;
    nodes.Create(4);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("10ms"));

    NetDeviceContainer d01 = p2p.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer d12 = p2p.Install(nodes.Get(1), nodes.Get(2));
    NetDeviceContainer d23 = p2p.Install(nodes.Get(2), nodes.Get(3));

    InternetStackHelper stack;
    OlsrHelper olsr;
    stack.SetRoutingHelper(olsr);
    stack.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.1.0", "255.255.255.0"); ipv4.Assign(d01);
    ipv4.SetBase("10.0.2.0", "255.255.255.0"); ipv4.Assign(d12);
    ipv4.SetBase("10.0.3.0", "255.255.255.0"); ipv4.Assign(d23);

    uint16_t port = 8080;
    PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = sink.Install(nodes.Get(3));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(12.0));

    OnOffHelper onoff("ns3::UdpSocketFactory", InetSocketAddress(nodes.Get(3)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), port));
    onoff.SetConstantRate(DataRate("500kbps"));
    onoff.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApp = onoff.Install(nodes.Get(0));
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));

    // Connect callbacks (single-parameter)
    for (uint32_t i = 0; i < d01.GetN(); ++i) {
        d01.Get(i)->TraceConnectWithoutContext("PhyTxEnd", MakeCallback(&TxTrace));
        d01.Get(i)->TraceConnectWithoutContext("PhyRxEnd", MakeCallback(&RxTrace));
    }
    for (uint32_t i = 0; i < d12.GetN(); ++i) {
        d12.Get(i)->TraceConnectWithoutContext("PhyTxEnd", MakeCallback(&TxTrace));
        d12.Get(i)->TraceConnectWithoutContext("PhyRxEnd", MakeCallback(&RxTrace));
    }
    for (uint32_t i = 0; i < d23.GetN(); ++i) {
        d23.Get(i)->TraceConnectWithoutContext("PhyTxEnd", MakeCallback(&TxTrace));
        d23.Get(i)->TraceConnectWithoutContext("PhyRxEnd", MakeCallback(&RxTrace));
    }

    Simulator::Stop(Seconds(12.0));
    AnimationInterface anim("manet-animation.xml"); // Generates XML for NetAnim
    Simulator::Run();
    Simulator::Destroy();
    outFile.close();
    return 0;
}
