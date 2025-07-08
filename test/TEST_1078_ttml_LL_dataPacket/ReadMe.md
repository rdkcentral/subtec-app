# Subtec ttml data L2 test

This python3 L2 test verifies ttml data, specifically when ttml duration is
less like ~500ms. This test has been added as there was a subtitle
flickering issue observed in this scenario at LLAMA-11626.

## Pre-requisites to L2 tests:

Subtec dependencies installed with *install_subtec_dependencies.sh* script
and subtec running.

sample.ttml - sample input ttml file to be placed in input_files folder.

## Run test using script:

From the *subtec-app/test/* folder run:

./run_l2_tests.py -i 1078
