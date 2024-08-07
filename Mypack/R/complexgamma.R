#' @title R interface to omplex gamma function of a vector or matrix argument.
#' @param z A vector or matrix (numeric or complex)
#' @return
#'  Returns a vector or matrix of complex values of the gamma function
#'  corresponding to the input numeric or complex values.
#' @details
#'  Computes the complex gamma function using the Lanczos approximation
#'  (see Wikipedia).
#' @examples
#' m <- matrix(1:12,3,4)
#' cgamma(m)
#'
#' @export
cgamma <- function(z) {
  cgammacpp(z)
}

#' @title Shows 3D plot of Complex Gamma Function
#' @details
#' When used with CRcpp framework be sure to use x11() to
#' bring up a graphics window before invoking this function.
#' @examples
#' showgamma()
#'
#' @export
showgamma <- function() {
    
  complexify <- function(x,y) {
    complex(real=x, imaginary=y)
  }
  Nreal <- 50
  Nimag <- 100
  rl <- seq(-4,4,length.out=Nreal)
  im <- seq(-2,2,length.out=Nimag)
  z <- outer(rl, im, complexify)
  gamma <- cgamma(z)

  ## persp axis labels do not recognize expression()
  persp(rl, im, abs(gamma),ticktype='detailed',theta=-20,
        main='Modulus of Complex Gamma Function',col='cyan',
        xlab="Re(z)",ylab="Im(z)",zlab="Gamma(z)")
}
