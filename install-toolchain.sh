#!/bin/bash
#
# Builds git@github.com:andwn/marsdev for straightforward installation of
# GCC 12.x and co. This script will clone marsdev, build it, install it into
# /opt/ and then clean up after itself.
#
# It has been tested on Debian 5.18, and ought to work on WSL.

# The marsdev build and installation target places its output in $MARSDEV.
# I elected to create a temporary directory for this so that root permissions
# are only needed during the final step.
export MARSDEV=/tmp/_marsdev_tmp_

# The destination for the toolchain (MD_BASE).
export MD_BASE=/opt/mdk-toolchain

# Clone marsdev.
mkdir -p $MARSDEV
cd $MARSDEV
# TODO: Move or delete _marsdev_git_ that already exists
git clone git@github.com:andwn/marsdev

# Build gcc, newlib, and co.
cd marsdev
make m68k-toolchain-newlib LANGS=c,c++

# Install it into MD_BASE.
# TODO: Move or delete mdk-toolchain that already exists
sudo mkdir -p $MD_BASE
sudo mv $MARSDEV/m68k-elf $MD_BASE

# Clean up.
cd /
rm -rf $MARSDEV
