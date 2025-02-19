#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>
#include "ns3/energy-module.h"
#include "ns3/device-energy-model.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("NBIoT");

// ==================== LOGGING CALLBACKS ====================
void EnergyConsumptionCallback(double oldEnergy, double newEnergy) {
  NS_LOG_DEBUG(Simulator::Now().GetSeconds() << "s: [ENERGY] Remaining: " 
                << newEnergy << " J (Δ " << newEnergy - oldEnergy << " J)");
}

void PhyTxTrace(std::string context, Ptr<const Packet> p) {
  std::size_t nodePos = context.find("/NodeList/");
  uint32_t nodeId = std::stoi(context.substr(nodePos + 10, 1));

  NS_LOG_DEBUG(Simulator::Now() << " Node " << nodeId 
                << " TX: " << p->GetSize() << " bytes");
}

void PhyRxTrace(std::string context, Ptr<const Packet> p) {
  NS_LOG_DEBUG(Simulator::Now().GetSeconds() << "s: [RX] " << context 
                << " received packet " << p->GetUid() << " (" << p->GetSize() << " bytes)");
}

void DlRxErrorTrace(std::string context, Ptr<const Packet> p, double sinr) {
  NS_LOG_DEBUG(Simulator::Now().GetSeconds() << "s: [ERROR] DL packet " 
                << p->GetUid() << " lost (SINR: " << sinr << " dB)");
}

void UlRxErrorTrace(std::string context, Ptr<const Packet> p, double sinr) {
  NS_LOG_DEBUG(Simulator::Now().GetSeconds() << "s: [ERROR] UL packet " 
                << p->GetUid() << " lost (SINR: " << sinr << " dB)");
}

