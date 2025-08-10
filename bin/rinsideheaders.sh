#!/bin/sh
# RInside requires some environment configuration and automatic
# loading of selected R objects. This is done with the help of
# system-dependent header files that are generated here and
# inserted into RInside/src. This script should be run with
# the working directory set to the path to the latter.
if [ "$1" = "" ]; then
  echo "Usage: rinsideheaders.sh <RInsideSrcDirPath>"
  exit 1
fi
cd $1
Rscript tools/RInsideAutoloads.r > RInsideAutoloads.h
Rscript tools/RInsideEnvVars.r > RInsideEnvVars.h
echo "-- Generated RInside headers"
