//! \file    DeadCellsModule.h
//! \brief   Processing module for flreconstruct
//! \details Process a things object
#ifndef DEADCELLSMODULE_HH
#define DEADCELLSMODULE_HH

// Standard Library
#include <vector>
#include <string>
#include <iostream>

// Third Party
#include "TFile.h"
//#include "TTree.h"

// - Boost
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"

// - Bayeux
#include "bayeux/dpp/base_module.h"
#include "bayeux/dpp/output_module.h"
#include "bayeux/mctools/simulated_data.h"
#include "bayeux/mctools/utils.h"
#include "bayeux/datatools/service_manager.h"
#include "bayeux/geomtools/manager.h"
#include "bayeux/geomtools/geometry_service.h"
#include "bayeux/version.h"

// - Falaise
#include "falaise/falaise.h"
#include "falaise/exitcodes.h"
#include "falaise/resource.h"
#include "falaise/version.h"
#include "falaise/snemo/datamodels/calibrated_data.h"
#include "falaise/snemo/datamodels/data_model.h"
#include "falaise/snemo/services/services.h"

// This Project
#include "lib/FLSimulateArgs.h"

class DeadCellsModule : public dpp::base_module {

 public:
  //! Construct module
  DeadCellsModule();
  //! Destructor
  virtual ~DeadCellsModule();
  //! Configure the module
  virtual void initialize(const datatools::properties& myConfig,
                          datatools::service_manager& flServices,
			  dpp::module_handle_dict_type& moduleDict);
  //! Process supplied data record
  virtual dpp::base_module::process_status process(datatools::things& workItem);
  //! Reset the module
  virtual void reset();

private: 
  
  // configurable data member
  //std::string filename_output_;

  // Macro which automatically creates the interface needed
  // to enable the module to be loaded at runtime
  DPP_MODULE_REGISTRATION_INTERFACE(DeadCellsModule);
};

#endif // SIMVALMODULE_HH