int main (int argc, char *argv[]) {
  // Enable detailed logging
  LogComponentEnable("LteUePhy", LOG_LEVEL_INFO);
  LogComponentEnable("LteUeMac", LOG_LEVEL_DEBUG);
  LogComponentEnable("LteEnbPhy", LOG_LEVEL_INFO);
  LogComponentEnable("LteEnbMac", LOG_LEVEL_DEBUG);
  LogComponentEnable("LteSpectrumPhy", LOG_LEVEL_DEBUG);
  LogComponentEnable("NBIoT", LOG_LEVEL_ALL);

  // ==================== CLI CONFIGURATION ====================
  Time simTime = Seconds(30);
  uint32_t numUeNodes = 1;
  uint32_t numEnbNodes = 1;
  double packetLossRate = 0.0;
  bool useCa = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("simTime", "Simulation duration", simTime);
  cmd.AddValue("numNodes", "Number of UE nodes", numUeNodes);
  cmd.AddValue("numRadioTowers", "Number of eNBs", numEnbNodes);
  cmd.AddValue("packetLossRate", "Target packet loss rate", packetLossRate);
  cmd.AddValue("useCa", "Enable carrier aggregation", useCa);
  cmd.Parse(argc, argv);

  NS_LOG_INFO("========== Simulation Configuration ==========");
  NS_LOG_INFO("UE Nodes: " << numUeNodes);
  NS_LOG_INFO("eNB Nodes: " << numEnbNodes);
  NS_LOG_INFO("Packet Loss Rate: " << packetLossRate);
  NS_LOG_INFO("Simulation Time: " << simTime.As(Time::S));
  NS_LOG_INFO("Carrier Aggregation: " << (useCa ? "Enabled" : "Disabled"));
  NS_LOG_INFO("==============================================");

  // ==================== LTE CONFIGURATION ====================
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
  lteHelper->SetEpcHelper(epcHelper);

  lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");

  Config::SetDefault("ns3::LteEnbRrc::EpsBearerToRlcMapping", 
                  EnumValue(LteEnbRrc::RLC_UM_ALWAYS));
  Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(46.0));
  Config::SetDefault("ns3::LteUePhy::NoiseFigure", DoubleValue(9.0));

  if (packetLossRate > 0) {
    NS_LOG_INFO("Configuring packet loss model (BER: " << packetLossRate << ")");
    Config::SetDefault("ns3::LteAmc::Ber", DoubleValue(packetLossRate));
    Config::SetDefault("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue(true));
    Config::SetDefault("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue(true));
  }

  if (useCa) {
    NS_LOG_INFO("Configuring carrier aggregation");
    Config::SetDefault("ns3::LteHelper::UseCa", BooleanValue(true));
    Config::SetDefault("ns3::LteHelper::NumberOfComponentCarriers", UintegerValue(2));
    Config::SetDefault("ns3::LteHelper::EnbComponentCarrierManager", 
                      StringValue("ns3::RrComponentCarrierManager"));
  }

  // ==================== NODE CREATION ====================
  NS_LOG_INFO("Creating " << numEnbNodes << " eNB node(s)");
  NodeContainer enbNodes;
  enbNodes.Create(numEnbNodes);

  NS_LOG_INFO("Creating " << numUeNodes << " UE node(s)");
  NodeContainer ueNodes;
  ueNodes.Create(numUeNodes);

  // ==================== MOBILITY ====================
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0, 0, 30));    // eNB position
  positionAlloc->Add(Vector(50, 0, 1.5));  // UE position (50m from eNB)
  mobility.SetPositionAllocator(positionAlloc);

  NS_LOG_INFO("Installing mobility models");
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);
  BuildingsHelper::Install(enbNodes);
  BuildingsHelper::Install(ueNodes);

  // ==================== NETWORK SETUP ====================
  NS_LOG_INFO("Installing LTE devices");
  NetDeviceContainer enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  NetDeviceContainer ueDevs = lteHelper->InstallUeDevice(ueNodes);

  // ==================== TRACE CONNECTIONS ====================
  NS_LOG_INFO("Connecting tracing callbacks");

  // Connect PHY traces for all devices
  for (uint32_t i = 0; i < ueDevs.GetN(); ++i) {
      Ptr<LteUePhy> uePhy = ueDevs.Get(i)->GetObject<LteUeNetDevice>()->GetPhy();
      
      // UE transmission/reception
      uePhy->TraceConnectWithoutContext("Tx", MakeCallback(&PhyTxTrace));
      uePhy->TraceConnectWithoutContext("Rx", MakeCallback(&PhyRxTrace));
  }

  for (uint32_t i = 0; i < enbDevs.GetN(); ++i) {
      Ptr<LteEnbPhy> enbPhy = enbDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetPhy();
      
      // eNB transmission/reception
      enbPhy->TraceConnectWithoutContext("Tx", MakeCallback(&PhyTxTrace));
      enbPhy->TraceConnectWithoutContext("Rx", MakeCallback(&PhyRxTrace));
  }

  // ==================== INTERNET STACK ====================
  NS_LOG_INFO("Installing internet stack on UEs");
  InternetStackHelper internet;
  internet.Install(ueNodes);

  // ==================== EPC SETUP ====================
  NS_LOG_INFO("Setting up EPC and remote host");
  Ptr<Node> pgw = epcHelper->GetPgwNode();

  // Create remote host
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create(1);
  Ptr<Node> remoteHost = remoteHostContainer.Get(0);
  InternetStackHelper internetRemote;
  internetRemote.Install(remoteHostContainer);

  // Create P2P link between PGW and remote host
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
  p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
  p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));
  
  NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);

  // Routing setup
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting(
    remoteHost->GetObject<Ipv4>());
  remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

  // Assign UE IP addresses
  Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address(ueDevs);

  // ==================== ENERGY SETUP ====================
  NS_LOG_INFO("Configuring energy sources");
  BasicEnergySourceHelper energySourceHelper;
  energySourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(10000.0));
  
  EnergySourceContainer energySources;
  energySources.Add(energySourceHelper.Install(enbNodes));
  energySources.Add(energySourceHelper.Install(ueNodes));

  // Connect energy traces
  for (EnergySourceContainer::Iterator it = energySources.Begin(); 
       it != energySources.End(); ++it) {
    DynamicCast<BasicEnergySource>(*it)->TraceConnectWithoutContext(
      "RemainingEnergy", MakeCallback(&EnergyConsumptionCallback));
  }

  // ==================== NETWORK ATTACHMENT ====================
  NS_LOG_INFO("Attaching UEs to base stations");
  lteHelper->Attach(ueDevs, enbDevs.Get(0));

  // ==================== BEARER ACTIVATION ====================
  // NS_LOG_INFO("Activating EPS bearers");
  // EpsBearer::Qci q = EpsBearer::NGBR_LOW_LAT_EMBB;  // Use NB-IoT specific QoS
  // EpsBearer bearer(q);
  // lteHelper->ActivateDataRadioBearer(ueDevs, bearer);

  // ==================== TRAFFIC SETUP ====================
  NS_LOG_INFO("Setting up applications");

  // UL Traffic from UE to remote host
  uint16_t ulPort = 5000;
  double interPacketInterval = 60.0;

  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  // UDP Server on remote host
  PacketSinkHelper ulPacketSinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), ulPort));
  serverApps.Add(ulPacketSinkHelper.Install(remoteHost));

  // UDP Client on UE
  UdpClientHelper ulClient(remoteHost->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), ulPort);
  ulClient.SetAttribute("MaxPackets", UintegerValue(UINT32_MAX));
  ulClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  ulClient.SetAttribute("PacketSize", UintegerValue(200));
  clientApps.Add(ulClient.Install(ueNodes.Get(0)));

  // Start applications
  serverApps.Start(Seconds(0.5));
  clientApps.Start(Seconds(1.0));
  serverApps.Stop(simTime - Seconds(1));
  clientApps.Stop(simTime - Seconds(1));

  // Enable LTE traces
  lteHelper->EnableTraces();

  // ==================== SIMULATION CONTROL ====================
  Simulator::Stop(simTime);
  NS_LOG_INFO("Starting simulation...");
  Simulator::Run();
  
  NS_LOG_INFO("Simulation completed");
  Simulator::Destroy();

  return 0;
}
