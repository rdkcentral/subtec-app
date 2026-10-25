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
def test_1083():
    print("test_1083".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for the final TTML case to be displayed before snapshotting logs
    wait_for_log_string(subtec_log_path, "displaying: [00:00:26.000-00:00:30.000]: [BR06 trailing br keeps prior text]", 45)
    copy_log_file(subtec_log_path, subtec_copy_path)

    sequence = [
        'onPacketReceived packet=type:TTML_SELECTION',
        'TtmlController created',
        'unmute received',
        'onPacketReceived packet=type:TTML_TIMESTAMP',
        'parseAttributes adding xml:id=s0',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=#FFFF00FF',
        'parseAttributes adding tts:fontSize=1c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif',
        'parseAttributes adding xml:id=transparentspace',
        'parseAttributes adding tts:backgroundColor=#000000FF',
        'parseAttributes adding tts:textAlign=left',
        'parseAttributes adding tts:color=#FFFF00FF',
        'parseAttributes adding tts:fontSize=1c',
        'parseAttributes adding tts:fontFamily=proportionalSansSerif']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1083 missing"

    # Case 5 validates two successive <br/> transitions with three visible lines.
    # A truly blank middle line is not observable from the info-level display log,
    # so this L2 test keeps the middle line visible and verifies line preservation.
    sequence = [
        'displaying: [00:00:00.000-00:00:05.000]: [BR01 direct before br] [BR01 validate direct after br]',
        'displaying: [00:00:06.000-00:00:10.000]: [BR02 pretty print before br] [BR02 validate trimmed indent after br]',
        'displaying: [00:00:11.000-00:00:15.000]: [BR03 span before br] [BR03 validate plain text after br]',
        'displaying: [00:00:16.000-00:00:20.000]: [BR04 plain text before br] [BR04 validate span after br]',
        'displaying: [00:00:21.000-00:00:25.000]: [BR05 first break line] [BR05 middle line survives] [BR05 second break line]',
        'displaying: [00:00:26.000-00:00:30.000]: [BR06 trailing br keeps prior text]']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1083 missing"

    print("test_1083".ljust(50), "Test Passed")