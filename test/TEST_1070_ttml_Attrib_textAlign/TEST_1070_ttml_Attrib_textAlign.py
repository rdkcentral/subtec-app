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
def test_1070():
    print("test_1070".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "displaying: [00:00:00.000-00:00:05.000]: [ ] [Test text align] [ ] [ ] [center] [ ] ", 20)
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
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:fontSize=1c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test text align] [ ] [ ] [left] [ ] ',
        'alignPenX textAlign=left',
        'drawChunk starting pen position: 23 0 (']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1070 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=right',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:fontSize=1c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=right',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:fontSize=1c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test text align] [ ] [ ] [right] [ ] ',
        'alignPenX textAlign=right',
        'drawChunk starting pen position: 1126 0 (']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1070 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=center',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:fontSize=1c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=center',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:fontSize=1c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test text align] [ ] [ ] [center] [ ] ',
        'alignPenX textAlign=center',
        'drawChunk starting pen position: 574 0 (']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1070 missing"

    print("test_1070".ljust(50), "Test Passed")
