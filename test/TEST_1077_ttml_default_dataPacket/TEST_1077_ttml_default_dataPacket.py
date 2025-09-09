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
def test_1077():
    print("test_1077".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "displaying: [00:00:05.400-00:00:10.720]: [This code snippet tests a corner case of] [default whitespace handling]", 20)
    copy_log_file(subtec_log_path, subtec_copy_path)

    sequence = [
        'onPacketReceived packet=type:TTML_SELECTION',
        'TtmlController created',
        'unmute received',
        'onPacketReceived packet=type:TTML_TIMESTAMP',
        '[TtmlEngine::Parser] parseAttributes adding tts:backgroundColor=#000000ff',
        '[TtmlEngine::DocumentInstance] generateTimeline chunk: \'Text sample has only newline character and spaces in\', style: []: font: WHITE, default, 160%, bg: BLACK, lineH: 125%, text: center, display: before, outline: BLACK, 0c',
        '[TtmlEngine::DocumentInstance] generateTimeline chunk: \'first line, has black background\', style: []: font: WHITE, default, 160%, bg: BLACK, lineH: 125%, text: center, display: before, outline: BLACK, 0c',
        '[TtmlEngine::DocumentInstance] generateTimeline chunk: \'This code snippet tests a corner case of\', style: []: font: LIME, default, 160%, bg: BLACK, lineH: 125%, text: center, display: before, outline: BLACK, 0c',
        '[TtmlEngine::DocumentInstance] generateTimeline chunk: \'default whitespace handling\', style: []: font: WHITE, default, 160%, bg: BLACK, lineH: 125%, text: center, display: before, outline: BLACK, 0c',
        'displaying: [00:00:00.000-00:00:05.400]: [Text sample has only newline character and spaces in] [first line, has black background]',
        'displaying: [00:00:05.400-00:00:10.720]: [This code snippet tests a corner case of] [default whitespace handling]']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1077 missing"

    # Give strings that are not expected in log
    logs = ['[TtmlEngine::DocumentInstance] generateTimeline chunk: \' \', style: []: font: WHITE, default, 160%, bg: TRANSPARENT, lineH: 125%, text: center, display: before, outline: BLACK, 0c']
    assert validate_string_absence(subtec_copy_path, logs), "Logs for test_1077 incorrect"

    print("test_1077".ljust(50), "Test Passed")
