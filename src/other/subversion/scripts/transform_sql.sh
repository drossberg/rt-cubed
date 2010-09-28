#!/bin/sh

varname=`basename $1 | tr "[a-z]" "[A-Z]" | tr "-" "_" | tr "." "_"`

echo "/* This file is automatically generated from"
echo " * $1"
echo " * Do not edit it directly, but edit the source file and rerun 'make'"
echo " */"
echo
echo "#define $varname \\"

comment=false

IFS="
"
while read line; do
  line=`echo "$line" | sed -e 's/\/\*.*\*\///g' | sed -e 's/[ ]*$//g'`

  if [ -z "$line" ]; then
    continue
  fi

  if [ "$comment" = "false" ] && echo "$line" | fgrep '/*' >/dev/null; then
    comment=true
  fi

  if [ "$comment" = "true" ] && echo "$line" | fgrep '*/' >/dev/null; then
    comment=false
    continue
  fi

  if [ "$comment" = "true" ]; then
    continue
  fi

  line="`echo "$line" | sed -e 's/"/\\"/g'`"  #' hi, emacs syntax coloring!
  if [ -n "$line" ]; then
    echo "  \"$line \"\\"
  fi
done

echo '  ""'