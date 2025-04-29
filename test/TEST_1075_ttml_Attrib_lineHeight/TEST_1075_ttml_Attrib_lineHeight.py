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
def test_1075():
    print("test_1075".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "drawLine font size: 54 line text length: 350 line height: 81", 20)
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
        'parseAttributes adding tts:lineHeight=0.25c',
        'parseAttributes adding tts:fontSize=0.25c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=0.25c',
        'parseAttributes adding tts:fontSize=0.25c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test line height] [ ] [ ] [0.25c] [ ]',
        'sizeToPixels size 18 (from 0.25c)',
        'drawLine font size: 18 line text length: 114 line height: 18']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1075 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=0.5c',
        'parseAttributes adding tts:fontSize=0.25c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=0.5c',
        'parseAttributes adding tts:fontSize=0.25c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test line height] [ ] [ ] [0.5c] [ ] ',
        'sizeToPixels size 18 (from 0.25c)',
        'drawLine font size: 18 line text length: 114 line height: 36']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1075 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=0.75c',
        'parseAttributes adding tts:fontSize=0.5c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=0.75c',
        'parseAttributes adding tts:fontSize=0.5c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test line height] [ ] [ ] [0.75c] [ ] ',
        'sizeToPixels size 36 (from 0.5c)',
        'drawLine font size: 36 line text length: 236 line height: 54']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1075 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=1.2c',
        'parseAttributes adding tts:fontSize=0.5c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=1.2c',
        'parseAttributes adding tts:fontSize=0.5c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test line height] [ ] [ ] [1.2c] [ ] ',
        'sizeToPixels size 36 (from 0.5c)',
        'drawLine font size: 36 line text length: 236 line height: 86']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1075 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=20px',
        'parseAttributes adding tts:fontSize=20px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=20px',
        'parseAttributes adding tts:fontSize=20px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test line height] [ ] [ ] [20px] [ ] ',
        'sizeToPixels size 20 (from 20px)',
        'drawLine font size: 20 line text length: 130 line height: 20']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1075 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=36px',
        'parseAttributes adding tts:fontSize=20px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=36px',
        'parseAttributes adding tts:fontSize=20px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test line height] [ ] [ ] [36px] [ ] ',
        'sizeToPixels size 20 (from 20px)',
        'drawLine font size: 20 line text length: 130 line height: 36']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1075 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=54px',
        'parseAttributes adding tts:fontSize=20px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=54px',
        'parseAttributes adding tts:fontSize=20px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test line height] [ ] [ ] [54px] [ ] ',
        'sizeToPixels size 20 (from 20px)',
        'drawLine font size: 20 line text length: 130 line height: 54']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1075 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=75px',
        'parseAttributes adding tts:fontSize=20px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=75px',
        'parseAttributes adding tts:fontSize=20px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test line height] [ ] [ ] [75px] [ ] ',
        'sizeToPixels size 20 (from 20px)',
        'drawLine font size: 20 line text length: 130 line height: 75']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1075 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=25%',
        'parseAttributes adding tts:fontSize=54px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=25%',
        'parseAttributes adding tts:fontSize=54px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test line height] [ ] [ ] [25%] [ ] ',
        'sizeToPixels size 54 (from 54px)',
        'drawLine font size: 54 line text length: 350 line height: 14']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1075 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=50%',
        'parseAttributes adding tts:fontSize=54px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=50%',
        'parseAttributes adding tts:fontSize=54px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test line height] [ ] [ ] [50%] [ ] ',
        'sizeToPixels size 54 (from 54px)',
        'drawLine font size: 54 line text length: 350 line height: 27']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1075 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=75%',
        'parseAttributes adding tts:fontSize=54px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=75%',
        'parseAttributes adding tts:fontSize=54px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test line height] [ ] [ ] [75%] [ ] ',
        'sizeToPixels size 54 (from 54px)',
        'drawLine font size: 54 line text length: 350 line height: 41']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1075 missing"

    sequence = [      
        'parseAttributes adding xml:id=s0', 
        'parseAttributes adding tts:backgroundColor=#000000FF', 
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=150%',
        'parseAttributes adding tts:fontSize=54px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=white',
        'parseAttributes adding tts:lineHeight=150%',
        'parseAttributes adding tts:fontSize=54px',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'displaying: [00:00:00.000-00:00:05.000]: [ ] [Test line height] [ ] [ ] [150%] [ ] ',
        'sizeToPixels size 54 (from 54px)',
        'drawLine font size: 54 line text length: 350 line height: 81']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1075 missing"

    print("test_1075".ljust(50), "Test Passed")