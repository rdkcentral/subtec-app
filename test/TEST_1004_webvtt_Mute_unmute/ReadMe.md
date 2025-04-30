# Subtec Webvtt mute-unmute packet sequence L2 test

This python3 L2 test verifies WebVTT unmute functionality when unmute packet is sent few seconds after a mute packet was sent. 

## Pre-requisites to L2 tests:

Subtec dependencies installed with *install_subtec_dependencies.sh* script
and subtec running.

test_sample.vtt - sample input vtt file to be placed in input_files folder.

## Run test using script:

From the *subtec-app/test/* folder run:

./run_l2_tests.py -i 1004