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

@pytest.mark.cc_708
@pytest.mark.attributes
def test_1051():
    print("test_1051".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "[ClosedCaptions::TextGfxDrawer] draw text:[Philanthropy.]")
    copy_log_file(subtec_log_path, subtec_copy_path)

    sequence = [
        'onPacketReceived packet=type:SUBTITLE_SELECTION',
        'Selecting CC Subtitles',
        'CcSubController created',
        '[ClosedCaptions::CcController] setActiveService: type 1, service 1',
        'onPacketReceived packet=type:UNMUTE',]
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1051 missing"

    sequence = [
        'new value for attibId[0x20] = 3',
        'new value for attibId[0x1] = 16711680',
        'new value for attibId[0x2] = 255',
        'new value for attibId[0x2000] = 0',
        'new value for attibId[0x1000] = 4',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Bound together]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[by a resilient spirit,]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[in wild Scandinavia.]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=22"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[[Wind blowing]]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1051 missing"

    sequence = [
        'new value for attibId[0x2000] = 16777215',
        'new value for attibId[0x1000] = 5',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=120"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:["Wild Scandinavia"]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1051 missing"

    sequence = [
        'new value for attibId[0x2000] = 65280',
        'new value for attibId[0x1000] = 2',
        ]
    marker_string1 = "[ClosedCaptions::TextGfxDrawer] draw text:[was made possible in part by]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1), "Logs for test_1051 missing"

    sequence = [
        'new value for attibId[0x2000] = 16711935',
        'new value for attibId[0x1000] = 3',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=160"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[was made possible in part by]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1051 missing"

    sequence = [
        'new value for attibId[0x2000] = 16776960',
        'new value for attibId[0x1000] = 1',
        '[ClosedCaptions::TextGfxDrawer] draw text:[a grant from]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Anne Ray Foundat]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[ion,]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[a Margaret A. Ca]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[rgill]'
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=177"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[Philanthropy.]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1051 missing"

    print("test_1051".ljust(50), "Test Passed")