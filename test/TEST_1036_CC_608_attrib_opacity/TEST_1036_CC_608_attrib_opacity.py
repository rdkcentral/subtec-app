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
def test_1036():
    print("test_1036".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:28:00]")
    copy_log_file(subtec_log_path, subtec_copy_path)

    sequence = [
        'onPacketReceived packet=type:SUBTITLE_SELECTION',
        'Selecting CC Subtitles',
        'CcSubController created',
        '[ClosedCaptions::CcController] setActiveService: type 0, service 1002',
        'onPacketReceived packet=type:UNMUTE']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1036 missing"

    sequence = [
        'new value for attibId[0x20] = 3',
        'new value for attibId[0x1] = 255',
        'new value for attibId[0x4] = 0',
        'new value for attibId[0x8] = 0',
        'new value for attibId[0x800] = 0',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=22"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:23:00]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1036 missing"

    sequence = [
        'new value for attibId[0x4] = 2',
        'new value for attibId[0x8] = 2',
        'new value for attibId[0x800] = 2',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:24:00]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=29"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:25:00]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1036 missing"

    sequence = [
        'new value for attibId[0x4] = 3',
        'new value for attibId[0x8] = 3',
        'new value for attibId[0x800] = 3',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:26:00]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:27:00]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=36"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:28:00]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1036 missing"

    print("test_1036".ljust(50), "Test Passed")