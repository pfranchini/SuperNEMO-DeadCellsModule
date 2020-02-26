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
## Configure the module:

There is the option to configure the output filename in the module configuration file.The final two lines of the configuration file must have:
```
[name="processing" type="testmodule"]
filename_out : string[1] = "output-filename.brio"
```
If not the default output will be `test-output.brio`.

## Run the module:
```
flreconstruct -i <brio_file_with_calibration_bank.brio> -p testmodule.conf.in
```

## Run the remaining reconstruction:
```
flreconstruct -i <test-output.brio> -p official-2.0.0_from_CAT.conf -o <test-output-reco.brio>
```
