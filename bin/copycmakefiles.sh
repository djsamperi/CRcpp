# Add CMakeLists.txt files for Rcpp and RInside, as well
# as Rcpp/src/CRcppBuildRcpp.cpp and RInside/src/CRcppBuildRInside.cpp.
# Path to CRcpp directory should be specified.
if [ "$1" = "" ]; then
    echo "Usage: sh copycmakefiles.sh <CRcppPath>"
    exit 1
fi
cd $1
if ! test -f Rcpp/CMakeLists.txt; then
    cp etc/cmake/Rcpp/CMakeLists.txt Rcpp/CMakeLists.txt
fi
if ! test -f RInside/CMakeLists.txt; then
    cp etc/cmake/RInside/CMakeLists.txt RInside/CMakeLists.txt
fi

if ! test -f Rcpp/src/CRcppBuildRcpp.cpp; then
    cp etc/cmake/Rcpp/CRcppBuildRcpp.cpp Rcpp/src
fi
if ! test -f RInside/src/CRcppBuildRInside.cpp; then
    cp etc/cmake/RInside/CRcppBuildRInside.cpp RInside/src
fi
