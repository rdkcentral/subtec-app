# Subtec Webvtt edge type L2 test

This python3 L2 test verifies WebVTT support for various edge type options
by sending cc attributes packets with attribute values to cover all 
supported edge type options.

Currently validation of whether the edge type is rendered as expected is 
to be done manually by checking that edge type is updated during test.

## Pre-requisites to L2 tests:

Subtec dependencies installed with *install_subtec_dependencies.sh* script
and subtec running.

test_edgeType*.vtt - sample input vtt files associated with corresponding
edge type to be placed in input_files folder.

## Run test using script:

From the *subtec-app/test/* folder run:

./run_l2_tests.py -i 1017