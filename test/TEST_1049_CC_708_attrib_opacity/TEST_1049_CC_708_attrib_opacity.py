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
def test_1049():
    print("test_1049".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "[ClosedCaptions::TextGfxDrawer] draw text:[Anne Ray Foundation,]")
    copy_log_file(subtec_log_path, subtec_copy_path)

    sequence = [
        'onPacketReceived packet=type:SUBTITLE_SELECTION',
        'Selecting CC Subtitles',
        'CcSubController created',
        '[ClosedCaptions::CcController] setActiveService: type 1, service 1',
        'onPacketReceived packet=type:UNMUTE',]
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1049 missing"

    sequence = [
        'new value for attibId[0x20] = 3',
        'new value for attibId[0x1] = 16711680',
        'new value for attibId[0x400] = 255',
        'new value for attibId[0x4] = 3',
        'new value for attibId[0x8] = 3',
        'new value for attibId[0x800] = 0',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Bound together]'
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=23"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[by a resilient spirit,]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1049 missing"

    sequence = [
        'new value for attibId[0x400] = 16711680',
        'new value for attibId[0x4] = 2',
        'new value for attibId[0x8] = 2',
        'new value for attibId[0x800] = 2',
        '[ClosedCaptions::TextGfxDrawer] draw text:[in wild Scandinavia.]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[[Wind blowing]]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=67"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:["Wild Scandinavia"]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1049 missing"

    sequence = [
        'new value for attibId[0x400] = 65280',
        'new value for attibId[0x4] = 0',
        'new value for attibId[0x8] = 0',
        'new value for attibId[0x800] = 3',
        ]
    marker_string1 = "[ClosedCaptions::TextGfxDrawer] draw text:[was made possible in part by]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1), "Logs for test_1049 missing"

    sequence = [
        '[ClosedCaptions::TextGfxDrawer] draw text:[a grant from]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Anne Ray Foundation,]',
    ]
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1049 missing"

    print("test_1049".ljust(50), "Test Passed")