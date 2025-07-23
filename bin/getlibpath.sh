# Get full path of the install location for a specified package,
# for example, sh getlibpath.sh Rcpp

if [ "$1" = "" ]; then
    echo "Usage: sh $0 <pkgname>"
    exit 1
fi
pkg=$1

Rscript --vanilla -e '

## Gets location of package given name.
getLibPath <- function(pkgName) {
    require(pkgName, character.only = TRUE, quietly=TRUE, warn.conflicts=FALSE)
    pkgEnv <- as.environment(paste0("package:",pkgName))
    attr <- attributes(pkgEnv)
    attr$path
}

pkgName = commandArgs(trailingOnly=TRUE)

result <- tryCatch( { 
  getLibPath(pkgName)
  }, error = function(err) { 
     "Package not found"
  } )

cat(result,"\n")
' $pkg

