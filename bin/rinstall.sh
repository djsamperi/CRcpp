#!/bin/sh
# Use R to install Rcpp/RInside/Mypack into R_LIBS
# Target directory in R_LIBS must exist.
# Remove object files and dll's from source dir after install.
# CMake will build binaries under CRcpp/build, and copy libs to R_LIBS
# where R can find them.

R CMD INSTALL Rcpp
(cd Rcpp;  Rscript -e 'devtools::clean_dll()')

R CMD INSTALL RInside
(cd RInside; Rscript -e 'devtools::clean_dll()')

cd Mypack
Rscript -e 'Rcpp::compileAttributes()'
Rscript -e 'devtools::document()'
# Rscript -e 'devtools::install(build_vignettes = TRUE)'
R CMD INSTALL .
Rscript -e 'devtools::clean_dll()'
