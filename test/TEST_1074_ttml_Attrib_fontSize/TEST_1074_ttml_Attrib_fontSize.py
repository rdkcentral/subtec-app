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
def test_1074():
    print("test_1074".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "displaying: [00:00:00.000-00:00:05.000]: [ ] [Test font size] [ ] [ ] [99%] [ ] ", 20)
    copy_log_file(subtec_log_path, subtec_copy_path)

    sequence = [
        'onPacketReceived packet=type:TTML_SELECTION',
        'TtmlController created',
        'unmute received',
        'onPacketReceived packet=type:TTML_TIMESTAMP',        
        'parseAttributes adding xml:id=s0',
        'parseAttributes adding tts:fontSize=0.25c',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:fontSize=0.25c',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test font size] [ ] [ ] [0.25c] [ ]',
        'sizeToPixels size 18 (from 25)',
        'drawLine font size: 18']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1074 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:fontSize=0.5c',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:fontSize=0.5c',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test font size] [ ] [ ] [0.5c] [ ] ',
        'sizeToPixels size 36 (from 50)',
        'drawLine font size: 36']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1074 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:fontSize=0.75c',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:fontSize=0.75c',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test font size] [ ] [ ] [0.75c] [ ] ',
        'sizeToPixels size 54 (from 75)',
        'drawLine font size: 54']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1074 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:fontSize=1.2c',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:fontSize=1.2c',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test font size] [ ] [ ] [1.2c] [ ] ',
        'sizeToPixels size 86 (from 120)',
        'drawLine font size: 86']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1074 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:fontSize=20px',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:fontSize=20px',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test font size] [ ] [ ] [20px] [ ] ',
        'sizeToPixels size 20 (from 20)',
        'drawLine font size: 20']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1074 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:fontSize=36px',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:fontSize=36px',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test font size] [ ] [ ] [36px] [ ] ',
        'sizeToPixels size 36 (from 36)',
        'drawLine font size: 36']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1074 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:fontSize=54px',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:fontSize=54px',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test font size] [ ] [ ] [54px] [ ] ',
        'sizeToPixels size 54 (from 54)',
        'drawLine font size: 54']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1074 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:fontSize=75px',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:fontSize=75px',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test font size] [ ] [ ] [75px] [ ] ',
        'sizeToPixels size 75 (from 75)',
        'drawLine font size: 75']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1074 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:fontSize=25%',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:fontSize=25%',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test font size] [ ] [ ] [25%] [ ] ',
        'sizeToPixels size 18 (from 2500)',
        'drawLine font size: 18']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1074 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:fontSize=50%',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:fontSize=50%',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test font size] [ ] [ ] [50%] [ ] ',
        'sizeToPixels size 36 (from 5000)',
        'drawLine font size: 36']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1074 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:fontSize=75%',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:fontSize=75%',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test font size] [ ] [ ] [75%] [ ] ',
        'sizeToPixels size 54 (from 7500)',
        'drawLine font size: 54']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1074 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:fontSize=99%',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:fontSize=99%',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test font size] [ ] [ ] [99%] [ ] ',
        'sizeToPixels size 71 (from 9900)',
        'drawLine font size: 71']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1074 missing"

    print("test_1074".ljust(50), "Test Passed")
