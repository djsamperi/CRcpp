// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
// RInside.cpp: R/C++ interface class library -- Easier R embedding into C++
//
// Copyright (C) 2009         Dirk Eddelbuettel
// Copyright (C) 2010 - 2022  Dirk Eddelbuettel and Romain Francois
//
// This file is part of RInside.
//
// RInside is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// RInside is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RInside.  If not, see <http://www.gnu.org/licenses/>.

#include <RInside.h>
#include <Callbacks.h>
#ifndef _WIN32
  #define R_INTERFACE_PTRS
  #include <Rinterface.h>
#endif

RInside* RInside::instance_m = 0 ;

const char *programName = "RInside";

#ifdef _WIN32
    // on Windows, we need to provide setenv which is in the file setenv.c here
    #include "setenv/setenv.c"
extern "C" {
    extern int optind;
}

#ifndef _MSC_VER
    #include <windef.h>
    char rHome[MAX_PATH+1];
#endif // _MSC_VER

#endif // _WIN32

#ifdef _MSC_VER

// The external reference to optind causes a link-time error under MSVC, so
// we disable use of this variable for now.
// 
// Including <windef.h> under MSVC brings in other headers that cause
// MSVC to choke (tested with Visual Studio 2019). The work-around is to
// use <windows.h> instead, along with the define WIN32_LEAN_AND_MEAN
// so this doesn't come with too much unnecessary baggage.
// 
// Also had to add a gettimeofday() function for MSVC below.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
   char rHome[MAX_PATH+1];
 
#include <stdint.h>

typedef struct timeval {
        long tv_sec;
        long tv_usec;
} TIMEVAL, * PTIMEVAL, * LPTIMEVAL;

#include <chrono>
// <chrono> has been enhanced for c++20, but support for the new features
// is spotty at best. On the other hand, we don't need much to approximate
// gettimeofday(), and RInside uses the time in microseconds to
// initialize a random number generator, so precision (as in gps_clock)
// is not important.

int gettimeofday(struct timeval* tp, struct timezone* tzp) {
// gettimeofday() is provided by MinGW via <sys/time.h>, but this is
// not available on MS Visual Studio, so we implement using <chrono>.
// The timezone parameter is obsolete today, so it should be specified
// as NULL. Recall that the microsecond field represents a residual,
// after seconds is computed, and usually, tv_usec < tv_sec.
// This code was posted on stackoverflow.com by lilucpp, with
// input from Howard Hinnant.
  namespace sc = std::chrono;
  sc::system_clock::duration d = sc::system_clock::now().time_since_epoch();
  sc::seconds s = sc::duration_cast<sc::seconds>(d);
  tp->tv_sec = s.count();
  tp->tv_usec = sc::duration_cast<sc::microseconds>(d - s).count();

  return 0;
}

#endif // end _MSC_VER

RInside::~RInside() {           // now empty as MemBuf is internal
    R_dot_Last();
    R_RunExitFinalizers();
    R_CleanTempDir();
    //Rf_KillAllDevices();
    //#ifndef WIN32
    //fpu_setup(FALSE);
    //#endif
    Rf_endEmbeddedR(0);
    instance_m = 0 ;
    delete global_env_m;
}

RInside::RInside(): global_env_m(NULL)
#ifdef RINSIDE_CALLBACKS
    , callbacks(0)
#endif
{
    initialize(0, 0, false, false, false);
}

