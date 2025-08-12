# Subtec ttml text outline L2 test

This python3 L2 test verifies ttml support for various text outline options 
by sending cc attributes packets with attribute values to 
cover all a combination of text outline options. 

Currently validation of whether the text outline is rendered as expected is 
to be done manually by checking that text outline is updated during test.

## Pre-requisites to L2 tests:

Subtec dependencies installed with *install_subtec_dependencies.sh* script
and subtec running.

test_sample_textOutline*.ttml - sample input ttml files associated with 
corresponding text outline to be placed in input_files folder.

## Run test using script:

From the *subtec-app/test/* folder run:

./run_l2_tests.py -i 1076