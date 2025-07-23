// Main CRcpp app simply runs RInside::repl().
// This is a partial implementation of Rterm that
// has the advantage of being part of RInside, and not
// separate app, useful for interacting with R while
// debugging.
#include <RInside.h>

extern "C" {
    void CRcppBuildRcpp(void);
    void CRcppBuildRInside(void);
}

int main(int argc, char *argv[]) {

    // Do not uncomment: this will cause a seg fault because
    // Rprintf is used before R is initialized.
    //CRcppBuildRcpp();
    
    RInside R(argc, argv, false, false, false);
    CRcppBuildRcpp();
    CRcppBuildRInside();
    R.parseEval("options(prompt = 'R > ')");
    R.repl() ;
    exit(0);
}

