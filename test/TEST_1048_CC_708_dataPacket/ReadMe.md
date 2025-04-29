# Subtec 708 closed captions L2 test

This python3 L2 test verifies 708 type closed captions functionality.

Currently Closed Captions hal L3 tests have been used to capture 708cc
data hexadecimal format in the decode callback for test input. This data
is then sent in fixed intervals.

## Pre-requisites to L2 tests:

Subtec dependencies installed with *install_subtec_dependencies.sh* script
and subtec running.

capture708.hex - sample hexadecimal data of 708 closed captions currently
expected in the format of "dataLength localPts data" to be placed 
in input_files folder.

## Run test using script:

From the *subtec-app/test/* folder run:

./run_l2_tests.py -i 1048