#!/bin/bash

testcases=$(find p2pchat/tests -type f -name "test_*" -executable)
if test -z $testcase;then
    echo "No test found. Please run \`make test' first"
    exit 1
fi

echo "Collected $(wc -l <<< $testcases) tests"
passed=0
failed=0
for testcase in $testcases ;do
    $testcase
    if [ $? -eq 0 ];then
        ret='[PASS]'
        passed=$(($passed + 1))
    else
        ret='[FAIL]'
        failed=$(($failed + 1))
    fi
    echo "$ret $testcase"
done
echo "Done. $passed pass, $failed failed."
