#!/usr/bin/python3
# Usage: run-and-redirect.py input output cmd ...

import subprocess
import sys

input = sys.argv[1]
output = sys.argv[2]
cmd = sys.argv[3]

finput = open(input, "rb")
if output == "/dev/null":
  foutput = subprocess.DEVNULL
else:
  foutput = open(output, "wb")

args = [cmd]
if len(sys.argv) > 4:
  args += sys.argv[4:]

result = subprocess.run(args, stdin=finput, stdout=foutput)
