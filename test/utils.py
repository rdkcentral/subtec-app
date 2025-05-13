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
import time
import glob
import inspect
import shutil
import re

# Global - since subtec is not restarted for every test, we keep track of last
# read position in log file to use to validate next tests.
last_read_position = 0

def copy_log_file(subtec_common_log, subtec_copy_path):
    """
    Copies subtec common log to a specified destination for every test

    This function ensures that the destination directory exists before
    attempting to copy the log file.

    Args:
        subtec_common_log (str): Full path to subtec common log file
                                that needs to be copied.
        subtec_copy_path (str): Filename and full path to the desired
                                destination where the log file should be 
                                copied.

    Returns:
        None.
    """
    os.makedirs(os.path.dirname(subtec_copy_path), exist_ok=True)
    shutil.copy2(subtec_common_log, subtec_copy_path)

def debug_log(string_to_print):
    """
    Custom log to print the filename, line number, and function name

    This function extracts filename, line number and function
    name of calling function, pads this string to 50 characters
    before printing passed string.

    Args:
        string_to_print (str): String to pass to python function print() 

    Returns:
        None.
    """
    # Using inspect to extract every variable is expensive, so use
    # temporary variables to extract current frame
    caller_function = inspect.currentframe().f_back
    caller_function_fcode = caller_function.f_code

    filename = os.path.basename(caller_function_fcode.co_filename)
    line_number = caller_function.f_lineno
    function_name = caller_function_fcode.co_name

    # If print is from a script, skip function_name
    if function_name == "<module>":
        info_str = f"[{filename}:{line_number}]"
    else:
        info_str = f"[{function_name}, {filename}:{line_number}]"

    formatter_str = f"{info_str:50}" # left align, pad with spaces

    print(f"{formatter_str} {string_to_print}")

def start_subtec(script_path, subtec_log_path = "subtec_console_common.log"):
    """
    Start subtec app using build.sh script

    This function executes build.sh script with "run" argument,
    redirecting both standard output and standard error to the specified logfile.
    'stdbuf -o0' is used to ensure that the output is unbuffered. This 
    subprocess runs in the background.

    Args:
        script_path (str): Path to build.sh script relative to utils.py
        subtec_log_path (str): Filename and path to use to redirect subtec app 
                                logs.

    Returns:
        None.
    """
    # Now a new log file will be created, so reset this variable
    global last_read_position
    last_read_position = 0

    with open(subtec_log_path, 'w+') as logfile:
        os.chdir(script_path)

        run_command = ['stdbuf', '-o0', './build.sh', 'run']
        try:
            result = subprocess.Popen(run_command, stdout=logfile, stderr=subprocess.STDOUT)
            debug_log("Start build.sh and subtec app")
            return True
        except subprocess.CalledProcessError as e:
            debug_log("Execution failed for build.sh run")
            debug_log(e.stderr)
            debug_log(e.stdout)
            return False
        except Exception as e:
            debug_log(f"Unexpected error for build.sh run : {e}")
            return False

def is_process_running(process_name):
    """
    Checks if given process is running

    This function runs pgrep command with given process name to check
    if this process is already running. This is used to check if subtec
    app is already running or needs to be started before every test.

    Args:
        process_name (str): Process to check if it is running or not

    Returns:
        True : if the given process is running
    """
    try:
        # Find the process ID (PID) of the process with the given name
        result = subprocess.run(['pgrep', '-f', process_name], capture_output=True, text=True)
        if result.stdout:
            #debug_log(f"{process_name} already running")

            return True
        #else:
           # debug_log(f"No process named {process_name} found")
    except Exception as e:
        debug_log(f"Error finding or killing process: {e}")

    return False

def find_and_kill_process(process_name):
    """
    Finds process id of a given process and kills it

    This function runs pgrep command with given process name to check
    if this process is running. Then the process id is extracted and
    passed to kill command to kill the process as part of subtec cleanup.

    Args:
        process_name (str): Process to kill, if it is running.

    Returns:
        True : if the given process is running
    """
    try:
        # Find the process ID (PID) of the process with the given name
        result = subprocess.run(['pgrep', '-f', process_name], capture_output=True, text=True)
        if result.stdout:
            pids = result.stdout.strip().split('\n')
            for pid in pids:
                debug_log(f"Killing process {process_name} with PID {pid}")
                ret = subprocess.run(['kill', '-9', pid])
                if ret.returncode:
                    debug_log("ERROR kill failed with return code" << ret.returncode)
                    debug_log("ERROR : " << ret.stderr)
       # else:
          #  debug_log(f"No process named {process_name} found")
    except Exception as e:
        debug_log(f"Error finding or killing process: {e}")

