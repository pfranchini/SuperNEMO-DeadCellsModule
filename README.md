# SuperNEMO-DeadCellsModule

Paolo Franchini 2020 - p.franchini@imperial.ac.uk

SuperNEMO module for removing hits in order to study dead cells in the Tracker.

The module removes tracker hits from the simulated data correspondent to a list of dead cells in the tracker.
The list can be randomly generated or read from a text file.

Input: simulated data with CD bank. \
Output: filtered simulated data with CD bank, to be used in a full reconstruction.

## Build:
```
git clone https://github.com/pfranchini/SuperNEMO-DeadCellsModule
mkdir SuperNEMO-DeadCellsModule.build
cd SuperNEMO-DeadCellsModule.build
cmake -DCMAKE_PREFIX_PATH=<path_Falaise_build> -DGSL_ROOT_DIR=<path_GSL_build> ../SuperNEMO-DeadCellsModule
make
make test
```

## Configure the module:
There is the way to configure the module, for the name of the output file and the option to use a list of dead cells or a randomized generation, using `DeadCellsModule.conf` produced in the build directory:
```
# - Module configuration:                                                                                                                                                  
[name="processing" type="DeadCellsModule"]
filename_out : string[1] = "test-output.brio"
random : boolean[1] = true                        # 'false' if reading a file with a list of dead cells
N_dead_cells : integer[1] = 0                     # number of dead cells to be randomly generated
dead_cells : string[1] = "list_of_cells.txt"      # file with a list of dead cells (side, layer, column)
```

## Run the module:
```
flreconstruct -i <brio_file_with_calibration_bank.brio> -p DeadCellsModule.conf
```

## Run the remaining reconstruction:
```
flreconstruct -i <test-output.brio> -p official-2.0.0_from_CAT.conf -o <test-output-reco.brio>
```
