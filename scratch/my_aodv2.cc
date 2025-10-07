#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include <fstream>

using namespace ns3;

// File streams
std::ofstream posFile("positions.csv");
std::ofstream pktFile("packets.csv");

// Position trace function
void PositionTrace(uint32_t nodeId, Vector oldPos, Vector newPos)
{
    posFile << Simulator::Now().GetSeconds() << "," << nodeId << ","
            << newPos.x << "," << newPos.y << std::endl;
}

// Packet Tx trace
void TxTrace(Ptr<const Packet> pkt, const Address &addr)
{
    pktFile << Simulator::Now().GetSeconds() << ",Tx,0,Server," << pkt->GetSize() << std::endl;
}

// Packet Rx trace
void RxTrace(Ptr<const Packet> pkt, const Address &addr)
{
    pktFile << Simulator::Now().GetSeconds() << ",Rx,Server,0," << pkt->GetSize() << std::endl;
}

int main(int argc, char *argv[])
{
    NodeContainer nodes;
    nodes.Create(2);

    // Mobility model with RandomWalk so positions change
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(0,50,0,50)),
                              "Speed", StringValue("ns3::UniformRandomVariable[Min=1.0|Max=5.0]"));
    mobility.Install(nodes);

    // Internet stack with AODV
    AodvHelper aodv;
    InternetStackHelper stack;
    stack.SetRoutingHelper(aodv);
    stack.Install(nodes);

    // Connect position trace
    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        Ptr<MobilityModel> mob = nodes.Get(i)->GetObject<MobilityModel>();
        if (mob)
        {
            mob->TraceConnect("CourseChange", "",
                              MakeBoundCallback(&PositionTrace, i));
        }
    }

    // UDP server on node 1
    uint16_t port = 9;
    UdpServerHelper server(port);
    ApplicationContainer serverApp = server.Install(nodes.Get(1));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // UDP client on node 0
    UdpClientHelper client(nodes.Get(1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), port);
    client.SetAttribute("MaxPackets", UintegerValue(20));
    client.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    client.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApp = client.Install(nodes.Get(0));
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));

    // Packet trace
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
