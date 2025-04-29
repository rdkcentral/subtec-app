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
def test_1055():
    print("test_1055".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "[ClosedCaptions::TextGfxDrawer] draw text:[anthropy.]")
    copy_log_file(subtec_log_path, subtec_copy_path)

    sequence = [
        'onPacketReceived packet=type:SUBTITLE_SELECTION',
        'Selecting CC Subtitles',
        'CcSubController created',
        '[ClosedCaptions::CcController] setActiveService: type 1, service 1',
        'onPacketReceived packet=type:UNMUTE',]
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1055 missing"

    sequence = [
        'new value for attibId[0x20] = 3',
        'new value for attibId[0x2] = 0',
        'new value for attibId[0x1] = 16777215',
        'new value for attibId[0x1000] = 4',
        'new value for attibId[0x2000] = 0',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Bound together]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[by a resilient spirit,]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=22"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[in wild Scandinavia.]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1055 missing"

    sequence = [
        'new value for attibId[0x2000] = 16777215',
        '[ClosedCaptions::TextGfxDrawer] draw text:[[Wind blowing]]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=88"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:["Wild Scandinavia"]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1055 missing"

    sequence = [
        'new value for attibId[0x2000] = 16711680',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=119"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:["Wild Scandinavia"]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1055 missing"

    sequence = [
        'new value for attibId[0x2000] = 65280',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=135"
    marker_string2 = 'draw text:[was made possible in part by]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1055 missing"

    sequence = [
        'new value for attibId[0x2000] = 255',
        '[ClosedCaptions::TextGfxDrawer] draw text:[a grant from]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Anne Ray Fou]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=156"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[ndation,]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1055 missing"

    sequence = [
        'new value for attibId[0x2000] = 16776960',
        '[ClosedCaptions::TextGfxDrawer] draw text:[a grant from]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Anne Ray Fou]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=172"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[ndation,]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1055 missing"

    sequence = [
        'new value for attibId[0x2000] = 16711935',
        '[ClosedCaptions::TextGfxDrawer] draw text:[a grant from]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Anne Ray Fou]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[ndation,]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[a Margaret]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[ A. Cargill]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Phil]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=183"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[anthropy.]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1055 missing"

    sequence = [
        'new value for attibId[0x2000] = 65535',
        '[ClosedCaptions::TextGfxDrawer] draw text:[a Margaret]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[ A. Cargill]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Phil]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=199"
    marker_string2 = '[ClosedCaptions::TextGfxDrawer] draw text:[anthropy.]'
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1055 missing"

    print("test_1055".ljust(50), "Test Passed")