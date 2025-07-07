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
def test_1078():
    print("test_1078".ljust(50), "Test Started")

    subtec_copy_path = "./output_files/" + get_log_filename()
    get_last_read_position(subtec_log_path)

    ## Run pre-requisites - find cpp test file, compile it, ensure subtec is started, run test
    run_test_prereq(script_dir)

    # wait for end of test
    wait_for_log_string(subtec_log_path, "displaying: [00:00:05.078-00:00:05.528]: [Test sample to test subtitle flickering]", 20)
    copy_log_file(subtec_log_path, subtec_copy_path)

    sequence = [
        'onPacketReceived packet=type:TTML_SELECTION',
        'TtmlController created',
        'unmute received',
        'onPacketReceived packet=type:TTML_TIMESTAMP',
        'displaying: [00:00:00.000-00:00:00.168]: [Test sample to test subtitle flickering]',
        'displaying: [00:00:02.168-00:00:02.648]: [Test sample to test subtitle flickering]',
        'displaying: [00:00:03.648-00:00:04.068]: [Test sample to test subtitle flickering]',
        'displaying: [00:00:05.078-00:00:05.528]: [Test sample to test subtitle flickering]']
    assert monitor_log_for_sequence(subtec_copy_path, sequence), "Logs for test_1078 missing"

    # To ensure there is no flickering, verify from logs that "renderer->clearscreen" was not
    # called after "renderer->update" 
    sequence = [
        '+[renderer->clearscreen]',
        ]
    marker_string1 = "-[renderer->update]"
    marker_string2 = "displaying: [00:00:02.168-00:00:02.648]: [Test sample to test subtitle flickering] "
    assert not check_logs_between_strings(subtec_copy_path, sequence, marker_string1, marker_string2), "Logs for test_1078 incorrect"

    print("test_1078".ljust(50), "Test Passed")
