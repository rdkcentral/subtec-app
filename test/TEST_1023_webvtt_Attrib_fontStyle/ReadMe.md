# Subtec Webvtt font style L2 test

This python3 L2 test verifies WebVTT support for various font style by sending
cc attributes packets with attribute values to cover all supported font styles. 

Currently validation of whether the font style is rendered as expected is 
to be done manually by checking that font style is updated during test.

## Pre-requisites to L2 tests:

Subtec dependencies installed with *install_subtec_dependencies.sh* script
and subtec running.

Required fonts must be available in /usr/share/fonts folder. 
Currently fonts need to be copied manually to this path for this test to pass. 
Fonts can be found in same path in devices or we can use below command :

	git clone ssh://gerrit.teamccp.com:29418/rdk/components/cpc/fonts/generic

test_fontStyle*.vtt - sample input vtt files associated with corresponding
font style to be placed in input_files folder.

## Run test using script:

From the *subtec-app/test/* folder run:

./run_l2_tests.py -i 1023