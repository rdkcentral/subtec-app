# Subtec ttml data L2 test

This python3 L2 test verifies ttml data packet functionality and validates
six end-to-end TTML line-break scenarios around `<br/>` handling.

Covered scenarios:
1. Direct text before and after `<br/>`
2. Pretty-printed TTML with indentation around `<br/>`
3. `<span>` before `<br/>`, plain text after it
4. Plain text before `<br/>`, `<span>` after it
5. Two successive `<br/>` transitions producing three visible lines
6. Trailing `<br/>` with no text after it

Case 5 intentionally uses a visible middle line because the info-level
`displaying:` log does not encode a truly blank line distinctly enough to
assert it in this L2 harness.

## Pre-requisites to L2 tests:

Subtec dependencies installed with *install_subtec_dependencies.sh* script
and subtec running.

sample.ttml - input TTML file containing the six `<br/>` regression scenarios.

## Run test using script:

From the *subtec-app/test/* folder run:

./run_l2_tests.py -i 1083
