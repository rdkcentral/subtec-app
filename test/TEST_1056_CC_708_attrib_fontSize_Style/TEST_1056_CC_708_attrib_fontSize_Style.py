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
def test_1056():
    print("test_1056".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "[ClosedCaptions::TextGfxDrawer] draw text:[Thank you.]")
    copy_log_file(subtec_log_path, subtec_copy_path)

    sequence = [
        'onPacketReceived packet=type:SUBTITLE_SELECTION',
        'Selecting CC Subtitles',
        'CcSubController created',
        '[ClosedCaptions::CcController] setActiveService: type 1, service 1',
        'onPacketReceived packet=type:UNMUTE',]
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1056 missing"

    sequence = [
        'new value for attibId[0x20] = 0',
        'new value for attibId[0x40] = 1',
        'new value for attibId[0x80] = 1',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=20"
    marker_string2 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=33"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1056 missing"

    sequence = [
        'new value for attibId[0x10] = 1',
        'new value for attibId[0x20] = 1',
        'new value for attibId[0x40] = 0',
        'new value for attibId[0x80] = 0',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Type.ttf',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Bound together]'
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=36"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[by a resilient spirit,]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1056 missing"

    sequence = [
        'new value for attibId[0x10] = 2',
        'new value for attibId[0x20] = 2',
        'new value for attibId[0x40] = 0',
        'new value for attibId[0x80] = 0',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Serif.ttf',
        '[ClosedCaptions::TextGfxDrawer] draw text:[in wild Scandinavia.]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[[Wind blowing]]',
        '[ClosedCaptions::TextGfxDrawer] draw text:["Wild Scandinavia"]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=66"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[was made possible in part by]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1056 missing"

    sequence = [
        'new value for attibId[0x10] = 3',
        'new value for attibId[0x20] = 3',
        'new value for attibId[0x40] = 0',
        'new value for attibId[0x80] = 0',
        ]
    marker_string1 = "[ClosedCaptions::TextGfxDrawer] draw text:[was made possible in part by]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1056 missing"

    sequence = [
        'onPacketReceived packet=type:SET_CC_ATTRIBUTES',
        'new value for attibId[0x10] = 4',
      #  'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Sans.ttf',
        '[ClosedCaptions::TextGfxDrawer] draw text:[a grant from]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Anne Ray Foundation,]',
        'onPacketReceived packet=type:SET_CC_ATTRIBUTES',
        'new value for attibId[0x10] = 5',
      #  'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Casual.ttf',
        '[ClosedCaptions::TextGfxDrawer] draw text:[a Margaret A. Cargill]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Philanthropy.]',
        'onPacketReceived packet=type:SET_CC_ATTRIBUTES',
        'new value for attibId[0x10] = 6',
       # 'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Script.ttf',
        '[ClosedCaptions::TextGfxDrawer] draw text:[and by contributions]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[ to]',
        'onPacketReceived packet=type:SET_CC_ATTRIBUTES',
        'new value for attibId[0x10] = 7',
       # 'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Sc.ttf',
        '[ClosedCaptions::TextGfxDrawer] draw text:[your PBS station]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[from viewers like you.]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[Thank you.]'
        ]
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1056 missing"

    print("test_1056".ljust(50), "Test Passed")