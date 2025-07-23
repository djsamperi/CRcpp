# Patch include/Rcpp/Language.h
# Path to CRcpp directory should be specified.
if [ "$1" = "" ]; then
    echo "Usage: sh languagepatch.sh <CRcppPath>"
    exit 1
fi
cd $1
cp patch/language/Language.h Rcpp/inst/include/Rcpp/Language.h

