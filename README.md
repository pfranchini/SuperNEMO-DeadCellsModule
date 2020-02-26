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

## Run:
```
flreconstruct -i <brio_file_with_calibration_bank.brio> -p testmodule.conf.in
```