def check_and_restart_subtec():
    """
    Ensures subtec is running before every test

    This function checks if build.sh and subttxrend-app processes are running
    before starting every test. If one of the processes is not running, it kills
    both and starts subtec app again. It waits for "subttxrend-app started" 
    string to make sure subtec is started again.

    Args:
        None

    Returns:
        None
    """
    debug_log("check_and_restart_subtec if required")
    process_names = ["build.sh", "subttxrend-app"]
    all_running = True

    for name in process_names:
        if not is_process_running(name):
            all_running = False

    if not all_running:
        # Kill both processes if either is not running, then restart
        cleanup_subtec()

        start_subtec("../../subttxrend-app/x86_builder", "../subtec_console_common.log")

        # wait to be sure subtec has started
        assert wait_for_log_string("../../test/subtec_console_common.log", "subttxrend-app started", 500, 1), "Subtec not started, check !"

def cleanup_subtec():
    """
    Cleans up subtec process and build.sh

    This function checks if build.sh and subttxrend-app processes are running
    and kills them as part of clean up.

    Args:
        None

    Returns:
        None
    """
    debug_log("cleanup_subtec")
    process_names = ["build.sh", "subttxrend-app"]
    for name in process_names:
        find_and_kill_process(name)

def calculate_webvtt_font_size (log_file_path, font_height):
    """
    Calculates webvtt font size

    This function checks the display height set during run time by
    parsing the log, and uses that display height to calculate font size
    similar to the logic in WebVTTConverter.hpp to validate font size.

    Args:
        log_file_path (str) : File name and full path to log file to use
        font_height (int) : kFontHeight value for each font size

    Returns:
        vh_to_pixels_result (int) : calculated font size
    """
    # Check in first 512kb, as the string we are looking for is present
    # within first few hundred lines of log
    chunk_size = 524288
    pattern = r"\[WebvttEngine::LineBuilder\] getOutputLines - line added rect:rect:\[(\d+);(\d+)\]\[(\d+)x(\d+)\] position:"

    # Open the log file for reading
    with open(log_file_path, 'r+', errors='ignore') as file:
        # Move to last read position, as every test appends logs to
        # the same console log file. This is done to avoid parsing the whole
        # log file for every test.
        file.seek(last_read_position)

        content = file.read(chunk_size)
        match = re.search(pattern, content)
        if match:
            extracted_height = int(match.group(2)) + int(match.group(4))

            hundredths_to_v_result = float(font_height) / 100.0
            display_height_float = float(extracted_height)
            vh_to_pixels_float = (display_height_float / 100.0) * hundredths_to_v_result
            vh_to_pixels_result = int(vh_to_pixels_float)
            debug_log(f"Calculated font size {vh_to_pixels_result} for font height {font_height}")

            return vh_to_pixels_result

def monitor_log_for_sequence(log_file_path, sequence, chunk_size = 524288):
    """
    Monitors log for expected messages in sequence.

    This function monitors the specified log file for the occurrence of a 
    specific sequence of strings, ensuring they appear in the correct order,
    even if not consecutively.
    This helps us validate tests by passing key strings as a sequence.
    This function is called multiple times for each test, this function
    could affect total run time of tests.

    Args:
        log_file_path (str) : File name and full path to log file to use
        sequence (list) : Sequence of expected log messages to validate the test
        chunk_size (int) : Set a chunk size to parse log file. This is usedful
                            when the file becomes bigger. Default : 512 kb

    Returns:
        True : if the entire sequence has been found in right order
        False : if one of the expected strings has not been found in log file.
    """
    # Initialize an index to track the position in the sequence
    sequence_index = 0
    remaining_line = ""

    # Open the log file for reading
    with open(log_file_path, 'r+', errors='ignore') as file:
        # Move to last read position, as every test appends logs to
        # the same console log file. This is done to avoid parsing the whole
        # log file for every test.
        file.seek(last_read_position)

        while True:
            # as log files becomes bigger with every test run, read file
            # in chunks to handle processing easier
            content = file.read(chunk_size)
            if not content:
                # Process any remaining partial line at the very end
                if remaining_line and sequence_index < len(sequence) and sequence[sequence_index] in remaining_line:
                    debug_log("check remaining_line for logs")
                    sequence_index += 1
                    if sequence_index == len(sequence):
                        return True
                break  #end of file

            # Find complete lines, and store partial line at end of chunk in remaining_line 
            lines = (remaining_line + content).splitlines(keepends=False)
            # This partial line is added to content in next chunk
            remaining_line = lines[-1] if content.endswith('\n') else lines[-1] if lines else ""
            current_lines_to_process = lines[:-1] if len(lines) > 1 else []

            # Process complete lines in the buffer
            for line in current_lines_to_process:
                # Check if the current line contains the current string in the sequence
                if sequence_index < len(sequence) and sequence[sequence_index] in line:
                    # Move to the next string in the sequence
                    sequence_index += 1
                    # If the entire sequence has been found, print a success message
                    if sequence_index == len(sequence):
                        #debug_log(f"Sequence {sequence} found in the log file.")
                        return True
                else:
                    # If the current line doesn't contain the expected string, continue
                    continue

        debug_log(f"String {sequence[sequence_index]} is not found in the log ! ")
    return False


