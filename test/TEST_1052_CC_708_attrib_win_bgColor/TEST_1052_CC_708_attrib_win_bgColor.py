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
def test_1052():
    print("test_1052".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "[ClosedCaptions::TextGfxDrawer] draw text:[ur PBS station]")
    copy_log_file(subtec_log_path, subtec_copy_path)

    sequence = [
        'onPacketReceived packet=type:SUBTITLE_SELECTION',
        'Selecting CC Subtitles',
        'CcSubController created',
        '[ClosedCaptions::CcController] setActiveService: type 1, service 1',
        'onPacketReceived packet=type:UNMUTE',]
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1052 missing"

    sequence = [
        'new value for attibId[0x20] = 3',
        'new value for attibId[0x800] = 0',
        'new value for attibId[0x400] = 0',
        'new value for attibId[0x2] = 16777215',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Bound together]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=21"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[by a resilient spirit,]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1052 missing"

    sequence = [
        'new value for attibId[0x400] = 16777215',
        'new value for attibId[0x2] = 0',
        '[ClosedCaptions::TextGfxDrawer] draw text:[in wild Scandinavia.]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[[Wind blowing]]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=63"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:["Wild Scandinavia"]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1052 missing"

    sequence = [
        'new value for attibId[0x400] = 16711680',
        'new value for attibId[0x2] = 65280',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=121"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:["Wild Scandinavia"]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1052 missing"

    sequence = [
        'new value for attibId[0x400] = 65280',
        'new value for attibId[0x2] = 16711680',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=139"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[was made possible in part by]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1052 missing"

    sequence = [
        'new value for attibId[0x400] = 255',
        'new value for attibId[0x2] = 16776960',
        '[ClosedCaptions::TextGfxDrawer] draw text:[a grant from]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Anne Ray Foundat]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=161"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[ion,]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1052 missing"

    sequence = [
        'new value for attibId[0x400] = 16776960',
        'new value for attibId[0x2] = 255',
        '[ClosedCaptions::TextGfxDrawer] draw text:[a grant from]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Anne Ray Foundat]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[ion,]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[a Margaret A. Ca]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[rgill]'
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=179"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[Philanthropy.]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1052 missing"

    sequence = [
        'new value for attibId[0x400] = 16711935',
        'new value for attibId[0x2] = 65535',
        '[ClosedCaptions::TextGfxDrawer] draw text:[a Margaret A. Ca]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[rgill]'
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=197"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[Philanthropy.]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1052 missing"

    sequence = [
        'new value for attibId[0x400] = 65535',
        'new value for attibId[0x2] = 16711935',
        '[ClosedCaptions::TextGfxDrawer] draw text:[a Margaret A. Ca]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[rgill]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Philanthropy.]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[and by contributions to]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[yo]'
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=215"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[ur PBS station]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1052 missing"

    print("test_1052".ljust(50), "Test Passed")