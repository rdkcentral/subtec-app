# Subtec 608 closed captions attributes L2 test for border color

This python3 L2 test verifies 608 type closed captions attributes 
functionality for border color.

Currently Closed Captions hal L3 tests have been used to capture 608cc
data hexadecimal format in the decode callback for test input. This data
is then sent in 1 second intervals.

Since the file is static, we send attribute packets in some combinations,
intermittently and validate the logs. 

Currently validation of whether the attribute change is rendered as expected 
is to be done manually by monitoring cc on screen during test.

## Pre-requisites to L2 tests:

Subtec dependencies installed with *install_subtec_dependencies.sh* script
and subtec running.

capture608.hex - sample hexadecimal data of 608 closed captions currently
expected in the format of "dataLength localPts data" to be placed 
in input_files folder.

## Run test using script:

From the *subtec-app/test/* folder run:

./run_l2_tests.py -i 1037
