#!/usr/bin/python
# vim: set fileencoding=utf-8 :

import sys,os,codecs,random,time

# --- main ---
# this is the way to fix python's handling of utf-8 on stdout
sys.stdout = codecs.getwriter('utf8')(sys.stdout)

DICT=sys.argv[1]
IN=sys.argv[2]


f = codecs.open(DICT,'r', 'utf-8')
tmpWL = f.readlines()
f.close()
D={}

for line in tmpWL:
    txt, phon = line.rstrip().split(" ")
    D[txt] = phon


f = codecs.open(IN,'r', 'utf-8')
tmpWL = f.readlines()
f.close()

for line in tmpWL:
    failed = 0
    out = ""
    for w in line.rstrip().split(" "):
        if w in D:
            out = out + D[w] + ' '
        else:
            failed = 1
            break
    if failed == 0:
        print line.rstrip() + "\t" + out.rstrip()
