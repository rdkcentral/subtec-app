#!/usr/bin/env python3
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
import subprocess
import shutil
import re
import sys
import argparse
import textwrap
import xml.etree.ElementTree as ET
import datetime
from utils import *

debug_log("run_l2_tests started")

# Build libsubtec.so required for tests to run, and copy it to test folder
def build_and_copy_lib():
    # Define paths
    makefile_dir = os.path.join(os.path.dirname(__file__), 'libsubtec')
    output_dir = os.path.dirname(__file__)
    lib_name = 'libsubtec.so'

    # Change working directory to where the Makefile is located
    os.chdir(makefile_dir)

    # Run the make command
    try:
        ret = subprocess.run(['make'], check=True)
        if ret.returncode:
            debug_log("Error during make for libsubtec.so, with return code" << ret.returncode)
            debug_log("ERROR : " << ret.stderr)
    except subprocess.CalledProcessError as e:
        debug_log(f"Error during make for libsubtec.so : {e}")
        return

    # Define source and destination file paths
    src_file = os.path.join(makefile_dir, lib_name)
    dest_file = os.path.join(output_dir, lib_name)

    # Copy the .so file to the test/ directory
    try:
        shutil.copy(src_file, dest_file)
    except FileNotFoundError:
        debug_log(f"File {lib_name} not found in {makefile_dir}.")
    except IOError as e:
        debug_log(f"Error copying file libsubtec.so to test folder : {e}")

    os.chdir(output_dir)

# Parse junit_results.xml and store status of tests
def parse_junit_xml(xml_file):
    tree = ET.parse(xml_file)
    root = tree.getroot()
    
    data = []
    
    for testsuite in root.findall('testsuite'):     
        for testcase in testsuite.findall('testcase'):
            classname = testcase.get('classname')

            # Extract relevant part of classname
            if classname:
                parts = classname.split('.')
                if len(parts) > 1:
                    # Assume the relevant part is after the last dot
                    case_name = parts[-1]
                else:
                    # If there's no dot, use the classname as is
                    case_name = classname
            status = 'PASS'
            
            if testcase.find('failure') is not None:
                status = 'FAIL'
            
            data.append([case_name, status])
    
    return data

# Print tests executed with status PASS/FAIL
def print_table(data):
    # Define headers
    header = ["Test Case Name", "Status"]
    
    # Determine the maximum length of each column for formatting, including headers
    max_lengths = [max(len(str(item[i])) for item in data) for i in range(len(data[0]))]
    max_lengths = [max(len(header[i]), max_lengths[i]) for i in range(len(header))]  # Include header lengths

    # Calculate the total width of the table (including vertical lines and padding)
    total_width = sum(max_lengths) + 7  # 7 spaces for padding and vertical lines

    # Print top horizontal line
    print('+' + '-' * (total_width - 2) + '+')  # -2 for the border '+' characters

    # Print header
    header_row = f"| {header[0]:<{max_lengths[0]}} | {header[1]:<{max_lengths[1]}} |"
    print(header_row)

    # Print separator line below header
    print('+' + '-' * (total_width - 2) + '+')  # -2 for the border '+' characters

    # Print rows
    for row in data:
        row_line = f"| {row[0]:<{max_lengths[0]}} | {row[1]:<{max_lengths[1]}} |"
        print(row_line)

    # Print bottom horizontal line
    print('+' + '-' * (total_width - 2) + '+')  # -2 for the border '+' characters

def install_python_dependencies():
    debug_log("install_python_dependencies")
    # Installing python test packages

    if os.path.isfile("requirements.txt"):
        debug_log("Using requirements.txt to install packages..")
        ret = subprocess.run('python3 -m pip install -r requirements.txt', shell=True)
        if ret.returncode:
            debug_log("ERROR python3 -m pip install failed with return code" << ret.returncode)
            debug_log("ERROR : " << ret.stderr)
            exit(1)

class StreamWrapper:
    """
    Wrapper for stdout to write to console and file
    """
    def __init__(self, stream):
        self.stream = stream
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"run_l2_subtec_{timestamp}.log"
        self.file = open(filename, 'w')  # Open timestamped log file

    def write(self, data):
        self.stream.write(data)
        self.file.write(data)    # Write to stdout and file

    def __getattr__(self, attr):
        return getattr(self.stream, attr)

sys.stdout = StreamWrapper(sys.stdout)
sys.stderr = sys.stdout
print(sys.argv)

epilog_string = ""
if 'VIRTUAL_ENV' not in os.environ:
    epilog_string = textwrap.dedent('''\
        Create and activate a virtual environment before running tests : -
            python3 -m venv l2venv
            source l2venv/bin/activate
        ''')

# Get test cases to execute or exclude.
# If no options provided, execute all test cases.
parser = argparse.ArgumentParser(description= "Script to run subtec L2 tests"
                                , epilog = epilog_string)

parser.add_argument("-i", "--testsuites_run", nargs='*', help="Test case numbers to execute")
parser.add_argument("-e", "--testsuites_exclude", nargs='*', help="Test case numbers to exclude")

# Parse the arguments
args, other_args = parser.parse_known_args()

install_python_dependencies()
import pytest

test_cwd = os.getcwd()

# Get list of tests
test_suite_dirs = {}  # E.G{ 1001: 'TEST_1001_something' ...}
test_suite_dirs_to_run = []
for directory in os.listdir(test_cwd):
    m = re.match(r'[A-Z_\-]+(\d{4,})', directory)
    if os.path.isfile(directory):
        pass
    elif m:
        num = m.group(1)
        if num in test_suite_dirs:
            debug_log("ERROR Duplicate numbers from directories {} {} expecting unique".format
                  (test_suite_dirs[num], directory))
            exit(1)
        if args.testsuites_run:
            if num in args.testsuites_run:
                test_suite_dirs[num] = directory
        elif args.testsuites_exclude:
            if num not in args.testsuites_exclude:
                test_suite_dirs[num] = directory
        else:
            test_suite_dirs[num] = directory

# Find list of tests to be executed
for num in sorted(test_suite_dirs.keys()):
    item_path = os.path.join(test_cwd, test_suite_dirs[num])
    if os.path.isdir(item_path) and test_suite_dirs[num].startswith("TEST_"):
        test_script = os.path.join(item_path, f"{test_suite_dirs[num]}.py")
        if os.path.isfile(test_script):
            test_suite_dirs_to_run.append(test_script)

build_and_copy_lib()

xml_results_file = 'junit_results.xml'

# Base options
options = ['-s', f"--junitxml={xml_results_file}"]
# Add other args to options, like -m for markers
options.extend(other_args)
# Add test suites to the options
options.extend(test_suite_dirs_to_run)
ret_code = pytest.main(options)

test_data = parse_junit_xml(xml_results_file)
print_table(test_data)

cleanup_subtec()