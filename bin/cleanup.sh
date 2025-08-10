#!/bin/sh
# Removes helper files that are generated in the Rcpp and RIside
# directory trees.
# Path to CRcpp directory should be specified.
if [ "$1" = "" ]; then
    echo "Usage: sh cleanup.sh <CRcppPath>"
    exit 1
fi
cd $1
rm -f Rcpp/CMakeLists.txt Rcpp/src/CRcppBuildRcpp.cpp
rm -f RInside/CMakeLists.txt RInside/src/CRcppBuildRInside.cpp
rm -f RInside/src/RInsideAutoloads.h RInside/src/RInsideEnvVars.h
