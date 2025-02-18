#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/energy-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/buildings-helper.h"
#include "ns3/yans-error-rate-model.h"
#include <cmath>

using namespace ns3;
using namespace ns3::energy; // So we can use EnergySourceContainer etc. without full qualification

// Custom tracking variables
double totalOutOfCoverageTime = 0.0;
uint32_t totalPacketsSent = 0;
uint32_t totalPacketsLost = 0;
double totalEnergyConsumed = 0.0;
double maxBufferBeforeSync = 0.0;

// Configure these parameters
const int NUM_AP = 3;                  // Number of WiFi access points
const int NUM_NODES = 10;              // Number of mobile nodes
const double COVERAGE_RADIUS = 50.0;   // Meters
const double SIM_TIME = 300.0;         // Seconds

// Helper function to calculate Euclidean distance
double CustomCalculateDistance(const Vector &a, const Vector &b) {
  double dx = a.x - b.x;
  double dy = a.y - b.y;
  double dz = a.z - b.z;
  return std::sqrt(dx*dx + dy*dy + dz*dz);
}

// Track node position and accumulate out-of-coverage time
void TrackNodePosition(Ptr<Node> node) {
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
  Vector pos = mobility->GetPosition();

  bool inCoverage = false;
  // Check against all AP nodes (assumed to be nodes 0 to NUM_AP-1 in NodeList)
  for (uint32_t i = 0; i < NUM_AP; ++i) {
    Ptr<Node> apNode = NodeList::GetNode(i);
    Vector apPos = apNode->GetObject<MobilityModel>()->GetPosition();
    if (CustomCalculateDistance(pos, apPos) <= COVERAGE_RADIUS) {
      inCoverage = true;
      break;
    }
  }

  if (!inCoverage) {
    totalOutOfCoverageTime += 0.1; // Update every 100ms
    maxBufferBeforeSync += 0.1 * 0.05; // Example accumulation rate
  } else {
    maxBufferBeforeSync = 0;
  }

  Simulator::Schedule(Seconds(0.1), &TrackNodePosition, node);
}

// Updated callback matching the expected signature: (double oldEnergy, double newEnergy)
void EnergyConsumptionCallback(double oldEnergy, double newEnergy) {
  NS_LOG_UNCOND("Energy changed from " << oldEnergy 
                << " J to " << newEnergy 
                << " J at " << Simulator::Now().GetSeconds() << " s");
}

