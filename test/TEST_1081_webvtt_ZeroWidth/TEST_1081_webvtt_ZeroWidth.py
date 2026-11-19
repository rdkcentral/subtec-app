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

@pytest.mark.webvtt
def test_1081():
    print("test_1081".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    # Run pre-requisites - compile cpp file, ensure subtec is started, run test.
    run_test_prereq(script_dir)

    # Wait until the final normal cue is processed.
    wait_for_log_string(subtec_log_path, "displaying: [00:00:20.000-00:00:30.000]: NORMAL right edge clipping with zero width overflow sample text zero width overflow sample text zero width overflow sample text zero width overflow sample text.", 45, 2)
    copy_log_file(subtec_log_path, subtec_copy_path)

    log_sequence = [
        'validate - RESET ALL received - resetting stream state',
        'onPacketReceived packet=type:RESET_CHANNEL',
        'onPacketReceived packet=type:WEBVTT_SELECTION',
        'onPacketReceived packet=type:UNMUTE',
        'onPacketReceived packet=type:WEBVTT_TIMESTAMP',
        'displaying: [00:00:00.000-00:00:05.000]: CASE 1 short text fits on screen.',
        'renderer->renderDocument - cue number:1',
        'displaying: [00:00:05.000-00:00:10.000]: CASE 2 right edge clipping with zero width overflow sample text zero width overflow sample text zero width overflow sample text zero width overflow sample text.',
        'renderer->renderDocument - cue number:1',
        'displaying: [00:00:10.000-00:00:15.000]: CASE 3 centered zero width cue can overflow on both sides and look oddly anchored instead of behaving like a narrow wrapped subtitle box.',
        'renderer->renderDocument - cue number:1',
        'displaying: [00:00:15.000-00:00:20.000]: CASE 4 end aligned zero width cue is anchored to the right edge and the long single line can extend leftward far beyond where a wrapped cue would normally sit on screen.',
        'renderer->renderDocument - cue number:1',
        'displaying: [00:00:20.000-00:00:30.000]: NORMAL right edge clipping with zero width overflow sample text zero width overflow sample text zero width overflow sample text zero width overflow sample text.',
        'renderer->renderDocument - cue number:1']
    assert monitor_log_for_sequence(subtec_copy_path, log_sequence), "Logs for test_1081 missing"
    assert validate_string_absence(subtec_copy_path, ['Floating point exception']), "Unexpected floating point exception logged"
    assert is_process_running("subttxrend-app"), "subttxrend-app exited after rendering zero-width cue"

    print("test_1081".ljust(50), "Test Passed")
