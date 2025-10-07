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

void
TxTrace (std::string context, Ptr<const Packet> p)
{
  outFile << Simulator::Now ().GetSeconds () << ",Tx," << p->GetSize () << std::endl;
}

void
RxTrace (std::string context, Ptr<const Packet> p, const Address &addr)
{
  outFile << Simulator::Now ().GetSeconds () << ",Rx," << p->GetSize () << std::endl;
}

int
main (int argc, char *argv[])
{
  // Open CSV file
  outFile.open ("packets2.csv");
  outFile << "Time,Type,Size" << std::endl;

  // Create 4 nodes
  NodeContainer nodes;
  nodes.Create (4);

  // Point-to-point helper
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("10ms"));

  // Connect nodes in a chain: n0--n1--n2--n3
  NetDeviceContainer d01 = p2p.Install (nodes.Get (0), nodes.Get (1));
  NetDeviceContainer d12 = p2p.Install (nodes.Get (1), nodes.Get (2));
  NetDeviceContainer d23 = p2p.Install (nodes.Get (2), nodes.Get (3));

  // Install internet + AODV
  InternetStackHelper internet;
  AodvHelper aodv;
  Ipv4ListRoutingHelper list;
  list.Add (aodv, 100);
  internet.SetRoutingHelper (list);
  internet.Install (nodes);

  // Assign IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i01 = ipv4.Assign (d01);

  ipv4.SetBase ("10.0.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i12 = ipv4.Assign (d12);

  ipv4.SetBase ("10.0.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i23 = ipv4.Assign (d23);

  // Install applications
  uint16_t port = 9;
  OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (i23.GetAddress (1), port)));
  onoff.SetConstantRate (DataRate ("500kbps"));
  onoff.SetAttribute ("PacketSize", UintegerValue (1024));
  ApplicationContainer apps = onoff.Install (nodes.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (9.0));

  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  apps = sink.Install (nodes.Get (3));
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (10.0));

  // Tracing TX and RX
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::OnOffApplication/Tx", MakeCallback (&TxTrace));
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback (&RxTrace));

  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();

  outFile.close ();
  return 0;
}
