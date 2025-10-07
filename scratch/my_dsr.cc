#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"
#include <fstream>

using namespace ns3;

std::ofstream outFile;

void TxTrace(Ptr<const Packet> p) {
    outFile << Simulator::Now().GetSeconds() << ",Tx," << p->GetSize() << std::endl;
}

void RxTrace(Ptr<const Packet> p) {
    outFile << Simulator::Now().GetSeconds() << ",Rx," << p->GetSize() << std::endl;
}

int main() {
    outFile.open("packets_dsr.csv");
    outFile << "Time,Type,Size" << std::endl;

    NodeContainer nodes;
    nodes.Create(4);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("10ms"));

    NetDeviceContainer d01 = p2p.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer d12 = p2p.Install(nodes.Get(1), nodes.Get(2));
    NetDeviceContainer d23 = p2p.Install(nodes.Get(2), nodes.Get(3));

    InternetStackHelper internet;
    DsrHelper dsr;
    DsrMainHelper dsrMain;
    internet.Install(nodes);
    dsrMain.Install(dsr, nodes);

    // Assign IPs
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.1.0", "255.255.255.0"); ipv4.Assign(d01);
    ipv4.SetBase("10.0.2.0", "255.255.255.0"); ipv4.Assign(d12);
    ipv4.SetBase("10.0.3.0", "255.255.255.0"); Ipv4InterfaceContainer i23 = ipv4.Assign(d23);

    uint16_t port = 8080;

    // PacketSink at last node
    PacketSinkHelper sinkHelper("ns3::UdpSocketFactory",
                                InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = sinkHelper.Install(nodes.Get(3));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(20.0));

    // OnOff traffic from first node TO last node
    OnOffHelper client("ns3::UdpSocketFactory",
                       InetSocketAddress(i23.GetAddress(1), port));
    client.SetConstantRate(DataRate("500kbps"));
    client.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApp = client.Install(nodes.Get(0));
    clientApp.Start(Seconds(2.0)); // Wait 2s for DSR to establish routes
    clientApp.Stop(Seconds(18.0));

    // Trace Tx/Rx at NetDevice level
    for (uint32_t i = 0; i < nodes.GetN(); ++i) {
        Ptr<Node> node = nodes.Get(i);
        for (uint32_t j = 0; j < node->GetNDevices(); ++j) {
            Ptr<NetDevice> dev = node->GetDevice(j);
            dev->TraceConnectWithoutContext("MacTx", MakeCallback(&TxTrace));
            dev->TraceConnectWithoutContext("MacRx", MakeCallback(&RxTrace));
        }
    }

    Simulator::Stop(Seconds(20.0));
    Simulator::Run();
    Simulator::Destroy();
    outFile.close();

    return 0;
}
