#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>
#include "ns3/energy-module.h"
#include "ns3/device-energy-model.h"
//#include "ns3/gtk-config-store.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("NBIoT");

// Function to log remaining energy
void EnergyConsumptionCallback(double oldEnergy, double newEnergy) {
  std::cout << "Energy changed from " << oldEnergy
      << " J to " << newEnergy
      << " J at " << Simulator::Now().GetSeconds() << "s";
}

int main (int argc, char *argv[])
{
  Time simTime = MilliSeconds (1050);
  bool useCa = true;
  // bool useCa = false;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("simTime", "Total duration of the simulation", simTime);
  cmd.AddValue ("useCa", "Whether to use carrier aggregation.", useCa);
  cmd.Parse (argc, argv);

  // to save a template default attribute file run it like this:
  // ./waf --command-template="%s --ns3::ConfigStore::Filename=input-defaults.txt --ns3::ConfigStore::Mode=Save --ns3::ConfigStore::FileFormat=RawText" --run src/lte/examples/lena-simple
  //
  // to load a previously created default attribute file
  // ./waf --command-template="%s --ns3::ConfigStore::Filename=input-defaults.txt --ns3::ConfigStore::Mode=Load --ns3::ConfigStore::FileFormat=RawText" --run src/lte/examples/lena-simple

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // Parse again so you can override default values from the command line
  cmd.Parse (argc, argv);

  if (useCa)
   {
     Config::SetDefault ("ns3::LteHelper::UseCa", BooleanValue (useCa));
     Config::SetDefault ("ns3::LteHelper::NumberOfComponentCarriers", UintegerValue (2));
     Config::SetDefault ("ns3::LteHelper::EnbComponentCarrierManager", StringValue ("ns3::RrComponentCarrierManager"));
   }

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  // Uncomment to enable logging
  lteHelper->SetLogDir ("/logs");
  lteHelper->EnableLogComponents ();

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (1);

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  // Default scheduler is PF, uncomment to use RR
  //lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");

  // Energy source configuration
  BasicEnergySourceHelper basicEnergySourceHelper;
  basicEnergySourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(10000.0)); // Initial energy

  // Install energy sources on all nodes
  EnergySourceContainer energySources;
  energySources.Add(basicEnergySourceHelper.Install(enbNodes));
  energySources.Add(basicEnergySourceHelper.Install(ueNodes));

  // Optional: Similar configuration for eNB energy models if available

  // Install Mobility Model
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ueNodes);
  BuildingsHelper::Install (ueNodes);

  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs = lteHelper->InstallUeDevice (ueNodes);

  // Attach a UE to a eNB
  lteHelper->Attach (ueDevs, enbDevs.Get (0));

  // Activate a data radio bearer
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs, bearer);
  lteHelper->EnableTraces ();

  Simulator::Stop (simTime);

  // Connect traces to energy sources
  for (EnergySourceContainer::Iterator it = energySources.Begin(); it != energySources.End(); ++it) {
      Ptr<BasicEnergySource> source = DynamicCast<BasicEnergySource>(*it);
      source->TraceConnectWithoutContext("RemainingEnergy", MakeCallback(&EnergyConsumptionCallback));
  }

  std::cout << "Test";
  Simulator::Run ();

  // GtkConfigStore config;
  // config.ConfigureAttributes ();
  //
  double totalEnergyConsumed = 0.0;
  for (EnergySourceContainer::Iterator it = energySources.Begin(); it != energySources.End(); ++it) {
      Ptr<BasicEnergySource> source = DynamicCast<BasicEnergySource>(*it);
      totalEnergyConsumed += (10000.0 - source->GetRemainingEnergy()); // Initial energy was 10,000 J
  }
  std::cout << "Total energy consumed: " << totalEnergyConsumed << " J";

  Simulator::Destroy ();
  return 0;
}
