#
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2021 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
import os

from utils import *
import pytest

# Get the directory of the current script
script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(script_dir)

subtec_log_path = script_dir + "/../" + "/subtec_console_common.log"

@pytest.mark.ttml
@pytest.mark.attributes
def test_1076():
    print("test_1076".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "sizeToPixels size 7 (from 7)", 20)
    copy_log_file(subtec_log_path, subtec_copy_path)

    sequence = [
        'onPacketReceived packet=type:TTML_SELECTION',
        'TtmlController created',
        'unmute received',
        'onPacketReceived packet=type:TTML_TIMESTAMP',        
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:fontSize=1c',
        'parseAttributes adding tts:textOutline=gray 0.25c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:fontSize=1c',
        'parseAttributes adding tts:textOutline=gray 0.25c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        ', outline: GRAY, 0.25c',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test text outline] [ ] [ ] [gray 0.25c] [ ] ',
        'sizeToPixels size 18 (from 25)']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1076 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:fontSize=1c',
        'parseAttributes adding tts:textOutline=red 5%',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:fontSize=1c',
        'parseAttributes adding tts:textOutline=red 5%',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        ', outline: RED, 5%',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test text outline] [ ] [ ] [red 5%]',
        'sizeToPixels size 4 (from 500)']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1076 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:fontSize=1c',
        'parseAttributes adding tts:textOutline=maroon 7px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:fontSize=1c',
        'parseAttributes adding tts:textOutline=maroon 7px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        ', outline: MAROON, 7px',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test text outline] [ ] [ ] [maroon 7px]',
        'sizeToPixels size 7 (from 7)']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1076 missing"

    print("test_1076".ljust(50), "Test Passed")
