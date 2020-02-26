#include "testmodule.h"

// Variables here:

//  output file
dpp::output_module simOutput;

//  event output
datatools::things workItem;
dpp::base_module::process_status status;
//snemo::datamodel::calibrated_data* ptr_calib_data = 0;
snemo::datamodel::calibrated_data *new_calibrated_data;

FLSimulate::FLSimulateArgs flSimParameters;

DPP_MODULE_REGISTRATION_IMPLEMENT(testmodule,"testmodule");


testmodule::testmodule() : dpp::base_module()
{
  //filename_output_="SimValidation.root";
}


testmodule::~testmodule() {
  //if (is_initialized())
  this->reset();
}


falaise::exit_code do_metadata( const FLSimulate::FLSimulateArgs& flSimParameters, datatools::multi_properties& flSimMetadata) {

  falaise::exit_code code = falaise::EXIT_OK;

  // System section:
  datatools::properties& system_props = flSimMetadata.add_section("flsimulate", "flsimulate::section");
  system_props.set_description("flsimulate basic system informations");

  system_props.store_string("bayeux.version", bayeux::version::get_version(),
                            "Bayeux version");

  system_props.store_string("falaise.version", falaise::version::get_version(),
                            "Falaise version");

  system_props.store_string(
			    "application", "flsimulate",
			    "The simulation application used to produce Monte Carlo data");

  system_props.store_string("application.version",
                            falaise::version::get_version(),
                            "The version of the simulation application");

  system_props.store_string("userProfile", flSimParameters.userProfile,
                            "User profile");

  system_props.store_integer("numberOfEvents", flSimParameters.numberOfEvents,
                             "Number of simulated events");

  system_props.store_boolean("doSimulation", flSimParameters.doSimulation,
                             "Activate simulation");

  system_props.store_boolean("doDigitization", flSimParameters.doDigitization,
                             "Activate digitization");

  if (!flSimParameters.experimentalSetupUrn.empty()) {
    system_props.store_string("experimentalSetupUrn",
                              flSimParameters.experimentalSetupUrn,
                              "Experimental setup URN");
  }

  system_props.store_boolean("embeddedMetadata",
                             flSimParameters.embeddedMetadata,
                             "Metadata embedding flag");

  boost::posix_time::ptime start_run_timestamp =
    boost::posix_time::second_clock::universal_time();
  system_props.store_string(
			    "timestamp", boost::posix_time::to_iso_string(start_run_timestamp),
			    "Run start timestamp");

  if (flSimParameters.doSimulation) {
    // Simulation section:
    datatools::properties& simulation_props = flSimMetadata.add_section(
									"flsimulate.simulation", "flsimulate::section");
    simulation_props.set_description("Simulation setup parameters");

    if (!flSimParameters.simulationSetupUrn.empty()) {
      simulation_props.store_string("simulationSetupUrn",
                                    flSimParameters.simulationSetupUrn,
                                    "Simulation setup URN");
    } else if (!flSimParameters.simulationManagerParams.manager_config_filename
	       .empty()) {
      simulation_props.store_path(
				  "simulationManagerConfig",
				  flSimParameters.simulationManagerParams.manager_config_filename,
				  "Simulation manager configuration file");
    }
  }

  if (flSimParameters.doDigitization) {
    // Digitization section:
    datatools::properties& digitization_props = flSimMetadata.add_section(
									  "flsimulate.digitization", "flsimulate::section");
    digitization_props.set_description("Digitization setup parameters");

    // Not implemented yet.
  }

  // Variants section:
  datatools::properties& variants_props = flSimMetadata.add_section(
								    "flsimulate.variantService", "flsimulate::section");
  variants_props.set_description("Variant setup");

  if (!flSimParameters.variantConfigUrn.empty()) {
    variants_props.store_string("configUrn", flSimParameters.variantConfigUrn,
                                "Variants setup configuration URN");
  } else if (!flSimParameters.variantSubsystemParams.config_filename.empty()) {
    variants_props.store_path(
			      "config", flSimParameters.variantSubsystemParams.config_filename,
			      "Variants setup configuration path");
  }

  if (!flSimParameters.variantProfileUrn.empty()) {
    variants_props.store_string("profileUrn", flSimParameters.variantProfileUrn,
                                "Variants profile URN");
  } else if (!flSimParameters.variantSubsystemParams.profile_load.empty()) {
    variants_props.store_path(
			      "profile", flSimParameters.variantSubsystemParams.profile_load,
			      "Variants profile path");
  }

  if (flSimParameters.variantSubsystemParams.settings.size()) {
    variants_props.store("settings",
                         flSimParameters.variantSubsystemParams.settings,
                         "Variants settings");
  }

  // Services section:
  datatools::properties& services_props =
    flSimMetadata.add_section("flsimulate.services", "flsimulate::section");
  services_props.set_description("Services configuration");

  if (!flSimParameters.servicesSubsystemConfigUrn.empty()) {
    services_props.store_string("configUrn",
                                flSimParameters.servicesSubsystemConfigUrn,
                                "Services setup configuration URN");
  } else if (!flSimParameters.servicesSubsystemConfig.empty()) {
    services_props.store_path("config", flSimParameters.servicesSubsystemConfig,
                              "Services setup configuration path");
  }

  return code;
}


