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

@pytest.mark.webvtt
@pytest.mark.attributes
def test_1023():
    print("test_1023".ljust(50), "Test Started")

    assert check_font_files(font_file_list), "Font files are not installed in your system ! This test will fail. Copy fonts to /usr/share/fonts/ to continue ! "

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "displaying: [00:00:00.000-00:00:05.000]: Sample text to test FONT_STYLE_SMALL_CAPITALS")
    copy_log_file(subtec_log_path, subtec_copy_path)

    sequence = [
        'onPacketReceived packet=type:RESET_CHANNEL',
        'onPacketReceived packet=type:WEBVTT_SELECTION', 
        'onPacketReceived packet=type:UNMUTE',
        'onPacketReceived packet=type:WEBVTT_TIMESTAMP',
        'new value for attibId[0x10] = -1', 
        'displaying: [00:00:00.000-00:00:05.000]: Sample text to test FONT_STYLE_EMBEDDED', 
        '']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1023 missing"
    
    sequence = [
        'new value for attibId[0x10] = 0',
        'displaying: [00:00:00.000-00:00:05.000]: Sample text to test FONT_STYLE_DEFAULT',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Mono.ttf']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1023 missing"
    
    sequence = [
        'new value for attibId[0x10] = 1',
        'displaying: [00:00:00.000-00:00:05.000]: Sample text to test FONT_STYLE_MONOSPACED_SERIF',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Type.ttf']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1023 missing"
    
    sequence = [
        'new value for attibId[0x10] = 2',
        'displaying: [00:00:00.000-00:00:05.000]: Sample text to test FONT_STYLE_PROPORTIONAL_SERIF',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Serif.ttf']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1023 missing"
    
    sequence = [
        'new value for attibId[0x10] = 3',
        'displaying: [00:00:00.000-00:00:05.000]: Sample text to test FONT_STYLE_MONOSPACED_SANSSERIF',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Mono.ttf']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1023 missing"
    
    sequence = [
        'new value for attibId[0x10] = 4',
        'displaying: [00:00:00.000-00:00:05.000]: Sample text to test FONT_STYLE_PROPORTIONAL_SANSSERIF',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Sans.ttf']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1023 missing"
    
    sequence = [
        'new value for attibId[0x10] = 5',
        'displaying: [00:00:00.000-00:00:05.000]: Sample text to test FONT_STYLE_CASUAL',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Casual.ttf']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1023 missing"
    
    sequence = [
        'new value for attibId[0x10] = 6',
        'displaying: [00:00:00.000-00:00:05.000]: Sample text to test FONT_STYLE_CURSIVE',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Script.ttf']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1023 missing"

    sequence = [
        'new value for attibId[0x10] = 7',
        'displaying: [00:00:00.000-00:00:05.000]: Sample text to test FONT_STYLE_SMALL_CAPITALS',
        'findFontFile - Matching font found: /usr/share/fonts/Cinecav_Sc.ttf']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1023 missing"

    print("test_1023".ljust(50), "Test Passed")