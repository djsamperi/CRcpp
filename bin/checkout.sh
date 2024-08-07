# Check out msvc branch for submodules Rcpp and RInside.
# The default behavior is to leave HEAD detached, because
# the submodule points to a commit, not to a reference.

# Path to CRcpp directory should be specified.
if [ "$1" = "" ]; then
    echo "Usage: sh copycmakefiles.sh <CRcppPath>"
    exit 1
fi
cd $1
cd Rcpp
git checkout msvc
cd ../RInside
git checkout msvc
