# Subtec Webvtt font size L2 test

This python3 L2 test verifies WebVTT support for various font size by sending
cc attributes packets with attribute values to cover all supported font size. 

## Pre-requisites to L2 tests:

Subtec dependencies installed with *install_subtec_dependencies.sh* script
and subtec running.

test_fontSize*.vtt - sample input vtt files associated with corresponding
font size to be placed in input_files folder.

## Run test using script:

From the *subtec-app/test/* folder run:

./run_l2_tests.py -i 1019