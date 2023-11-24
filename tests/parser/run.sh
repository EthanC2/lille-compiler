#!/bin/bash
set -eu pipefail

# 0. Import test library
. ../testlib.sh

# 2. Initialize local variables
rm tests/*.pal || true
tests=($(ls tests))
nfailed=0
npassed=0

if [[ ! -x "$MY_COMPILER" ]]; then
    >&2 echo "you forgot to compile your compiler, chief"
    exit 1
fi

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
        npassed=$(($npassed + 1))
    else
        echo -e ${RED}FAILED${WHITE}

        echo -e "\n${CYAN}EXPECTED:${WHITE}"
        if [[ -z "$expected" ]]; then
            echo -e "<NOTHING>\n"
        else
            echo -e "$expected\n"
        fi

        echo -e "${CYAN}ACTUAL:${WHITE}"
        if [[ -z "$actual" ]]; then
            echo -e "<NOTHING>\n"
        else
            echo -e "$actual\n"
        fi

        nfailed=$(($nfailed + 1))
    fi
done

echo -e "\n${GREEN}$npassed${WHITE} TESTS PASSED"
echo -e "${RED}$nfailed${WHITE} TESTS FAILED"
