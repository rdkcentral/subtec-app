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

font_file_list = [
'/usr/share/fonts/Cinecav_Mono.ttf',
'/usr/share/fonts/Cinecav_Type.ttf',
'/usr/share/fonts/Cinecav_Serif.ttf',
'/usr/share/fonts/Cinecav_Sans.ttf',
'/usr/share/fonts/Cinecav_Casual.ttf',
'/usr/share/fonts/Cinecav_Script.ttf',
'/usr/share/fonts/Cinecav_Sc.ttf'
]

@pytest.mark.cc_608
@pytest.mark.attributes
def test_1034():
    print("test_1034".ljust(50), "Test Started")

    assert check_font_files(font_file_list), "Font files are not installed in your system ! This test will fail. Copy fonts to /usr/share/fonts/ to continue ! "

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:48:00]")
    copy_log_file(subtec_log_path, subtec_copy_path)

    sequence = [
        'onPacketReceived packet=type:SUBTITLE_SELECTION',
        'Selecting CC Subtitles',
        'CcSubController created',
        '[ClosedCaptions::CcController] setActiveService: type 0, service 1002',
        'onPacketReceived packet=type:UNMUTE']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1034 missing"

    sequence = [
        'new value for attibId[0x80] = 1',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=18"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:23:00]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1034 missing"

    sequence = [
        'new value for attibId[0x80] = 0',
        'new value for attibId[0x40] = 1',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:24:00]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=24"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:25:00]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1034 missing"

    sequence = [
        'new value for attibId[0x20] = 0',
        'new value for attibId[0x40] = 0',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:26:00]',
        ]
    marker_string1 = "onPacketReceived packet=type:SET_CC_ATTRIBUTES counter=30"
    marker_string2 = "[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:27:00]"
    assert check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1034 missing"

    sequence = [
        'onPacketReceived packet=type:SET_CC_ATTRIBUTES',
        'new value for attibId[0x20] = 1',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:28:00]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:29:00]',
        'onPacketReceived packet=type:SET_CC_ATTRIBUTES',
        'new value for attibId[0x20] = 2',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:30:00]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:31:00]',
        'onPacketReceived packet=type:SET_CC_ATTRIBUTES',
        'new value for attibId[0x20] = 3',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:32:00]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:33:00]',
        'onPacketReceived packet=type:SET_CC_ATTRIBUTES',
        'new value for attibId[0x10] = 1',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Type.ttf',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:34:00]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:35:00]',
        'onPacketReceived packet=type:SET_CC_ATTRIBUTES',
        'new value for attibId[0x10] = 2',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Serif.ttf',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:36:00]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:37:00]',
        'onPacketReceived packet=type:SET_CC_ATTRIBUTES',
        'new value for attibId[0x10] = 3',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:38:00]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:39:00]',
        'onPacketReceived packet=type:SET_CC_ATTRIBUTES',
        'new value for attibId[0x10] = 4',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Sans.ttf',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:40:00]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:41:00]',
        'onPacketReceived packet=type:SET_CC_ATTRIBUTES',
        'new value for attibId[0x10] = 5',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Casual.ttf',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:42:00]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:43:00]',
        'onPacketReceived packet=type:SET_CC_ATTRIBUTES',
        'new value for attibId[0x10] = 6',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Script.ttf',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:44:00]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:45:00]',
        'onPacketReceived packet=type:SET_CC_ATTRIBUTES',
        'new value for attibId[0x10] = 7',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Sc.ttf',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:46:00]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:47:00]',
        '[ClosedCaptions::TextGfxDrawer] draw text:[swe: 00:00:48:00]']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1034 missing"

    print("test_1034".ljust(50), "Test Passed")