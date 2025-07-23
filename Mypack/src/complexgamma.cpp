/**
 * Computes the complex gamma function using Lanczos approximation.
 */

#include <Rcpp.h>
using namespace Rcpp;

// More p's can be included for increased accuracy.
static double pi = 3.14159265358979323846264338327950288;
static double p[] = {0.99999999999980993, 676.5203681218851, 
		     -1259.1392167224028, 771.32342877765313, 
		     -176.61502916214059, 12.507343278686905,
		     -0.13857109526572012, 9.9843695780195716e-6, 
		     1.5056327351493116e-7 };

// Complex gamma function using Lanczos approximation (see Wikipedia).
// Uses the reflection formula: Gamma(z) Gamma(1-z) = pi/sin(pi*z) when
// Re(z) < 0.5. Internal function of a single complex argument.
std::complex<double> cgamma(std::complex<double> z) {

    int g = 7;

    // Reflection formula: Gamma(1-z) Gamma(z) = pi/sin(pi z).
    if(z.real() < 0.5)
	return pi/(sin(pi*z)*cgamma(1.0-z));

    z -= 1;
    std::complex<double> x = p[0];
    for(int i = 1; i < g+2; ++i) {
	x += p[i]/(z+(double)i);
    }
    std::complex<double> t = z + 0.5 + (double)g;
    return sqrt(2*pi)*pow(t,z+0.5)*exp(-t)*x;
}

//' @title R interface to complex gamma function.
//' @param inRvec complex vector or 2d matrix of complex numbers
//' @details The `Rcomplex` data structure definition has changed
//' recently in `R_ext/Complex.h`.
//' @return Returns a vector or matrix of complex values.
// [[Rcpp::export()]]
SEXP cgammacpp(SEXP inRvec) {

    Rcpp::ComplexVector in_cv, out_cv;
    Rcpp::ComplexMatrix in_cm, out_cm;
    Rcomplex *inRptr=0, *outRptr=0;

    // Setup pointers to data as a linear vector (even when matrix).
    int len=0;
    if(Rf_isMatrix(inRvec)) {
	in_cm = Rcpp::ComplexMatrix(inRvec);
	out_cm = Rcpp::ComplexMatrix(in_cm.nrow(),in_cm.ncol());
	len = in_cm.nrow()*in_cm.ncol();
	inRptr = COMPLEX(in_cm);   // ptr to Rcomplex
	outRptr = COMPLEX(out_cm); // ptr to Rcomplex
    }
    else {
	in_cv = Rcpp::ComplexVector(inRvec);
	out_cv = Rcpp::ComplexVector(in_cv.size());
	len = in_cv.size();
	inRptr = COMPLEX(in_cv);   // ptr to Rcomplex
	outRptr = COMPLEX(out_cv); // ptr to Rcomplex
    }

    // Get C++ pointers.
    std::complex<double>* inCptr = reinterpret_cast<std::complex<double>*>(inRptr);
    std::complex<double>* outCptr = reinterpret_cast<std::complex<double>*>(outRptr);

    // Do the computations and return the output ComplexVector.
    std::transform(&inCptr[0], &inCptr[len], &outCptr[0], cgamma);

    return Rf_isMatrix(inRvec) ? out_cm : out_cv;
}

// Optional function used to return compiler toolchain info...

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#ifdef _MSC_VER
// _MSC_VER is MMNN (M=Major,N=Minor)
// _MSC_FULL_VER is MMNNBBBBB (M=Major,N=Minor,B=Build)
const char* compiler_info = "Compiled with MSVC " TOSTRING(_MSC_FULL_VER);
#elif defined(__clang__) && defined(__APPLE__) && defined(__MACH__)
const char* compiler_info = "Compiled with Clang " __VERSION__;
#elif defined(__GNUC__)
const char* compiler_info = "Compiled with GNU g++ " __VERSION__;
#else
const char* compiler_info = "Unknown Compiler";
#endif
    
//' @title Return C++ toolchain used to build package \code{Mypack}
//' @details
//' The Windows toolchain (MSVC) shows version info MMNNBBBBB, where
//' MM=Major, NN=Minor, and BBBBB=Build.
// [[Rcpp::export()]]
Rcpp::CharacterVector cpptoolchain() {
    return compiler_info;
}

