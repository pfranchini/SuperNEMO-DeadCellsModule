/*=================================================================
/
/ SuperNemo - Dead Cells Module
/
/ Paolo Franchini 2020 - p.franchini@imperial.ac.uk
/ https://github.com/pfranchini/SuperNEMO-DeadCellsModule
/
===================================================================*/

#include "DeadCellsModule.h"

// output file
dpp::output_module simOutput;

// event output
datatools::things workItem;
dpp::base_module::process_status status;
snemo::datamodel::calibrated_data *new_calibrated_data;

// metadata
FLSimulate::FLSimulateArgs flSimParameters;

// various variables
bool randomize = true;
std::string filename_output = "test-output.brio";
int N_dead_cells;
std::string dead_cells_list;
int dead_cells[2500][3];  // Define a matrix (side, layer, column) of dead cells 
Long64_t hits = 0;
Long64_t killed = 0;


DPP_MODULE_REGISTRATION_IMPLEMENT(DeadCellsModule,"DeadCellsModule");

//=================================================================

DeadCellsModule::DeadCellsModule() : dpp::base_module() {}


DeadCellsModule::~DeadCellsModule() {
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
  if (!flSimParameters.experimentalSetupUrn.empty())
    system_props.store_string("experimentalSetupUrn",
                              flSimParameters.experimentalSetupUrn,
                              "Experimental setup URN");
  system_props.store_boolean("embeddedMetadata",
                             flSimParameters.embeddedMetadata,
                             "Metadata embedding flag");
  boost::posix_time::ptime start_run_timestamp = boost::posix_time::second_clock::universal_time();
  system_props.store_string( "timestamp", boost::posix_time::to_iso_string(start_run_timestamp), "Run start timestamp");
  if (flSimParameters.doSimulation) {
    // Simulation section:
    datatools::properties& simulation_props = flSimMetadata.add_section("flsimulate.simulation", "flsimulate::section");
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


void dead_cells_service(int dc[2500][3]) {

  try {
    std::ifstream infile(dead_cells_list);   // File for list of dead cells
    Long64_t i=0;
    int side, layer, column;
    while (infile >> side >> layer >> column){
      dead_cells[i][0]=side;
      dead_cells[i][1]=layer;
      dead_cells[i][2]=column;
      i++;
    }
    N_dead_cells=i;
    std::cout << "Read " << N_dead_cells << " dead cells" << std::endl;
  }
  catch (std::logic_error& e) {
    std::cerr << "Problem in the list of dead cells " << e.what() << std::endl;
  };
  

};


void DeadCellsModule::initialize(const datatools::properties& myConfig,
			    datatools::service_manager& flServices,
			    dpp::module_handle_dict_type& /*moduleDict*/){

  // Extract the filename_out key from the supplied config, if the key exists.
  // datatools::properties throws an exception if the key isn't in the config, so catch this if thrown
  try {
    myConfig.fetch("filename_out", filename_output);
  }
  catch (std::logic_error& e) {
    std::cerr << "Problem in the output file " << e.what() << std::endl;
  };

  // Extract option to create random dead cells or to read them (from a file)   
  try {
    myConfig.fetch("random", randomize);
    if (randomize)
      try {
	myConfig.fetch("N_dead_cells", N_dead_cells);
      }
      catch (std::logic_error& e) {
	std::cerr << "Error in configuration files: number of dead cells not present " << e.what() << std::endl;
      }
    else
      try {
        myConfig.fetch("dead_cells", dead_cells_list);
      } 
      catch (std::logic_error& e) {
	std::cerr << "Error in configuration files: file with list of dead cells not present " << e.what() << std::endl;
      } 
  } catch (std::logic_error& e) {}

  
  std::cout << "Dead cells module initialized..." << std::endl;
  std::cout << "Output file: " << filename_output << std::endl;
  
 
  flSimParameters = FLSimulate::FLSimulateArgs::makeDefault();

  datatools::service_manager services("DeadCells", "SuperNEMO Dead Cells");
  datatools::properties services_config;
  services_config.store("name", "DeadCells");
  services_config.store("description","SuperNEMO Demonstrator Dead Cells module");
  std::vector<std::string> scf = {"@falaise:snemo/demonstrator/geometry/" "GeometryService.conf"};
  services_config.store("services.configuration_files", scf);
  services.initialize(services_config);

  // Output metadata management:
  datatools::multi_properties flSimMetadata("name", "type", "Metadata associated to a flsimulate run");
  do_metadata(flSimParameters, flSimMetadata);  

  // Metadata management:
  simOutput.set_name("DeadCellsOutput");
  simOutput.set_single_output_file(filename_output);
  datatools::multi_properties& metadataStore =  simOutput.grab_metadata_store();
  metadataStore = flSimMetadata;
  simOutput.initialize_simple();   

  // set up CD data bank
  new_calibrated_data = &(workItem.add<snemo::datamodel::calibrated_data>("CD"));

  // Define the dead cells:

  if (randomize) {
    // Random dead cells
    bool isNew = false;
    std::cout << "Generating " << N_dead_cells << " random dead cells..." << std::endl;
    for (Long64_t i=0; i<N_dead_cells; i++){
      // need to check if the dead_cell is not already in the list
      isNew = false;
      while (!isNew) {  // try a random cell
	dead_cells[i][0]=rand() % 2;    // side
	dead_cells[i][1]=rand() % 9;    // layer
	dead_cells[i][2]=rand() % 113;  // column
	isNew = true; // assume is new
	for (Long64_t j=0; j<i; j++){  // look if is alredy in the  matrix
	  if ( (dead_cells[i][0]==dead_cells[j][0]) && (dead_cells[i][1]==dead_cells[j][1]) && (dead_cells[i][2]==dead_cells[j][2]) ) {
	    isNew = false; // is already in the matrix so will try again
	    break;
	  }
	}
      }
    }
    // Save list of dead cells in a text file to be used in another module
    std::ofstream deadcells_file;
    std::string deadcells_filename = "random_dead_cells_"+std::to_string(N_dead_cells)+".txt";
    deadcells_file.open(deadcells_filename);
    for (Long64_t i=0; i<N_dead_cells; i++)
      deadcells_file << dead_cells[i][0] << " " << dead_cells[i][1] << " " << dead_cells[i][2] << "\n";      
    deadcells_file.close();
  }
  else {
    // Dead cells from service
    std::cout << "Reading dead cells from file..." << std::endl;
    dead_cells_service(dead_cells);
  }

  // print list of dead cells   
  /*for (Long64_t i=0; i<N_dead_cells; i++)                                                                                                                             
    std::cout << dead_cells[i][0] << "-" << dead_cells[i][1] << "-" << dead_cells[i][2] << std::endl;                                                                     
  */

  this->_set_initialized(true);
  
}

//! [DeadCellsModule::Process]
dpp::base_module::process_status DeadCellsModule::process(datatools::things& event) {
  
  bool kill=false;
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

      hits++;  // counts the number of hits for all the events

      kill=false;
      // Find if this hit of this event is on any dead cell or not   
      for (Long64_t k=0; k<N_dead_cells; k++){
	if ( (trackerHitHdl->get_side()==dead_cells[k][0])&&(trackerHitHdl->get_layer()==dead_cells[k][1])&&(trackerHitHdl->get_row()==dead_cells[k][2]) ){  // is dead
	  kill=true;
	  killed++;
	  /*i = &trackerHitHdl - &calData.calibrated_tracker_hits()[0];
	  std::cout << "hit to be removed: " << i << std::endl;
	  std::cout << "Tracker Hit: " << trackerHitHdl->get_x() << "," << trackerHitHdl->get_y() << "," << trackerHitHdl->get_z() << std::endl;
	  std::cout << "             " << trackerHitHdl->get_side() << "," << trackerHitHdl->get_layer() << "," << trackerHitHdl->get_row() << std::endl;
	  */
          break; // ends loop since the hit is already on one dead cell      
	}
      } // end loop in the killing procedure  
      
      if (!kill){ // keeps the Tracker data
	new_hit_handle = trackerHitHdl;
	new_calibrated_data->calibrated_tracker_hits().push_back(new_hit_handle);
      }
    } // end loop in the tracker hits

    // Print output                                                                                                                                                    
    /*const auto& calData2 = workItem.get<snemo::datamodel::calibrated_data>("CD");
    for (const auto& hit : calData2.calibrated_tracker_hits()) {
      std::cout << "Saved Hit: " << hit->get_x() << "," << hit->get_y() << "," << hit->get_z() << std::endl;
      std::cout << "           " << hit->get_side() << "," << hit->get_layer() << "," << hit->get_row() << std::endl;
      }
    */
    
    // Calorimeter data kept in any case
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


//! [DeadCellsModule::reset]
void DeadCellsModule::reset() {

  // Print stats
  std::cout << "Total number of hits in the tracker: " << hits << std::endl;
  std::cout << "Killed hits: " << killed << std::endl;

  std::cout << "END" << std::endl;

  this->_set_initialized(false);

}