def check_logs_between_strings(log_file_path, sequence, marker_string1, marker_string2=""):
    """
    Checks for expected logs between two given strings (in any sequence)

    This function checks if the given strings in sequence are present
    between either two instances of same string1 or between instances of
    string1 and string2.
    This is useful when the attribute packets received are logged in different 
    sequence in different runs.

    Args:
        log_file_path (str) : File name and full path to log file to use
        sequence (list) : Sequence of expected log messages to validate the test
        marker_string1 (str) : Reference marker string 1
        marker_string2 (str) : Reference marker string 2, optional if we want
                                logs to be checked between two instances of 
                                string1

    Returns:
        True : if the entire sequence has been found
        False : For error scenarios like if marker string(s) not found,
                start_index is less than end_index or if any expected log
                strings from sequence are not found in logs.   
    """
    # Open the log file for reading
    with open(log_file_path, 'r+', errors='ignore') as file:
        # Move to last read position, as every test appends logs to
        # the same console log file. This is done to avoid parsing the whole
        # log file for every test.
        file.seek(last_read_position)

        file_content = file.read()

        string1_occurrences = []
        file_index = 0
        end_index = 0

        # Find all occurrences of marker string1
        while True:
            file_index = file_content.find(marker_string1, file_index)
            if file_index == -1:
                break
            string1_occurrences.append(file_index)
            file_index += len(marker_string1)

        if not string1_occurrences:
            debug_log(f"\"{marker_string1}\" found number of times : {len(string1_occurrences)} ")
            return False

        # set start_index to first occurrence of first string
        start_index = string1_occurrences[0] + len(marker_string1)

        # If only one marker string is passed, set end_index to last occurrence of first string
        if marker_string2 == "":
            if len(string1_occurrences) < 2:
                debug_log(f"\"{marker_string1}\" found number of times : {len(string1_occurrences)} ")
                return False
            end_index = string1_occurrences[len(string1_occurrences) - 1]
        else:
            # Find all occurrences of marker string2
            string2_occurrences = []
            file_index = 0

            while True:
                file_index = file_content.find(marker_string2, file_index)
                if file_index == -1:
                    break
                string2_occurrences.append(file_index)
                file_index += len(marker_string2)

            if not string2_occurrences:
                debug_log(f"\"{marker_string2}\" found number of times : {len(string2_occurrences)} ")
                return False

            # set end_index to last occurrence of second string
            end_index = string2_occurrences[len(string2_occurrences) - 1]

        if start_index >= end_index:
            debug_log(f"start_index {start_index} is greater than end_index {end_index}")
            return False

        # read file section between calculated positions
        log_section = file_content[start_index:end_index]
        log_lines = log_section.splitlines()

        if sequence:
            for log in sequence:
                if not any(log in line for line in log_lines):
                    debug_log(f"{log} is not found")
                    return False  # Return False if any log is not found
          #  debug_log(f"{sequence} is found")
            return True #return true if all logs are found.

        #debug_log(f"{sequence} is found or no sequence passed !")
        return True #if no logs to check, return true, as the section was found.

