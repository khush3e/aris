#!/bin/bash

myinfo() {
 echo "This script tests all *.tle files in this folder."
 echo "Usage: $0 <aris-executable>"
 }

ARIS="$1"

if [ "$ARIS" = "" ]; then
 myinfo
 echo "No executable is given."
 exit 1
 fi

test -x "$ARIS" || {
 myinfo
 echo "The file $ARIS is not executable."
 exit 2
}

"$ARIS" -h | grep --silent "GNU Aris" || {
 myinfo
 echo "The executable $ARIS does not look like an instance of GNU Aris."
 exit 3
}

N=0
GOOD=0
for TLE in `find . -name '*.tle' -type f`; do
 "$ARIS" -f "$TLE" -e 2>/dev/null && GOOD=$((GOOD+1)) || {
  echo "$TLE has regression:"
  "$ARIS" -f "$TLE" -e -v
  echo
  }
 N=$((N+1))
 done
echo "$GOOD good (out of $N cases)"

[ "$GOOD" -eq "$N" ] || exit 1
