#!/bin/sh
# Verify that the various build systems produce identical results on a Unixlike system
set -ex

# If suffix not set to "", default to -ng
suffix=${suffix--ng}
# Caller can also set CMAKE_ARGS if desired
CMAKE_ARGS=${CMAKE_ARGS}

# Tell GNU's ld etc. to use Jan 1 1970 when embedding timestamps
# Probably only needed on older systems (ubuntu 14.04?)
#export SOURCE_DATE_EPOCH=0
# Tell Apple's ar etc. to use zero timestamps
export ZERO_AR_DATE=1

# Use same compiler for make and cmake builds
if test "$CC"x = ""x
then
  if clang --version
  then
    export CC=clang
  elif gcc --version
  then
    export CC=gcc
  fi
fi

# New build system
# Happens to delete top-level zconf.h
# (which itself is a bug, https://github.com/madler/zlib/issues/162 )
# which triggers another bug later in configure,
# https://github.com/madler/zlib/issues/499
rm -rf btmp2 pkgtmp2
mkdir btmp2 pkgtmp2
export DESTDIR=$(pwd)/pkgtmp2
cd btmp2
  cmake -G Ninja ${CMAKE_ARGS} ..
  ninja -v
  ninja install
cd ..

# Original build system
rm -rf btmp1 pkgtmp1
mkdir btmp1 pkgtmp1
export DESTDIR=$(pwd)/pkgtmp1
cd btmp1
  case $(uname) in
  Darwin)
    export LDFLAGS="-Wl,-headerpad_max_install_names"
    ;;
  Linux)
    if grep -i fedora /etc/os-release > /dev/null
    then
        # Note: Fedora patches cmake to use -O2 in release, which
        # does not match the -O3 configure sets :-(
        export CFLAGS="-O2 -DNDEBUG"
    fi
    ;;
  esac
  ../configure
  make
  make install
cd ..

repack_ar() {
  if ! cmp --silent pkgtmp1/usr/local/lib/libz$suffix.a pkgtmp2/usr/local/lib/libz$suffix.a
  then
    echo "Warning: libz$suffix.a does not match.  Assuming ar needs -D option.  Unpacking..."
    cd pkgtmp1; ar x usr/local/lib/libz$suffix.a; rm usr/local/lib/libz$suffix.a; cd ..
    cd pkgtmp2; ar x usr/local/lib/libz$suffix.a; rm usr/local/lib/libz$suffix.a; for a in *.c.o; do b=$(echo $a | sed 's/\..*//'); mv $a $b.o; done; cd ..
  fi
}

case $(uname) in
Darwin)
  # Alas, dylibs still have an embedded hash or something,
  # so nuke it.
  # FIXME: find a less fragile way to deal with this.
  dylib1=$(find pkgtmp1 -name '*.dylib*' -type f)
  dylib2=$(find pkgtmp2 -name '*.dylib*' -type f)
  dd conv=notrunc if=/dev/zero of=$dylib1 skip=1337 count=16
  dd conv=notrunc if=/dev/zero of=$dylib2 skip=1337 count=16
  ;;
FreeBSD|Linux)
  # The ar on newer systems defaults to -D (i.e. deterministic),
  # but FreeBSD 12.1, Debian 8, and Ubuntu 14.04 seem to not do that.
  # I had trouble passing -D safely to the ar inside CMakeLists.txt,
  # so punt and unpack the archive if needed before comparing.
  repack_ar
  ;;
esac

if diff -Nur pkgtmp1 pkgtmp2
then
  echo pkgcheck-cmake-bits-identical PASS
else
  echo pkgcheck-cmake-bits-identical FAIL
  diffoscope pkgtmp?/usr/local/lib/libz-ng.so.1.*.*
  exit 1
fi
