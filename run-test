#!/bin/bash

if [ $# -eq 0 ]; then
  echo "Usage: $0 <testcase_group_name>"
  echo "Available testcase groups from config.json:"
  jq -r '.Groups[].GroupName' testcases/config.json | sed 's/^/  /'
  exit 1
fi

if [ ! -e testcases/config.json ]; then
  echo "./testcases/config.json does not exist, please extract testcases to that directory."
  exit 1
fi

if [ ! -e "$(which jq)" ]; then
  echo 'You need the program "jq" to use this script.'
  echo 'Run "sudo apt install jq" to install it.'
  exit 1
fi

if ! jq -e ".Groups[] | select(.GroupName == \"$1\")" testcases/config.json > /dev/null; then
  echo "Testcase group '$1' not found in config.json"
  echo "Available testcase groups:"
  jq -r '.Groups[].GroupName' testcases/config.json | sed 's/^/  /'
  exit 1
fi

list=$(jq ".Groups[] | select(.GroupName == \"$1\") | .TestPoints[]" testcases/config.json)

exename="code"
if [ ! -e "$exename" ]; then
  echo "Please compile the ticket system and place the executable at \"$exename\""
  exit 1
fi

function tmp () {
  if [ Darwin = "$(uname -s)" ]; then
    mktemp /tmp/ticket.XXXXXXXXXX
  else
    mktemp -p /tmp ticket.XXXXXXXXXX
  fi
}

function tmpdir () {
  if [ Darwin = "$(uname -s)" ]; then
    mktemp -d /tmp/ticket.XXXXXXXXXX
  else
    mktemp -d -p /tmp ticket.XXXXXXXXXX
  fi
}

testdir="$(tmpdir)"
cp "$exename" "$testdir/"
exe="$testdir/$exename"
cwd="$(pwd)"
basedir="$cwd/testcases"

cd "$testdir"

for i in $list; do
  outfile="$(tmp)"
  echo "About to run input #$i..."
  time "$exe" < "$basedir/$i.in" > "$outfile"
  difffile="$(tmp)"
  if diff -ZB "$outfile" "$basedir/$i.out" > "$difffile"; then true; else
    cat "$difffile" | head -5
    backup="test-$1-$i-$(date '+%s').log"
    cp "$outfile" "$cwd/$backup"
    echo "Test failed on input #$i."
    echo "Output saved to $backup"
    exit 1
  fi
  rm "$outfile" "$difffile"
done

rm -r "$testdir"
echo "Testcase complete, answer correct."