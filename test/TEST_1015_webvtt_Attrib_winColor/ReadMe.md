# Subtec Webvtt window opacity L2 test

This python3 L2 test verifies WebVTT support for various window opacity options
by sending cc attributes packets with attribute values to cover all 
supported window opacity options.

Currently validation of whether the window opacity is rendered as expected is 
to be done manually by checking that window opacity is updated during test.

## Pre-requisites to L2 tests:

Subtec dependencies installed with *install_subtec_dependencies.sh* script
and subtec running.

test_winOpacity*.vtt - sample input vtt files associated with corresponding
window opacity to be placed in input_files folder.

## Run test using script:

From the *subtec-app/test/* folder run:

./run_l2_tests.py -i 1015