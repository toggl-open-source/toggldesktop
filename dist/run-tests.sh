#!/bin/bash

### Options:
# !!!NO SPACES ANYWHERE!!! because I'm laz^H^H^H^H^H^H want to keep it simple
# name - test name, to be displayed on slack
# test - path to the test binary (supports GTest binaries only)
# slack_channel - Slack channel to report to
# slack_url - Slack hook URL
### Example:
# ./run-tests.sh name="App test" \
#                test=./src/test/TogglAppTest \
#                slack_channel="#desktop-bots" \
#                slack_url=https://hooks.slack.com/services/ABCDEFGHI/JKLMNOPQR/STUVWXYZ1234567890abcdef

set -e

for line in $@; do
  eval "$line"
done

function rdom () {
    local IFS=\> ;
    read -d \< E C
}

# parse the xml with pure bash, basically just remember the last testsuite and testcase and print them out if they contain a failure
function parse() {
    last_testsuite=""
    last_testcase=""
    while rdom; do
        if [[ $E == "testsuite"* ]]; then
            last_testsuite=$(sed 's/.* name="\([^"]*\)".*/\1/' <<< $E)
        fi
        if [[ $E == "testcase"* ]]; then
            last_testcase=$(sed 's/.* name="\([^"]*\)".*/\1/' <<< $E)
        fi
        if [[ $E == "failure"* ]]; then
            # -n is intentional so curl doesn't eat the new lines
            echo -n "${last_testsuite}.${last_testcase}\\n"
        fi
    done < $1
}

# construct a slack payload
function payload() {
    github_sha_short=$(cut -c-8 <<< "$GITHUB_SHA")
    # create a temporary file to store the parse result somewhere
    failed_tests=$(mktemp)
    parse $1 > "$failed_tests"
    failed_count=$(echo -ne $(cat "$failed_tests") | wc -l)
    # first print the json preamble setting the channel and enabling markdown
    echo -n 'payload={"channel": "'$slack_channel'", "mrkdwn": true, "text": "'
    # then insert the actual message to be displayed
    echo -n "$failed_count tests failed in the $name test suite on platform \`$OSTYPE\` in commit \`<https://github.com/$GITHUB_REPOSITORY/commit/$GITHUB_SHA|$github_sha_short>\`:"
    # preformatted section start
    echo -n '```'
    # list the failed tests
    cat "$failed_tests"
    # preformatted section end
    echo -n '```'
    # json end
    echo -n '"}'
    rm "$failed_tests"
}

function report() {
    # pipe the output of the payload function directly to curl
    payload $1 | curl -X POST -d @- $slack_url
}

function execute() {
    if ! $test --gtest_output=xml:${name}_result.xml; then
        if [[ "$GITHUB_REF" == "refs/heads/master" ]]; then
            report ${name}_result.xml
        else
            echo "Test results were not reported because we're not in master"
            echo "Current ref is: $GITHUB_REF"
        fi
    fi
    rm ${name}_result.xml
}

execute