def validate_string_absence(log_file_path, strings_to_exclude, chunk_size=524288):
    """
    Validates that a set of strings are not present in a log file.

    Args:
        log_file_path (str): Path to the log file.
        strings_to_exclude (list): List of strings that should NOT be present.
        chunk_size (int): The number of bytes to read in each chunk.

    Returns:
        True: if none of the strings in strings_to_exclude are found
              in the log file, False otherwise.
    """
    remaining_line = ""
    found_unwanted = set()  # Use a set to efficiently track found unwanted strings

    try:
        with open(log_file_path, 'r', errors='ignore') as file:
            # Move to last read position, as every test appends logs to
            # the same console log file. This is done to avoid parsing the whole
            # log file for every test.
            file.seek(last_read_position)

            while True:
                # as log files becomes bigger with every test run, read file
                # in chunks to handle processing easier
                content = file.read(chunk_size)
                if not content:
                    break  # End of file

                # Find complete lines, and store partial line at end of chunk in remaining_line 
                lines = (remaining_line + content).splitlines(keepends=False)
                # This partial line is added to content in next chunk
                remaining_line = lines[-1] if content.endswith('\n') else lines[-1] if lines else ""
                current_lines_to_process = lines[:-1] if len(lines) > 1 else []

                # Process complete lines in the buffer
                for line in current_lines_to_process:
                    for string_exclude in strings_to_exclude:
                        if string_exclude in line:
                            found_unwanted.add(string_exclude)
                            # No need to continue checking other unwanted strings in this line
                            break

                # Check the remaining partial line as well
                if remaining_line:
                    for string_exclude in strings_to_exclude:
                        if string_exclude in remaining_line:
                            found_unwanted.add(string_exclude)
                            break

                if found_unwanted:
                    debug_log("The following unwanted strings were found in the log file:")
                    for found in found_unwanted:
                        debug_log(f"- '{found}'")
                    return False

    except FileNotFoundError:
        debug_log(f"Error: Log file not found at {log_file_path}")
        return False
    except Exception as e:
        debug_log(f"An error occurred: {e}")
        return False

    debug_log("None of the specified unwanted strings were found in the log file")
    return True

def get_log_filename():
    """
    Creates a log file name with test number

    This function extracts name of calling function and creates a filename
    to store subtec console log for each test.

    Args:
        None

    Returns:
        filename (str) : subtec console log filename for each test
    """
    caller_function = inspect.stack()[1]
    test_name = caller_function.function
    filename = f"./{test_name}_subtec.log"
    return filename

def get_last_read_position(log_file):
    """
    Returns last_read_position value

    This function returns last_read_position value, for next test to use as
    starting position.

    Args:
        log_file (str) : File name and path of subtec log file to validate test

    Returns:
        None
    """
    global last_read_position
    if os.path.exists(log_file):
        try:
            with open(log_file, 'r+', errors='ignore') as file:
                # Seek to the end of file
                file.seek(0, 2)
                # Update the last read position (which is now end of file)
                last_read_position = file.tell()
        except IOError as e:
            debug_log(f"Error reading log file: {e}")
    else:
        # Update the last read position (which is now start of file)
        last_read_position = 0

def wait_for_log_string(log_file, search_string, timeout=15, check_interval=3):
    """
    Waits for occurrence of given log string

    This function checks log file for availability of given string, in given
    intervals up to seconds mentioned in timeout.

    Args:
        log_file (str) : File name and path of subtec log file to validate test
        search_string (str) : Log string to wait for
        timeout (int) : Maximum time to wait for the string occurence in log
        check_interval (int) : how often the log is checked

    Returns:
        True : if log is found within given Timeout period
    """
    counter = 0

    while counter < timeout:
        try:
            with open(log_file, 'r+', errors='ignore') as file:
                # Move to the last read position
                file.seek(last_read_position)
                # Read new lines
                new_lines = file.read()

                # Search for log string in new lines
                if search_string in new_lines:
                    debug_log(f"Found expected string - {search_string}")
                    return True

        except IOError as e:
            debug_log(f"Error reading log file: {e}")
            return False

        time.sleep(check_interval)
        counter += check_interval
    debug_log(f"Timeout: '{search_string}' not found in log file in '{counter}' seconds")
    return False

def check_font_files(font_file_list):
    """
    Check if required font files exist for font tests

    This function checks if the required font files exist in expected
    paths. Called before attribute tests involving font styles.

    Args:
        font_file_list (list) : File path for test folder

    Returns:
        True : if all required font style files are available
    """
    for file in font_file_list:
        if not os.path.exists(file):
            return False
    debug_log("All required font style files are available ! ")
    return True

