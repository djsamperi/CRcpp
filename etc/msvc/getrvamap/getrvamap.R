## Test case from docs (virtual addresses depend on run context)
## va = 7ffac3d43cb7
## ba = 7ffac3cb0000
## rva = va - ba = 0x93cb7

## Usage:
##   Open Visual Studio CMD window (so dumpbin is in PATH).
##   Rscript getrvamap.R "c:/Program Files/R/R-4.2.2/bin/x64/R.dll" 7ffac3d43cb7 7ffac3cb0000

## Select pipeR pipeline package if desired (default magrittr).
USE_PIPER <- FALSE
if(USE_PIPER) {
    library(pipeR)
}

library(stringr) ## find substrings (String, ByteString, or Text in Haskell)
library(rlist) ## takeWhile, skipWhile, etc. (Data.List in Haskell)
library(bignum) ## to work with large hex numbers (Integer in Haskell)

## Convert lines of text (a vector) into a list of records.
vecToList <- function(v) {
    l = list()
    for(x in v) {
        fields <- str_split(str_squish(x)," ")
        vec = fields[[1]]
        len <- length(vec)
        if(len == 4)
            ## There may be invalid lines with four fields, but
            ## they will be filtered out using text.
            l <- append(l,list(list(ordinal=vec[1],
                                    hint=vec[2],
                                    RVA=vec[3],
                                    name=vec[4],
                                    text=x)))
    }
    return(l)
}

testRVA <- function(rvaText) {
    rva <- biginteger(paste("0x",rvaText,sep=""))
    return(rva > rva_min && rva < rva_max)
}

args = commandArgs(trailingOnly=TRUE)
if(length(args) != 3) { ## Doesn't include program name
    stop("Usage: getrvamap <dllfile> <virtualAddr> <baseAddr>")
}

dllfile = args[1]

va = biginteger(paste("0x",args[2],sep=""))
ba = biginteger(paste("0x",args[3],sep=""))
rva = va - ba
                
delta = biginteger("0x1000")
rva_min = rva - delta
rva_max = rva + delta

cmd <- "dumpbin"
cmdArgs <- c("/exports", paste("\"",dllfile,"\"",sep=""))
cmdResults <- system2(cmd, cmdArgs, stdout = TRUE)
resultVector <- str_split(cmdResults, "[\r\n]", simplify=TRUE)
resultList <- vecToList(resultVector)

## Filter pipeline
if(USE_PIPER) {
    result <- resultList %>>%
        list.skipWhile(!str_detect(text,"RVA")) %>>%
        list.skip(2) %>>%
        list.takeWhile(!str_detect(text,"Summary")) %>>%
        list.sort(RVA) %>>%
        list.filter(testRVA(RVA))
} else {
    result <- resultList %>%
        list.skipWhile(!str_detect(text,"RVA")) %>%
        list.skip(2) %>%
        list.takeWhile(!str_detect(text,"Summary")) %>%
        list.sort(RVA) %>%
        list.filter(testRVA(RVA))
}

cat('DLL symbols near RVA = ',format(rva, notation = "hex"),
    " (delta = ", format(delta, notation = "hex"), ")\n")
cat('RVA     ','Name\n')
for(k in 1:length(result)) {
   item = result[[k]]
    cat(item$RVA, item$name, '\n')
}


