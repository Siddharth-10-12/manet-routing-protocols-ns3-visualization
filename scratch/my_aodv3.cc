#include "ns3/point-to-point-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include <fstream>

using namespace ns3;

std::ofstream posFile("positions.csv");
std::ofstream pktFile("packets.csv");

// Trace function for positions
void PositionTrace(Ptr<const MobilityModel> mob) {
    Vector pos = mob->GetPosition();
    posFile << Simulator::Now().GetSeconds() << "," 
            << mob->GetObject<Node>()->GetId() << ","
            << pos.x << "," << pos.y << std::endl;
}

// Trace function for packet Tx
void TxTrace(Ptr<const Packet> pkt) {
    pktFile << Simulator::Now().GetSeconds() << ",Tx," 
            << pkt->GetSize() << std::endl;
}

// Trace function for packet Rx
void RxTrace(Ptr<const Packet> pkt) {
    pktFile << Simulator::Now().GetSeconds() << ",Rx," 
            << pkt->GetSize() << std::endl;
}

int main(int argc, char *argv[]) {
    // Create nodes
    NodeContainer nodes;
    nodes.Create(2);

    // Mobility
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Internet stack and AODV
    AodvHelper aodv;
    InternetStackHelper stack;
    stack.SetRoutingHelper(aodv);
    stack.Install(nodes);

    // Assign IP addresses
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer devices = p2p.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    ipv4.Assign(devices);

    // Trace node positions
    for (uint32_t i = 0; i < nodes.GetN(); ++i) {
        Ptr<MobilityModel> mob = nodes.Get(i)->GetObject<MobilityModel>();
        mob->TraceConnectWithoutContext("CourseChange",
            MakeCallback(&PositionTrace));
    }

    // UDP server on node 1
    uint16_t port = 9;
    UdpServerHelper server(port);
    ApplicationContainer serverApp = server.Install(nodes.Get(1));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // UDP client on node 0
    UdpClientHelper client(Ipv4Address("10.1.1.2"), port);
    client.SetAttribute("MaxPackets", UintegerValue(100));
    client.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    client.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApp = client.Install(nodes.Get(0));
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));

    // Packet traces
    Ptr<UdpClient> clientPtr = DynamicCast<UdpClient>(clientApp.Get(0));
    clientPtr->TraceConnectWithoutContext("Tx", MakeCallback(&TxTrace));
    Ptr<UdpServer> serverPtr = DynamicCast<UdpServer>(serverApp.Get(0));
    serverPtr->TraceConnectWithoutContext("Rx", MakeCallback(&RxTrace));

    Simulator::Stop(Seconds(12.0));
    Simulator::Run();
    Simulator::Destroy();

    posFile.close();
    pktFile.close();
    return 0;
}
