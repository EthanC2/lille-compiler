#!/bin/bash
set -eu

# 0. Import test library
. ../testlib.sh

# 2. Initialize local variables
categories=($(ls tests))
nfailed=0
npassed=0

if [[ ! -x "$MY_COMPILER" ]]; then
    >&2 echo "you forgot to compile your compiler, chief"
    exit 1
fi

for category in ${categories[@]}; do
    tests=($(ls tests/$category))
    echo "category: $category"

    for test in ${tests[@]}; do
	if [[ ! -e "expected/$category" ]]; then
	    >&2 "error: category directory \"$category\" is missing its companion directory in \"./expected\""
	    exit 1
	fi

	if [[ ! -e "expected/$category/$test" ]]; then
	    >&2 "error: test case \"$test\" is missing its companion answer file in \"./expected\""
	    exit 2
	fi

	actual=$($MY_COMPILER tests/$category/$test 2>&1 | head -n -1)
	expected=$(< expected/$category/$test)

	echo -n "[test: $category/$test]: "
	if [[ $actual == $expected ]]; then
	    echo -e ${GREEN}PASSED${WHITE}
	    npassed=$(($npassed + 1))
	else
	    echo -e ${RED}FAILED${WHITE}

	    echo -e "\n${CYAN}EXPECTED:${WHITE}"
	    [[ -n "$expected" ]] && echo -e "$expected\n" || echo -e "<NOTHING>\n"

	    echo -e "${CYAN}ACTUAL:${WHITE}"
	    [[ -n "$actual" ]] && echo -e "$actual\n" || echo -e "<NOTHING>\n"

	    nfailed=$(($nfailed + 1))
	fi
    done
done

echo -e "\n${GREEN}$npassed${WHITE} TESTS PASSED"
echo -e "${RED}$nfailed${WHITE} TESTS FAILED"
