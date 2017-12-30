#!/bin/sh

testcases=$(find p2pchat/tests -type f -name "test_*" -executable)
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
