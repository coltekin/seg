#!/bin/sh
#
# This script filters given CHILDES files remove punctuation,
# disfluencies, and some other extralinguistic annotation in
# the CHILDES transcripts.
#
# cat $(grep  -l -r '^@ID.*CHI.*|0\;' .)  

grep  '^\*' |grep -v '^.CHI'\
    | cut -f2- \
    | sed  's/[0-9_]*//g' \
    | sed  's/\[[^]]*\]//g' \
    | sed 's/[.,!?:]//g'\
    | sed -r 's/\S+@[kcubnlfopt]\>//g'\
    | sed -r 's/\S+_\S+//g'\
    | sed -r 's/[+]//g'\
    | sed -r 's,xxx*,,g'\
    | sed -r 's,//,,g'\
    | sed 's,([^)]*),,g'\
    | sed -r 's,[&]\S+,,g'\
    | sed -r 's,[<>],,g'\
    | sed -r 's,\<0,,g'\
    | sed -r 's,["/„^],,g'\
    | sed -r 's/\S+@wp\>//g'\
    | sed -r 's/\S+@si\>//g'\
    | sed -r 's/\S+@snle\>//g'\
    | sed -r 's/\S+-\S+//g'\
    | sed -r 's/\s+/ /g' \
    | sed -r 's/^ //g' \
    | sed -r 's/\S+/\l&/g' \
    | sed -r '/^\s*$/d'
