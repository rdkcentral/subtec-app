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

@pytest.mark.cc_608
@pytest.mark.attributes
def test_1039():
    print("test_1039".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:38:00]")
    copy_log_file(subtec_log_path, subtec_copy_path)

    sequence = [
        'onPacketReceived packet=type:SUBTITLE_SELECTION',
        'Selecting CC Subtitles',
        'CcSubController created',
        '[ClosedCaptions::CcController] setActiveService: type 0, service 1002',
        'onPacketReceived packet=type:UNMUTE']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1039 missing"

    sequence = [
        'new value for attibId[0x20] = 3',
        'new value for attibId[0x800] = 0',
        'new value for attibId[0x400] = 0',
        'new value for attibId[0x2] = 16777215',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=21"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:23:00]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1025 missing"

    sequence = [
        'new value for attibId[0x400] = 16777215',
        'new value for attibId[0x2] = 0',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:24:00]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=27"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:25:00]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1025 missing"

    sequence = [
        'new value for attibId[0x400] = 16711680',
        'new value for attibId[0x2] = 65280',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:26:00]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=33"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:27:00]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1025 missing"

    sequence = [
        'new value for attibId[0x400] = 65280',
        'new value for attibId[0x2] = 16711680',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:28:00]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=39"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:29:00]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1025 missing"

    sequence = [
        'new value for attibId[0x400] = 255',
        'new value for attibId[0x2] = 16776960',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:30:00]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:31:00]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=45"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:32:00]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1025 missing"

    sequence = [
        'new value for attibId[0x400] = 16776960',
        'new value for attibId[0x2] = 255',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=51"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:33:00]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1025 missing"

    sequence = [
        'new value for attibId[0x400] = 16711935',
        'new value for attibId[0x2] = 65535',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:34:00]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=57"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:35:00]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1025 missing"

    sequence = [
        'new value for attibId[0x400] = 65535',
        'new value for attibId[0x2] = 16711935',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:36:00]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:37:00]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=63"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:38:00]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1025 missing"


    print("test_1039".ljust(50), "Test Passed")