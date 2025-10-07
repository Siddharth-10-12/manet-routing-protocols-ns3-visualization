#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include <fstream>
#include <iostream>

using namespace ns3;

std::ofstream outFile;

void TxTrace (std::string context, Ptr<const Packet> p) {
  outFile << Simulator::Now().GetSeconds() << ",Tx," << p->GetSize() << std::endl;
}

void RxTrace (std::string context, Ptr<const Packet> p, const Address &addr) {
  outFile << Simulator::Now().GetSeconds() << ",Rx," << p->GetSize() << std::endl;
}

int main (int argc, char *argv[]) {
  outFile.open ("packets_aodv.csv");
  outFile << "Time,Type,Size" << std::endl;

  NodeContainer nodes;
  nodes.Create (4);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("10ms"));

  NetDeviceContainer d01 = p2p.Install (nodes.Get(0), nodes.Get(1));
  NetDeviceContainer d12 = p2p.Install (nodes.Get(1), nodes.Get(2));
  NetDeviceContainer d23 = p2p.Install (nodes.Get(2), nodes.Get(3));

  InternetStackHelper internet;
  AodvHelper aodv;
  Ipv4ListRoutingHelper list;
  list.Add (aodv, 100);
  internet.SetRoutingHelper (list);
  internet.Install (nodes);

// ----------------- START REPLACE BLOCK -----------------
Ipv4AddressHelper ipv4;

// Assign IPs and capture interface containers so we can get the last node IP
ipv4.SetBase ("10.0.1.0", "255.255.255.0");
Ipv4InterfaceContainer i01 = ipv4.Assign (d01);

ipv4.SetBase ("10.0.2.0", "255.255.255.0");
Ipv4InterfaceContainer i12 = ipv4.Assign (d12);

ipv4.SetBase ("10.0.3.0", "255.255.255.0");
Ipv4InterfaceContainer i23 = ipv4.Assign (d23);

// Install the application: client on node 0, sink on node 3
uint16_t port = 9;

// Remote address = IP of node 3 on the last link (guaranteed correct)
Address remoteAddress = InetSocketAddress (i23.GetAddress(1), port);

// Client: OnOff sending UDP to last node
OnOffHelper onoff ("ns3::UdpSocketFactory", remoteAddress);
onoff.SetConstantRate (DataRate ("500kbps"));
onoff.SetAttribute ("PacketSize", UintegerValue (1024));
ApplicationContainer clientApp = onoff.Install (nodes.Get (0));
clientApp.Start (Seconds (1.0));
clientApp.Stop (Seconds (9.0));

// Server/Sink: listening on port at node 3
PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
ApplicationContainer sinkApp = sink.Install (nodes.Get (3));
sinkApp.Start (Seconds (0.0));
sinkApp.Stop (Seconds (10.0));

// Connect trace callbacks for Tx and Rx (these paths are standard)
Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::OnOffApplication/Tx", MakeCallback (&TxTrace));
Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback (&RxTrace));
// ----------------- END REPLACE BLOCK -----------------

  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();
  outFile.close ();
  return 0;
}