def find_cpp_src_files(file_path):
    """
    Finds cpp test src files in given path

    This function ensures only only cpp test file is present in the 
    test folder, to be considered for compilation and test execution.

    Args:
        file_path (str) : File path for test folder

    Returns:
        cpp_files : Filename and path of cpp test file in test folder
    """
    # Search for the .cpp file in the script's directory
    cpp_files = glob.glob(os.path.join(file_path, '*.cpp'))

    # Check if exactly one .cpp file was found
    if len(cpp_files) == 1:
        cpp_file = cpp_files[0]
        debug_log(f"Found test .cpp file: {os.path.basename(cpp_file)}")
        return cpp_file
    elif len(cpp_files) == 0:
        debug_log("No .cpp file found.")
    else:
        debug_log(f"More than one .cpp file found at {file_path}")
    return None

def run_test_prereq(script_dir, include_paths=["../libsubtec"]):
    """
    Runs pre-requisites for each test

    This function runs pre-requisites for each test - check subtec is running,
    compile cpp file, run executable from compilation.

    Args:
        script_dir (str) : File path for test folder
        include_paths (str) : include paths to use to compile test cpp file

    Returns:
        None
    """
    cpp_file = find_cpp_src_files(script_dir)
    assert cpp_file, "Check if test folder has a cpp file !"

    # Ensure subtec app is running
    check_and_restart_subtec()

    # Compile cpp test file
    os.chdir(script_dir)
    executable = compile_cpp(cpp_file, include_paths)

    # Run binary file
    if executable:
        if run_executable(executable, None) is False:
            assert False, "Failed to execute C program"
    else:
        assert False, "Failed to compile C program"

def compile_cpp(file_path, include_paths=["../libsubtec"], library_paths=None, libraries=["subtec"], defines=["SUBTEC_PACKET_DEBUG=1"]):
    """
    Compiles cpp test file

    This function accepts include paths, list of libraries and library
    paths, along with compilation flags required to compile cpp test file.

    Args:
        file_path (str) : File path for test folder
        include_paths (str) : include paths to use to compile test cpp file
        library_paths (str) : libary paths to use to compile test cpp file
                            and link libraries
        libraries (str) : List of libraries to link to run test cpp file
        defines (str) : Compilation flags to use to compile test cpp file.

    Returns:
        binary_name : test binary name, if cpp test file compilation is successful.
    """
    # Check if the cpp file exists
    if not os.path.isfile(file_path):
        raise FileNotFoundError(f"The file {file_path} does not exist.")

    # Create binary name based on test name
    file_name = os.path.basename(file_path)
    file_name_wo_extension = os.path.splitext(file_name)[0]
    binary_name = "_".join(file_name_wo_extension.split('_')[:2]) + "_bin"

    # Remove the existing cpp bin file if it exists
    if os.path.exists(binary_name):
        os.remove(binary_name)

    construct_utils_cpp_path = "../utils/utils.cpp"
    # Base compile command
    compile_command = ["g++", file_path, construct_utils_cpp_path, "-o", binary_name]

    # Add include paths if provided
    if include_paths:
        for path in include_paths:
            compile_command.extend(["-I", path])

    compile_command.extend(["-I", "../subtecparser"])
    compile_command.extend(["-I", "../utils"])

    # Add library paths if provided
    if library_paths is None:
        library_paths = [os.path.dirname(os.path.abspath(__file__))]

    for path in library_paths:
        compile_command.extend(["-L", path])

    # Add libraries if provided
    if libraries:
        for lib in libraries:
            compile_command.extend(["-l", lib])

    # Add defines if provided
    if defines:
        for define in defines:
            compile_command.extend(["-D", define])

    try:
        result = subprocess.run(compile_command, check=True, capture_output=True, text=True)
        if result.returncode:
            debug_log("ERROR cpp test file compilation failed with return code" << result.returncode)
            debug_log("ERROR : " << result.stderr)
        return binary_name
    except subprocess.CalledProcessError as e:
        debug_log("Compilation of test cpp file failed")
        debug_log(e.stderr)
        return None

