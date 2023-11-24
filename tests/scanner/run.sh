#!/bin/bash

# 1. Load shared content (variables, functions)
. ../testlib.sh

# 2. Initialize local variables
tests=($(ls tests))

for test in ${tests[@]}; do
    if [[ ! -e "expected/$test" ]]; then
        >&2 "error: test case \"$test\" is missing its companion answer file in \"expected\""
        exit 1
    fi

    actual=$($MY_COMPILER tests/$test 2>&1 | head -n -1)
    expected=$(< expected/$test)

    echo -n "[test: $test]: "
    if [[ $actual == $expected ]]; then
        echo -e ${GREEN}PASSED${WHITE}
    else
        echo -e ${RED}FAILED${WHITE}

        echo -e "\n${CYAN}EXPECTED:${WHITE}"
        echo -e "$expected\n"

        echo -e "${CYAN}ACTUAL:${WHITE}"
        echo -e "$actual\n"

    fi
done
