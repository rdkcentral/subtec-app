# Subtec Webvtt null-only data recovery L2 test

This python3 L2 test verifies WebVTT recovery after a data packet containing
only a single NUL byte is followed by a valid WebVTT data packet on the same
channel. It validates that the invalid payload is rejected as a bad WebVTT
header and that both cues from the following valid payload are displayed.

## Pre-requisites to L2 tests:

Subtec dependencies installed with *install_subtec_dependencies.sh* script
and subtec running.

test_sample.vtt - valid WebVTT input file to be placed in input_files folder.

## Run test using script:

From the *subtec-app/test/* folder run:

./run_l2_tests.py -i 1082