def run_executable(executable_path, ld_library_path=None):
    """
    Executes the cpp test binary

    This function executes cpp test binary for every test.

    Args:
        executable_path (str) : File path for test folder
        ld_library_path (str) : Value to set to LD_LIBRARY_PATH

    Returns:
        True : if binary was executed successfully.
    """
    if not os.path.isfile(executable_path):
        raise FileNotFoundError(f"The executable {executable_path} does not exist.")

    # Set the environment variable for LD_LIBRARY_PATH
    env = os.environ.copy()

    if ld_library_path:
        env["LD_LIBRARY_PATH"] = ld_library_path
    else:
        library_paths = [os.path.dirname(os.path.abspath(__file__))]
        ld_library_path = ":".join(library_paths)
        env["LD_LIBRARY_PATH"] = ld_library_path

    try:
        process = subprocess.Popen([f"./{executable_path}"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1, env=env,)
        # This is to print unbuffered console output
        for line in process.stdout:
            print(line, end='')

        process.stdout.close()
        process.wait()

        debug_log("CPP test execution is done, validating logs next ! ")

        return True
    except subprocess.CalledProcessError as e:
        debug_log("Execution failed for test case cpp ")
        debug_log(e.stderr)
        debug_log(e.stdout)
        return False
    except Exception as e:
        debug_log(f"Unexpected error for test case cpp : {e}")
        return False

"""
    Example flow of how these functions are invoked when executing two sample tests as below :

    ./run_l2_tests.py -i 1023 1035 1064

    run_l2_tests.py : 
        parse arguments, 
        install_python_dependencies(), 
        get list of tests to be executed (1023, 1035, 1064), 
        build_and_copy_lib(),
        pass options to pytest,
        start tests
        
    TEST_1023_webvtt_Attrib_fontStyle.py :
        utils.py:check_font_files()
        utils.py:get_log_filename() (test_1023_subtec.log)
        utils.py:get_last_read_position() - zero initially
        utils.py:run_test_prereq()
            find_cpp_src_files()
            check_and_restart_subtec()
                is_process_running()
                cleanup_subtec() - find_and_kill_process() if required
                start_subtec() - since this is first test, starts subtec
                wait_for_log_string() - to ensure subtec started
            compile_cpp()
            run_executable() - Here TEST_1023_bin, compiled from TEST_1023_webvtt_Attrib_fontStyle.cpp is executed
            TEST_1023_webvtt_Attrib_fontStyle.cpp - Sends all packets required for test
        utils.py:wait_for_log_string() - Wait for test to be completed
        utils.py:copy_log_file() - Create a copy of common subtec log here
        utils.py:monitor_log_for_sequence() - to verify logs in given sequence
        Prints results as Passed or asserts if some log is missing and marked as Failure
        Pytest control goes to next python test

    TEST_1035_CC_608_attrib_winborderType.py :
        utils.py:get_log_filename() (test_1035_subtec.log)
        utils.py:get_last_read_position() - end of log file after previous test, is considered as starting point for this test
        utils.py:run_test_prereq()
            find_cpp_src_files()
            check_and_restart_subtec()
                is_process_running() - if subtec is running as expected, does not go to cleanup_subtec() or start_subtec()
            compile_cpp()
            run_executable() - Here TEST_1035_bin, compiled from TEST_1035_CC_608_attrib_winborderType.cpp is executed
            TEST_1035_CC_608_attrib_winborderType.cpp - Sends all packets required for test
        utils.py:wait_for_log_string() - Wait for test to be completed
        utils.py:copy_log_file() - Create a copy of common subtec log here
        utils.py:monitor_log_for_sequence() - to verify logs in given sequence
        utils.py:check_logs_between_strings() - check logs between two given log strings
        Prints results as Passed or asserts if some log is missing and marked as Failure

    TEST_1064_ttml_Pause_resume_woSel.py :
        utils.py:get_log_filename() (test_1064_subtec.log)
        utils.py:get_last_read_position() - end of log file after previous test, is considered as starting point for this test
        utils.py:run_test_prereq()
            find_cpp_src_files()
            check_and_restart_subtec()
                is_process_running() - if subtec is running as expected, does not go to cleanup_subtec() or start_subtec()
            compile_cpp()
            run_executable() - Here TEST_1064_bin, compiled from TEST_1064_ttml_Pause_resume_woSel.cpp is executed
            TEST_1064_ttml_Pause_resume_woSel.cpp - Sends all packets required for test
        utils.py:wait_for_log_string() - Wait for test to be completed
        utils.py:copy_log_file() - Create a copy of common subtec log here
        utils.py:monitor_log_for_sequence() - to verify logs in given sequence
        utils.py:validate_string_absence() - ensures that strings not expected, are not present in logs.
        Prints results as Passed or asserts if some log is missing and marked as Failure

    run_l2_tests.py:parse_junit_xml()
    run_l2_tests.py:print_table()
    utils.py:cleanup_subtec()
"""
