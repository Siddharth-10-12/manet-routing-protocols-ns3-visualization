#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/olsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MyOlsrSimulation");

int main (int argc, char *argv[])
{
    uint32_t nNodes = 10;
    double simTime = 30.0; // seconds

    CommandLine cmd;
    cmd.AddValue ("nNodes", "Number of nodes", nNodes);
    cmd.AddValue ("simTime", "Simulation time in seconds", simTime);
    cmd.Parse (argc, argv);

    // Create nodes
    NodeContainer nodes;
    nodes.Create (nNodes);

    // Wifi PHY + channel
    WifiHelper wifi;
    wifi.SetStandard ("802.11b");

    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    wifiPhy.SetChannel (wifiChannel.Create ());

    WifiMacHelper wifiMac;
    wifiMac.SetType ("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);

    // Mobility - Random Movement
    MobilityHelper mobility;

    Ptr<UniformRandomVariable> randX = CreateObject<UniformRandomVariable> ();
    randX->SetAttribute ("Min", DoubleValue (0.0));
    randX->SetAttribute ("Max", DoubleValue (500.0));

    Ptr<UniformRandomVariable> randY = CreateObject<UniformRandomVariable> ();
    randY->SetAttribute ("Min", DoubleValue (0.0));
    randY->SetAttribute ("Max", DoubleValue (500.0));

    Ptr<PositionAllocator> positionAlloc = CreateObject<RandomRectanglePositionAllocator> ();
    positionAlloc->SetAttribute ("X", PointerValue (randX));
    positionAlloc->SetAttribute ("Y", PointerValue (randY));

    mobility.SetPositionAllocator (positionAlloc);
    mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                               "Speed", StringValue ("ns3::UniformRandomVariable[Min=1.0|Max=5.0]"),
                               "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=0.5]"),
                               "PositionAllocator", PointerValue (positionAlloc));
    mobility.Install (nodes);

    // Internet stack + OLSR routing
    InternetStackHelper internet;
    OlsrHelper olsr;
    Ipv4ListRoutingHelper list;
    list.Add (olsr, 100);
    internet.SetRoutingHelper (list);
    internet.Install (nodes);

    // IP addresses
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

    // UDP OnOff application: Node 0 -> Node 1
    uint16_t port = 9;
    OnOffHelper onoff ("ns3::UdpSocketFactory",
                       InetSocketAddress (interfaces.GetAddress (1), port));
    onoff.SetConstantRate (DataRate ("10Mbps")); // Increased rate to generate ~500–550 packets
    onoff.SetAttribute ("PacketSize", UintegerValue (512)); // Smaller packet size
    ApplicationContainer apps = onoff.Install (nodes.Get (0));
    apps.Start (Seconds (0.0)); // start immediately
    apps.Stop (Seconds (simTime));

    // Packet sink on Node 1
    PacketSinkHelper sink ("ns3::UdpSocketFactory",
                           InetSocketAddress (Ipv4Address::GetAny (), port));
    apps = sink.Install (nodes.Get (1));
    apps.Start (Seconds (0.0));
    apps.Stop (Seconds (simTime));

    // NetAnim animation
    AnimationInterface anim("my_olsr.xml"); // generate XML file
    anim.SetMaxPktsPerTraceFile (50000);

    for (uint32_t i = 0; i < nodes.GetN (); ++i)
    {
        std::string desc = std::to_string(i); // node number displayed as label

        if (i == 0) { // Sender
            anim.UpdateNodeColor(nodes.Get(i), 255, 0, 0);  // Red
        }
        else if (i == 1) { // Receiver
            anim.UpdateNodeColor(nodes.Get(i), 255, 255, 0); // Yellow
        }
        else if (i % 5 == 0) { // MPR node
            anim.UpdateNodeColor(nodes.Get(i), 0, 255, 0);  // Green
        }
        else { // Regular node
            anim.UpdateNodeColor(nodes.Get(i), 0, 0, 255);  // Blue
        }

        anim.UpdateNodeDescription(nodes.Get(i), desc);     // Display node number
    }

    Simulator::Stop (Seconds (simTime));
    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
