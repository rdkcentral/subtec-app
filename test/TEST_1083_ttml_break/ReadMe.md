# Subtec ttml data L2 test

This python3 L2 test verifies ttml data packet functionality and checks
displayed text content/order for six scenarios around `<br/>` handling.

Covered scenarios:
1. Direct text before and after `<br/>`
2. Pretty-printed TTML with indentation around `<br/>`
3. `<span>` before `<br/>`, plain text after it
4. Plain text before `<br/>`, `<span>` after it
5. Text around two `<br/>` transitions with a visible middle segment
6. Trailing `<br/>` with no text after it

The info-level `displaying:` log collapses all whitespace, including line
boundaries, so this L2 test cannot verify separate or blank lines.
`ParserTest::testParseMultipleBreaks` and
`ParserTest::testParseConsecutiveBreaks` in
`subttxrend-ttml/test/Parser_test.cpp` assert the parsed line structure.

## Pre-requisites to L2 tests:

Subtec dependencies installed with *install_subtec_dependencies.sh* script
and subtec running.

sample.ttml - input TTML file containing the six `<br/>` regression scenarios.

## Run test using script:

From the *subtec-app/test/* folder run:

./run_l2_tests.py -i 1083