void testmodule::initialize(const datatools::properties& myConfig,
                                   datatools::service_manager& flServices,
                                   dpp::module_handle_dict_type& /*moduleDict*/){

  // Extract the filename_out key from the supplied config, if
  // the key exists. datatools::properties throws an exception if
  // the key isn't in the config, so catch this if thrown and don't do
  // anything
  //try {
  //  myConfig.fetch("filename_out",this->filename_output_);
  //} catch (std::logic_error& e) {
  
  std::cout << "============= Initialize" << std::endl;

  flSimParameters = FLSimulate::FLSimulateArgs::makeDefault();

  datatools::service_manager services("DeadCells",
				      "SuperNEMO Dead Cells");
  datatools::properties services_config;
  services_config.store("name", "DeadCells");
  services_config.store(
			"description",
			"SuperNEMO Demonstrator Dead Cells module");
  std::vector<std::string> scf = {
        "@falaise:snemo/demonstrator/geometry/"
        "GeometryService.conf"};
  services_config.store("services.configuration_files", scf);
  services.initialize(services_config);

  // Output metadata management:
  datatools::multi_properties flSimMetadata("name", "type", "Metadata associated to a flsimulate run");
  do_metadata(flSimParameters, flSimMetadata);  

  // Metadata management:
  simOutput.set_name("DeadCellsOutput");
  simOutput.set_single_output_file("test-output.brio");
  datatools::multi_properties& metadataStore =  simOutput.grab_metadata_store();
  metadataStore = flSimMetadata;
  simOutput.initialize_simple();   


  // set up CD data bank
  new_calibrated_data = &(workItem.add<snemo::datamodel::calibrated_data>("CD"));
  //new_calibrated_data = *ptr_calib_data;

  // Define the dead cells
  // ....


  this->_set_initialized(true);

}


//! [testmodule::Process]
dpp::base_module::process_status
testmodule::process(datatools::things& event) {
  
  int i = 0;

  // Grab Calibrated Data bank
  try {
  
    // To Read                    
    //const mctools::simulated_data& CD = event.get<mctools::simulated_data>("CD");
    const auto& calData = event.get<snemo::datamodel::calibrated_data>("CD");

    snemo::datamodel::calibrated_data::tracker_hit_handle_type new_hit_handle(new snemo::datamodel::calibrated_tracker_hit);
    snemo::datamodel::calibrated_tracker_hit& new_calibrated_tracker_hit = new_hit_handle.grab();

    // Loops over tracker hits for this event
    for (const auto& trackerHitHdl : calData.calibrated_tracker_hits()) {

      // Find if this hit of this event is on any dead cell or not   
      // .....

      if ((trackerHitHdl->get_side()!=1) || (trackerHitHdl->get_layer()!=4) || (trackerHitHdl->get_row()!=86)) {

	i = &trackerHitHdl - &calData.calibrated_tracker_hits()[0];

	std::cout << "hit to be removed: " << i << std::endl;
	std::cout << "Tracker Hit: " << trackerHitHdl->get_x() << "," << trackerHitHdl->get_y() << "," << trackerHitHdl->get_z() << std::endl;
	std::cout << "             " << trackerHitHdl->get_side() << "," << trackerHitHdl->get_layer() << "," << trackerHitHdl->get_row() << std::endl;

	new_hit_handle = trackerHitHdl;
	new_calibrated_data->calibrated_tracker_hits().push_back(new_hit_handle);

      }

    } // for

    // Print output                                                                                                                                                    
    const auto& calData2 = workItem.get<snemo::datamodel::calibrated_data>("CD");
    for (const auto& hit : calData2.calibrated_tracker_hits()) {
      std::cout << "Saved Hit: " << hit->get_x() << "," << hit->get_y() << "," << hit->get_z() << std::endl;
      std::cout << "           " << hit->get_side() << "," << hit->get_layer() << "," << hit->get_row() << std::endl;
    }

    // Calorimeter data
    new_calibrated_data->calibrated_calorimeter_hits() = calData.calibrated_calorimeter_hits();

    // write workItem in the new brio file
    dpp::base_module::process_status status = simOutput.process(workItem);    
    if (status != dpp::base_module::PROCESS_OK)
      std::cerr << "Output module failed" << std::endl;

    // clear for next event
    new_calibrated_data->reset();


  }
  
  catch (std::logic_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return dpp::base_module::PROCESS_INVALID;
  }// end try on CD bank
  
  // returns a status, see ref dpp::processing_status_flags_type
  return dpp::base_module::PROCESS_OK;
}


//! [testmodule::reset]
void testmodule::reset() {

  // Print stats
  //std::cout << "Total number of hits: " << hits << std::endl;
  //std::cout << "Killed pixels: " << killed << std::endl;

  std::cout << "END" << std::endl;

  this->_set_initialized(false);

}