#ifdef _WIN32
#if R_VERSION >= R_Version(4,2,0)
static int myReadConsole(const char *prompt, unsigned char *buf, int len, int addtohistory) {
#else
static int myReadConsole(const char *prompt, char *buf, int len, int addtohistory) {
#endif
    fputs(prompt, stdout);
    fflush(stdout);
    if (fgets((char *)buf, len, stdin))
        return 1;
    else
        return 0;
}

static void myWriteConsole(const char *buf, int len) {
    fwrite(buf, sizeof(char), len, stdout);
    fflush(stdout);
}

static void myCallBack() {
    /* called during i/o, eval, graphics in ProcessEvents */
}

static void myBusy(int which) {
    /* set a busy cursor ... if which = 1, unset if which = 0 */
}

void myAskOk(const char *info) {

}

int myAskYesNoCancel(const char *question) {
    const int yes = 1;
    return yes;
}

#endif

RInside::RInside(const int argc, const char* const argv[], const bool loadRcpp,
                 const bool verbose, const bool interactive)
#ifdef RINSIDE_CALLBACKS
    		: callbacks(0)
#endif
{
    initialize(argc, argv, loadRcpp, verbose, interactive);
}

// TODO: use a vector<string> would make all this a bit more readable
void RInside::initialize(const int argc, const char* const argv[], const bool loadRcpp,
                         const bool verbose, const bool interactive) {

    if (instance_m) {
        throw std::runtime_error( "can only have one RInside instance" ) ;
    } else {
        instance_m = this ;
    }

    verbose_m = verbose;          	// Default is false
    interactive_m = interactive;

    // generated from Makevars{.win}
    #include "RInsideEnvVars.h"

    #ifdef _WIN32
    // we need a special case for Windows where users may deploy an RInside binary from CRAN
    // which will have R_HOME set at compile time to CRAN's value -- so let's try to correct
    // this here: a) allow user's setting of R_HOME and b) use R's get_R_HOME() function
    if (getenv("R_HOME") == NULL) { 		// if on Windows and not set
        char *rhome = get_R_HOME();		// query it, including registry
        if (rhome != NULL) {                    // if something was found
            setenv("R_HOME", get_R_HOME(), 1);  // store what we got as R_HOME
        }					// this will now be used in next blocks
    }
    #endif

    for (int i = 0; R_VARS[i] != NULL; i+= 2) {
        if (getenv(R_VARS[i]) == NULL) { // if env variable is not yet set
            if (setenv(R_VARS[i],R_VARS[i+1],1) != 0){
                throw std::runtime_error(std::string("Could not set R environment variable ") +
                                         std::string(R_VARS[i]) + std::string(" to ") +
                                         std::string(R_VARS[i+1]));
            }
        }
    }

    #ifndef _WIN32
    R_SignalHandlers = 0;               // Don't let R set up its own signal handlers
    #endif

    init_tempdir();

    const char *R_argv[] = {(char*)programName, "--gui=none", "--no-save",
                            "--silent", "--vanilla", "--slave", "--no-readline"};
    int R_argc = sizeof(R_argv) / sizeof(R_argv[0]);
    if (interactive_m) R_argc--; //Deleting the --no-readline option in interactive mode
    Rf_initEmbeddedR(R_argc, (char**)R_argv);

    #ifndef _WIN32
    R_CStackLimit = -1;      		// Don't do any stack checking, see R Exts, '8.1.5 Threading issues'
    #endif

    R_ReplDLLinit();                    // this is to populate the repl console buffers

    structRstart Rst;

// MSVC defines ReadConsole and WriteConsole as macros, and this breaks
// code from <R_HOME>/include/R_ext/RStartup.h (structRstart). So we
// disable these macros here.
#ifdef _MSC_VER
#undef ReadConsole
#undef WriteConsole
#endif
    
    R_DefParams(&Rst);
    Rst.R_Interactive = (Rboolean) interactive_m;       // sets interactive() to eval to false
#ifdef _WIN32

    char *temp = getenv("R_HOME");       // which is set above as part of R_VARS
#ifdef _MSC_VER
    // Fix MSVC warning about length(rHome) = MAX_PATH
    strncpy(rHome, temp, MAX_PATH-1);  
#else
    strncpy(rHome, temp, MAX_PATH);
#endif

    // Note that these settings apply to _WIN32 only.
    // Only Rst.R_Interactive above applies in all cases.
    // But when the latter is TRUE under Windows the user is
    // not asked about saving the workspace, because the
    // Windows-only callback myAskYesNoCancel does not ask.
    Rst.rhome = rHome;
    Rst.home = getRUser();
    Rst.CharacterMode = LinkDLL;
    Rst.ReadConsole = myReadConsole;
    Rst.WriteConsole = myWriteConsole;
    Rst.CallBack = myCallBack;
    Rst.ShowMessage = myAskOk;
    Rst.YesNoCancel = myAskYesNoCancel;
    Rst.Busy = myBusy;

    //See front-ends/rtest.c for info about this parameters.
    //Rst.EmitEmbeddedUTF8 = (Rboolean)FALSE;
    //Rst.WriteConsoleEx = myWriteConsoleEx;
    //Rst.R_Quiet = (Rboolean)TRUE;
    //Rst.SaveAction = SA_NOSAVE; // save workspace prompt on exit
    
#endif // _WIN32
    R_SetParams(&Rst);

    if (true || loadRcpp) {             // we always need Rcpp, so load it anyway
        // Use R_LIBS to locate Rcpp.
        const char* rlibs = std::getenv("R_LIBS");
        if(rlibs != nullptr) {
            SEXP paths;
            PROTECT(paths = Rf_allocVector(STRSXP, 1));
            SET_STRING_ELT(paths, 0, Rf_mkChar(rlibs));
            Rf_eval(Rf_lang2(Rf_install(".libPaths"), paths), R_GlobalEnv);
            UNPROTECT(1);
        } else {
            Rprintf("R_LIBS not set");
        }


        // Rf_install is used best by first assigning like this so that symbols get into
        // the symbol table where they cannot be garbage collected; doing it on the fly
        // does expose a minuscule risk of garbage collection -- with thanks to Doug Bates
        // for the explanation and Luke Tierney for the heads-up
        SEXP suppressMessagesSymbol = Rf_install("suppressMessages");
        SEXP requireSymbol = Rf_install("require");
        SEXP reqsymlang, langobj;
        // Protect temporaries as suggested by 'rchk', with thanks to Tomas Kalibera
        PROTECT(reqsymlang = Rf_lang2(requireSymbol, Rf_mkString("Rcpp")));
        PROTECT(langobj = Rf_lang2(suppressMessagesSymbol, reqsymlang));
        Rf_eval(langobj, R_GlobalEnv);
        UNPROTECT(2);
    }

    global_env_m = new Rcpp::Environment();         // member variable for access to R's global environment

    autoloads();                        // loads all default packages, using code autogenerate from Makevars{,.win}

    if ((argc - optind) > 1){           // for argv vector in Global Env */
        Rcpp::CharacterVector s_argv( argv+(1+optind), argv+argc );
        assign(s_argv, "argv");
    } else {
        assign(R_NilValue, "argv") ;
    }

    init_rand();                        // for tempfile() to work correctly */
}

void RInside::init_tempdir(void) {
    const char *tmp;
    // FIXME:  if per-session temp directory is used (as R does) then return
    tmp = getenv("TMPDIR");
    if (tmp == NULL) {
        tmp = getenv("TMP");
        if (tmp == NULL) {
            tmp = getenv("TEMP");
            if (tmp == NULL)
                tmp = "/tmp";
            }
    }
    R_TempDir = (char*) tmp;
    if (setenv("R_SESSION_TMPDIR",tmp,1) != 0){
        throw std::runtime_error(std::string("Could not set / replace R_SESSION_TMPDIR to ") + std::string(tmp));
    }
}

void RInside::init_rand(void) { 		// code borrows from R's TimeToSeed() in datetime.c
#ifdef _MSC_VER
    unsigned int pid = 1234; // getpid() for Windows?
#else
    unsigned int pid = getpid();
#endif
    struct timeval tv;          		// this is ifdef'ed by R, we just assume we have it
    gettimeofday (&tv, NULL);           // we could borrow autoconf setup from littler to ensure
    unsigned int seed = (unsigned int)(((uint_least64_t) tv.tv_usec << 16) ^ tv.tv_sec);
    seed ^= (pid << 16);         		// R 2.14.0 started to also use pid to support parallel
    srand(seed);
}

void RInside::autoloads() {

    #include "RInsideAutoloads.h"

    // Autoload default packages and names from autoloads.h
    //
    // This function behaves in almost every way like
    // R's autoload:
    // function (name, package, reset = FALSE, ...)
    // {
    //     if (!reset && exists(name, envir = .GlobalEnv, inherits = FALSE))
    //        stop("an object with that name already exists")
    //     m <- match.call()
    //     m[[1]] <- as.name("list")
    //     newcall <- eval(m, parent.frame())
    //     newcall <- as.call(c(as.name("autoloader"), newcall))
    //     newcall$reset <- NULL
    //     if (is.na(match(package, .Autoloaded)))
    //        assign(".Autoloaded", c(package, .Autoloaded), env = .AutoloadEnv)
    //     do.call("delayedAssign", list(name, newcall, .GlobalEnv,
    //                                                         .AutoloadEnv))
    //     invisible()
    // }
    //
    // What's missing is the updating of the string vector .Autoloaded with
    // the list of packages, which by my code analysis is useless and only
    // for informational purposes.
    //
    //

    // we build the call :
    //
    //  delayedAssign( NAME,
    //          autoloader( name = NAME, package = PACKAGE),
    //          .GlobalEnv,
    //          .AutoloadEnv )
    //
    //  where :
    //  - PACKAGE is updated in a loop
    //  - NAME is updated in a loop
    //
    //

    int i,j, idx=0, nobj ;
    Rcpp::Language delayed_assign_call(Rcpp::Function("delayedAssign"),
                                       R_NilValue,     // arg1: assigned in loop
                                       R_NilValue,     // arg2: assigned in loop
                                       *global_env_m,
                                       global_env_m->find(".AutoloadEnv")
                                       );
    Rcpp::Language::Proxy delayed_assign_name  = delayed_assign_call[1];

    Rcpp::Language autoloader_call(Rcpp::Function("autoloader"),
                                   Rcpp::Named( "name", R_NilValue) ,  // arg1 : assigned in loop
                                   Rcpp::Named( "package", R_NilValue) // arg2 : assigned in loop
                                   );
    Rcpp::Language::Proxy autoloader_name = autoloader_call[1];
    Rcpp::Language::Proxy autoloader_pack = autoloader_call[2];
    delayed_assign_call[2] = autoloader_call;

    try {
        for( i=0; i<packc; i++){

            // set the 'package' argument of the autoloader call */
            autoloader_pack = pack[i] ;

            nobj = packobjc[i] ;
            for (j = 0; j < nobj ; j++){

                // set the 'name' argument of the autoloader call */
                autoloader_name = packobj[idx+j] ;

                // Set the 'name' argument of the delayedAssign call */
                delayed_assign_name = packobj[idx+j] ;

                // evaluate the call */
                delayed_assign_call.eval() ;

            }
            idx += packobjc[i] ;
        }
    } catch( std::exception& ex){
        throw std::runtime_error(std::string("Error calling delayedAssign: ") + std::string(ex.what()));
    }
}

// this is a non-throwing version returning an error code
int RInside::parseEval(const std::string & line, SEXP & ans) {
    ParseStatus status;
    SEXP cmdSexp, cmdexpr = R_NilValue;
    int i, errorOccurred;

    mb_m.add((char*)line.c_str());

    PROTECT(cmdSexp = Rf_allocVector(STRSXP, 1));
    SET_STRING_ELT(cmdSexp, 0, Rf_mkChar(mb_m.getBufPtr()));

    cmdexpr = PROTECT(R_ParseVector(cmdSexp, -1, &status, R_NilValue));

    switch (status){
    case PARSE_OK:
        // Loop is needed here as EXPSEXP might be of length > 1
        for(i = 0; i < Rf_length(cmdexpr); i++){
            ans = R_tryEval(VECTOR_ELT(cmdexpr, i), *global_env_m, &errorOccurred);
            if (errorOccurred) {
                if (verbose_m) Rf_warning("%s: Error in evaluating R code (%d)\n", programName, status);
                UNPROTECT(2);
                mb_m.rewind();
                return 1;
            }
            if (verbose_m) {
                Rf_PrintValue(ans);
            }
        }
        mb_m.rewind();
        break;
    case PARSE_INCOMPLETE:
        // need to read another line
        break;
    case PARSE_NULL:
        if (verbose_m) Rf_warning("%s: ParseStatus is null (%d)\n", programName, status);
        UNPROTECT(2);
        mb_m.rewind();
        return 1;
        break;
    case PARSE_ERROR:
        if (verbose_m) Rf_warning("Parse Error: \"%s\"\n", line.c_str());
        UNPROTECT(2);
        mb_m.rewind();
        return 1;
        break;
    case PARSE_EOF:
        if (verbose_m) Rf_warning("%s: ParseStatus is eof (%d)\n", programName, status);
        break;
    default:
        if (verbose_m) Rf_warning("%s: ParseStatus is not documented %d\n", programName, status);
        UNPROTECT(2);
        mb_m.rewind();
        return 1;
        break;
    }
    UNPROTECT(2);
    return 0;
}

void RInside::parseEvalQ(const std::string & line) {
    SEXP ans;
    int rc = parseEval(line, ans);
    if (rc != 0) {
        throw std::runtime_error(std::string("Error evaluating: ") + line);
    }
}

void RInside::parseEvalQNT(const std::string & line) {
    SEXP ans;
    parseEval(line, ans);
}

RInside::Proxy RInside::parseEval(const std::string & line) {
    SEXP ans;
    int rc = parseEval(line, ans);
    if (rc != 0) {
        throw std::runtime_error(std::string("Error evaluating: ") + line);
    }
    return Proxy( ans );
}

RInside::Proxy RInside::parseEvalNT(const std::string & line) {
    SEXP ans;
    parseEval(line, ans);
    return Proxy( ans );
}

Rcpp::Environment::Binding RInside::operator[]( const std::string& name ){
    return (*global_env_m)[name];
}

RInside& RInside::instance(){
    return *instance_m;
}

RInside* RInside::instancePtr(){
    return instance_m;
}

extern "C" {
     void run_Rmainloop(void);
}
 
void RInside::repl() {
    run_Rmainloop();

    // Causes problems under Windows.
    // For example, terminates after plot(0,0)
    // R_ReplDLLinit();
    // while (R_ReplDLLdo1() > 0) {}
}

/* callbacks */

#ifdef RINSIDE_CALLBACKS

void Callbacks::Busy_( int which ){
    R_is_busy = static_cast<bool>( which ) ;
    Busy( R_is_busy ) ;
}

int Callbacks::ReadConsole_( const char* prompt, unsigned char* buf, int len, int addtohistory ){
    try {
        std::string res( ReadConsole( prompt, static_cast<bool>(addtohistory) ) ) ;

        /* At some point we need to figure out what to do if the result is
         * longer than "len"... For now, just truncate. */

        int l = res.size() ;
        int last = (l>len-1)?len-1:l ;
        strncpy( (char*)buf, res.c_str(), last ) ;
        buf[last] = 0 ;
        return 1 ;
    } catch( const std::exception& ex){
        return -1 ;
    }
}


void Callbacks::WriteConsole_( const char* buf, int len, int oType ){
    if( len ){
        buffer.assign( buf, len ) ;
        WriteConsole( buffer, oType) ;
    }
}

void RInside_ShowMessage( const char* message ){
    RInside::instance().callbacks->ShowMessage( message ) ;
}

void RInside_WriteConsoleEx( const char* message, int len, int oType ){
    RInside::instance().callbacks->WriteConsole_( message, len, oType ) ;
}

int RInside_ReadConsole(const char *prompt, unsigned char *buf, int len, int addtohistory){
    return RInside::instance().callbacks->ReadConsole_( prompt, buf, len, addtohistory ) ;
}

void RInside_ResetConsole(){
    RInside::instance().callbacks->ResetConsole() ;
}

void RInside_FlushConsole(){
    RInside::instance().callbacks->FlushConsole() ;
}

void RInside_ClearerrConsole(){
    RInside::instance().callbacks->CleanerrConsole() ;
}

void RInside_Busy( int which ){
    RInside::instance().callbacks->Busy_(which) ;
}

void RInside::set_callbacks(Callbacks* callbacks_){
    callbacks = callbacks_ ;

#ifdef _WIN32
    // do something to tell user that he doesn't get this
#else

    /* short circuit the callback function pointers */
    if( callbacks->has_ShowMessage() ){
        ptr_R_ShowMessage = RInside_ShowMessage ;
    }
    if( callbacks->has_ReadConsole() ){
        ptr_R_ReadConsole = RInside_ReadConsole;
    }
    if( callbacks->has_WriteConsole() ){
        ptr_R_WriteConsoleEx = RInside_WriteConsoleEx ;
        ptr_R_WriteConsole = NULL;
        }
    if( callbacks->has_ResetConsole() ){
        ptr_R_ResetConsole = RInside_ResetConsole;
    }
    if( callbacks->has_FlushConsole() ){
        ptr_R_FlushConsole = RInside_FlushConsole;
    }
    if( callbacks->has_CleanerrConsole() ){
        ptr_R_ClearerrConsole = RInside_ClearerrConsole;
    }
    if( callbacks->has_Busy() ){
        ptr_R_Busy = RInside_Busy;
    }

    R_Outputfile = NULL;
    R_Consolefile = NULL;
#endif
}

#endif

#ifdef _MSC_VER

extern "C" {
     
#include <string.h>
#include <stdio.h>

int opterr = 1,             /* if error message should be printed */
    optind = 1,             /* index into parent argv vector */
    optopt = 0,             /* character checked for validity */
    optreset = 0;           /* reset getopt */
char *optarg;            /* argument associated with option */

#define BADCH   (int)'?'
#define BADARG  (int)':'
#define EMSG    ""
    
/*
* getopt -- Parse argc/argv argument vector.
*/
int getopt(int nargc, char * const nargv[], const char *ostr)
{
    // Example ostr = ":if:lrx"
    static char *place = EMSG;              /* option letter processing */
    const char *oli;                        /* option letter list index */

    // *place is 0 initiall...
    if (optreset || !*place) {              /* update scanning pointer */
	optreset = 0;
	if (optind >= nargc || *(place = nargv[optind]) != '-') {
	    place = EMSG; // Non-option parameter (no leading '-')
	    return (-1);
	}
	// place[0] == '-', so we look at place[1]. If we get '--',
	// there are no more options, return -1, otherwise next if...
	if (place[1] && *++place == '-') {      /* found "--" */
	    ++optind; // place points to option after '--'
	    place = EMSG;
	    return (-1);
	}
    }	                                       /* option letter okay? */
    if ((optopt = (int)*place++) == (int)':' ||
	!(oli = strchr(ostr, optopt))) {
	/*
	 * if the user didn't specify '-' as an option,
	 * assume it means -1.
	 */
	if (optopt == (int)'-')
	    return (-1);
	if (!*place)
	    ++optind;
	if (opterr && *ostr != ':')
	    (void)printf("illegal option -- %c\n", optopt);
	return (BADCH);
    }
    if (*++oli != ':') {                    /* don't need argument */
	optarg = NULL;
	if (!*place)
	    ++optind;
    }
    else {                                  /* need an argument */
	if (*place)                     /* no white space */
	    optarg = place;
	else if (nargc <= ++optind) {   /* no arg */
	    place = EMSG;
	    if (*ostr == ':')
		return (BADARG);
	    if (opterr)
		(void)printf("option requires an argument -- %c\n", optopt);
	    return (BADCH);
	}
	else                            /* white space */
	    optarg = nargv[optind];
	place = EMSG;
	++optind;
    }
    return (optopt);                        /* dump back option letter */
}

} // end extern "C"    
#endif // _MSC_VER
