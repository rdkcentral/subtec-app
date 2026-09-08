# Subtec Webvtt zero-width cue L2 test

This python3 L2 test verifies that WebVTT cues with `size:0%` do not stop subtitle processing.
It covers start-, center-, and end-aligned zero-width cues, including long overflow text, and then
sends a normal cue that should still be rendered afterward.

## Pre-requisites to L2 tests:

Subtec dependencies installed with *install_subtec_dependencies.sh* script
and subtec running.

`test_sample_zero_width.vtt` - sample input VTT file to be placed in `input_files` folder.

## Run test using script:

From the *subtec-app/test/* folder run:

./run_l2_tests.py -i 1081
