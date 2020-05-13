#!/usr/bin/python3
import sys

with open(sys.argv[2], "w") as outfile:
  for line in open(sys.argv[1], "r"):
    outfile.write(line)
    line = line.strip()
    if line == "#define ZCONF_H" or line == "#define ZCONFNG_H":
      outfile.write("#cmakedefine Z_HAVE_UNISTD_H\n")
      outfile.write("#cmakedefine NEED_PTRDIFF_T\n")
      outfile.write("#if defined(NEED_PTRDIFF_T)\n")
      outfile.write("#cmakedefine PTRDIFF_TYPE\n")
      outfile.write("#endif\n")
