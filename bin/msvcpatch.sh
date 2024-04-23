# Patch Rcpp and RInside source files for Microsoft Visual C++ (Visual Studio)
# Note: Patches are for specific versions of Rcpp and RInside determined
#       by github hashes (see references in CRcpp on GitHub).
# Path to CRcpp directory should be specified.
if [ "$1" = "" ]; then
    echo "Usage: sh msvcpatch.sh <CRcppPath>"
    exit 1
fi
cd $1
cp etc/msvc/compiler.h Rcpp/inst/include/Rcpp/platform/
cp etc/msvc/date.cpp   Rcpp/src/
cp etc/msvc/Rcpp.h     Rcpp/inst/include/
cp etc/msvc/RInsideCommon.h RInside/inst/include/
cp etc/msvc/RInside.cpp     RInside/src/

