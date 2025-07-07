# Subtec ttml data L2 test

This python3 L2 test verifies ttml data, specifically testing default
xml whitespace handling when we have only newline character and whitespaces
in first line of text.

## Pre-requisites to L2 tests:

Subtec dependencies installed with *install_subtec_dependencies.sh* script
and subtec running.

sample.ttml - sample input ttml file to be placed in input_files folder.

## Run test using script:

From the *subtec-app/test/* folder run:

./run_l2_tests.py -i 1077