int main(int argc, char *argv[]) {
  uint32_t seed = static_cast<uint32_t>(std::time(nullptr)); // Default to current time

  // Set simulation parameters
  double simTime = SIM_TIME;
  CommandLine cmd;
  cmd.AddValue("seed", "Random seed value", seed);
  cmd.AddValue("simTime", "Total duration of the simulation", simTime);
  cmd.Parse(argc, argv);

  RngSeedManager::SetSeed(seed);
  std::cout << "Using seed: " << seed << std::endl;

  // WiFi configuration defaults
  Config::SetDefault("ns3::RangePropagationLossModel::MaxRange", DoubleValue(COVERAGE_RADIUS));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiPhy::CcaEdThreshold", DoubleValue(-62.0));

  // Create AP and station nodes
  NodeContainer apNodes;
  NodeContainer staNodes;
  apNodes.Create(NUM_AP);
  staNodes.Create(NUM_NODES);

  // Mobility for APs (grid) and stations (random)
  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                "MinX", DoubleValue(0.0),
                                "MinY", DoubleValue(0.0),
                                "DeltaX", DoubleValue(COVERAGE_RADIUS * 2),
                                "DeltaY", DoubleValue(COVERAGE_RADIUS * 2),
                                "GridWidth", UintegerValue(NUM_AP),
                                "LayoutType", StringValue("RowFirst"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(apNodes);
  BuildingsHelper::Install(apNodes);

  mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=200]"),
                                "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=200]"));
  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                            "Bounds", RectangleValue(Rectangle(0, 200, 0, 200)),
                            "Speed", StringValue("ns3::UniformRandomVariable[Min=1|Max=2]"));
  mobility.Install(staNodes);

  // Configure WiFi: create channel, set error model on channel pointer
  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211n);

  YansWifiChannelHelper channelHelper = YansWifiChannelHelper::Default();
  channelHelper.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                                 "Exponent", DoubleValue(3.0));
  Ptr<YansWifiChannel> wifiChannel = channelHelper.Create();

  YansWifiPhyHelper phy;
  phy.SetErrorRateModel("ns3::YansErrorRateModel");
  phy.SetChannel(wifiChannel);

  WifiMacHelper mac;
  Ssid ssid = Ssid("ns3-wifi");

  // Energy model configuration
  BasicEnergySourceHelper energySource;
  energySource.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(10000.0));
  WifiRadioEnergyModelHelper radioEnergyHelper;
  radioEnergyHelper.Set("TxCurrentA", DoubleValue(0.3));   // 300 mA
  radioEnergyHelper.Set("RxCurrentA", DoubleValue(0.1));   // 100 mA
  radioEnergyHelper.Set("IdleCurrentA", DoubleValue(0.05)); // 50 mA

  // Install WiFi devices
  NetDeviceContainer apDevices;
  NetDeviceContainer staDevices;

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
  apDevices = wifi.Install(phy, mac, apNodes);

  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
  staDevices = wifi.Install(phy, mac, staNodes);

  // Install energy models on stations
  EnergySourceContainer energySources;
  for (uint32_t i = 0; i < staNodes.GetN(); ++i) {
    energySources.Add(energySource.Install(staNodes.Get(i)));
  }
  DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install(staDevices, energySources);

  // Internet stack and IP addresses
  InternetStackHelper stack;
  stack.Install(apNodes);
  stack.Install(staNodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterfaces = address.Assign(apDevices);
  Ipv4InterfaceContainer staInterfaces = address.Assign(staDevices);

  // Applications: install UDP server on stations, client on first AP
  ApplicationContainer serverApps;
  ApplicationContainer clientApps;

  uint16_t port = 9;
  UdpServerHelper server(port);
  serverApps = server.Install(staNodes);
  serverApps.Start(Seconds(0.0));
  serverApps.Stop(Seconds(simTime));

  UdpClientHelper client(staInterfaces.GetAddress(0), port);
  client.SetAttribute("MaxPackets", UintegerValue(UINT32_MAX));
  client.SetAttribute("Interval", TimeValue(Seconds(0.1)));
  client.SetAttribute("PacketSize", UintegerValue(1024));

  clientApps = client.Install(apNodes.Get(0));
  clientApps.Start(Seconds(1.0));
  clientApps.Stop(Seconds(simTime - 1));

  // Schedule tracking of station positions
  for (uint32_t i = 0; i < staNodes.GetN(); ++i) {
    Simulator::Schedule(Seconds(0.1), &TrackNodePosition, staNodes.Get(i));
  }

  // Flow monitor configuration
  FlowMonitorHelper flowHelper;
  Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

  // Calculate flow metrics
  monitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();
  Time totalDelay = Seconds(0);
  for (auto const& stat : stats) {
    totalDelay += stat.second.delaySum;
    totalPacketsSent += stat.second.txPackets;
    totalPacketsLost += stat.second.lostPackets;
  }

  // Energy calculation: sum energy consumed (convert Joules to Watt-hour)
  for (EnergySourceContainer::Iterator it = energySources.Begin(); it != energySources.End(); ++it) {
    totalEnergyConsumed += ((10000.0 - (*it)->GetRemainingEnergy()) / 3600.0);
  }

  // Output results to stdout
  std::cout << "\n=== Simulation Results ===\n"
            << "Total energy consumed: " << totalEnergyConsumed << " Wh\n"
            << "Out-of-coverage time: " << totalOutOfCoverageTime << " seconds\n"
            << "Packet loss rate: " << (totalPacketsLost * 100.0 / totalPacketsSent) << "%\n"
            << "Average packet delay: " << (totalDelay.GetMilliSeconds() / totalPacketsSent) << " ms\n"
            << "Max data stored before sync: " << (maxBufferBeforeSync / 1000) << " MB\n";

  // Connect energy trace callback
  for (EnergySourceContainer::Iterator it = energySources.Begin(); it != energySources.End(); ++it) {
    (*it)->TraceConnectWithoutContext("RemainingEnergy", MakeCallback(&EnergyConsumptionCallback));
  }

  Simulator::Destroy();
  return 0;
}
