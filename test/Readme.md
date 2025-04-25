Readme.md

# How to run subtec on ubuntu

To install required packages for subtec to build successsfully on ubuntu, run below script from  ***subtec-app/test/*** folder for first time.

    bash install_subtec_dependencies.sh

To manually build and run subtec from ***subtec-app/subttxrend-app/x86_builder/*** folder using below command.

    ./build.sh run

**Python 3 installation is required for the subtec L2 framework** 

# Subtec L2 test framework set up

To install required packages for tests, run the below scripts from ***subtec-app/test/*** folder for first time

    ./l2framework_testenv.sh

Activate python virtual environment "l2venv" that will have been created.

    source l2venv/bin/activate

# What run_l2_tests.py does
1. Installs required python modules when run for first time
2. Builds and starts subtec app
3. Selects list of tests to run from command line options
4. Runs pytest on that list of tests
5. Generates test results in a table format from results.xml produced by pytest

# How to run L2 test suites using run_l2_tests.py

**To run all test suites, use the following command from test directory**

    ./run_l2_tests.py

**To build and run specified test suites, use the following command**

    ./run_l2_tests.py -i 1001 1002

**To run all test suites but exclude those specified, use the following command**

    ./run_l2_tests.py -e 1001 1002

**To run tests marked as attributes, use the following command**

    ./run_l2_tests.py -m "attributes"

**To run tests marked as webvtt and attributes, use the following command**

    ./run_l2_tests.py -m "webvtt and attributes"

    Based on the pytest marks listed in pytest.ini, several combinations of
    tests can be run with "-m" option using keywords "and, or, not".

**To get help about script, use the following command**

    ./run_l2_tests.py -h
    
## Test output log files

Test logs will be created in test folder for every run under ***run_l2_subtec_yyyymmdd_hhmmsslog***

Subtec app run log will be created in test folder as ***subtec_console_common.log***

Each test will output log files to a subdirectory 'output_files' E.G

    .../test/TEST_1001_webvtt_SelectionPacket/output_files

## Invoking pytest directly
Once required python modules have been installed (E.G via an invocation of ./run_l2_tests) 
then pytest can be used to run individual tests, or all tests. No .xml or .json results file will be produced when invoked this way.

    pytest TEST_1001_webvtt_SelectionPacket

# To run with a python virtual environment

Create the venv, activate the env, run the tests. 

    python3 -m venv l2venv
    source l2venv/bin/activate
    ./run_l2_tests.py ...

# How to add new test suite
  
- Create new test suite folder under ***subtec-app/test/***

- The name of the test suite folder should be in format of ***TEST_XXXX_testname*** - with string ***TEST_*** followed by a unique 4+ digit number and test name. ***e.g. TEST_1001_webvtt_SelectionPacket*** 

- Copy in needed files from some other test as a starting point.

- Note that TEST_xxxx.cpp and TEST_xxxx.py will have to be named so that the file name matches that of the containing directory.

- Place input files required for test under ***input_files*** sub-directory in test suite folder.

- TEST_xxxx.cpp - Include required header files, add sequence of functions to be called for the test case.

- TEST_xxxx.py - Pre-requisites to compile cpp file, execute it, ensure subtec is running are done in run_test_prereq() in utils.py. Use wait_for_log_string() to wait for test case execution to be complete. List out expected log messages in the right sequence in log_sequence and call monitor_log_for_sequence() to validate if the expected logs are available in subtec log in the expected sequence.

- If the new test uses a new python package then it will need to be added into the requirements.txt 

- Note that subtec process is not restarted between tests, so make sure that the previous test attributes are cleared for the new test to work as expected.

- A copy of source files to run subtec app is placed under libsubtec/ and subtecparser/ folders of test/ folder. If there are significant code changes in those files, this copy of those files also needs to be updated.