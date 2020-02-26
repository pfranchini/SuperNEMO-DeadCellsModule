# SuperNEMO-DeadCellsModule

Paolo Franchini - p.franchini@imperial.ac.uk

SuperNEMO module for removing hits for Dead Cells in the Tracker

## Build:
```
git clone https://github.com/pfranchini/SuperNEMO-DeadCellsModule
mkdir SuperNEMO-DeadCellsModule.build
cd SuperNEMO-DeadCellsModule.build
cmake -DCMAKE_PREFIX_PATH=<path_Falaise_build> -DGSL_ROOT_DIR=<path_GSL_build> ../SuperNEMO-DeadCellsModule
make
```

## Run the module:
```
flreconstruct -i <brio_file_with_calibration_bank.brio> -p testmodule.conf.in
```

## Run the remaining reconstruction:
```
flreconstruct -i <test-output.brio> -p official-2.0.0.conf -o <test-output-reco.brio>
```
