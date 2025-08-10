#!/bin/sh
# Patch Rcpp and RInside source files for use with Microsoft compiler.
# Also includes a general patch for RInside.cpp. Microsoft changes
# are indicated by _MSC_VER define.
# Path to CRcpp directory should be specified.
if [ "$1" = "" ]; then
    echo "Usage: sh applypatch.sh <CRcppPath>"
    exit 1
fi
cd $1
cp patch/compiler.h Rcpp/inst/include/Rcpp/platform/
cp patch/date.cpp   Rcpp/src/
cp patch/Rcpp.h     Rcpp/inst/include/
cp patch/RInsideCommon.h RInside/inst/include/
cp patch/RInside.cpp     RInside/src/

