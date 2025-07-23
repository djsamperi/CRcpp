-- Copyright (c) 2023 by Dominick Samperi

-- Test case from docs (virtual addresses depend on run context)
-- va = 7ffac3d43cb7
-- ba = 7ffac3cb0000
-- rva = va - ba = 0x93cb7

-- Usage: Bring up Visual Studio CMD window (so dumpbin is in PATH).
--   Start ghci (stack ghci) with getrvamap.hs in working directory.
--   :load getrvamap.hs
--   :run main "c:/Program Files/R/R-4.2.2/bin/x64/R.dll" 7ffac3d43cb7 7ffac3cb0000
-- Alternatively, compile using ghc and run the executable.

import System.Environment(getArgs)
import System.Process(readProcess)  
import qualified Data.List.Split as S
import qualified Data.List as L
import Numeric (readHex, showHex, showIntAtBase)
import System.IO
import System.Exit

-- Fields become accessor functions, for example
-- ordinal :: Rec -> String. Remember that these names
-- have module scope and can only be resused in a
-- let context (see main). The text field is the string
-- that the other fields are derived from.
data Rec = Rec { ordinal :: String,
                 hint :: String,
                 rva  :: String,
                 name :: String,
                 text :: String } deriving(Show,Eq,Ord)

-- Recall that String = [Char] (list of Char), so it makes sense
-- to look at all tails of "FooRVA", a list containing "FooRVA", "ooRVA",
-- etc. Then "RVA" is a substring of "FooRVA" if and only if
-- it is a prefix of some tail, so we look for "any" such case.
contains :: String -> String -> Bool
contains substr str = any (L.isPrefixOf substr) (L.tails str)

-- We are interested in RVA's close to the one where a fault occurred.
testRVA :: Integer -> Integer -> Rec -> Bool
testRVA minRVA maxRVA rec =
    value > minRVA && value < maxRVA
      where
        value = hexStrToInt (rva rec)

-- Function composition shows computations in the reverse order
-- that they will be carried out.
process :: Integer -> Integer -> [Rec] -> [Rec]
process rvaMin rvaMax slist =
  L.filter (\rec -> testRVA rvaMin rvaMax rec)
     $ L.sortOn (rva) 
        $ L.takeWhile (\x -> not (contains "Summary" (text x)))
           $ tail $ L.dropWhile (\x -> not (contains "RVA" (text x))) slist

-- Do notation shows computations in the order
-- that they will be carried out, as in the pipeline version
-- in getrvamap.R.
processdo :: Integer -> Integer -> [Rec] -> [Rec]
processdo rvaMin rvaMax slist = do
  let a1 = tail $ L.dropWhile (\x -> not (contains "RVA" (text x))) slist
  let a2 = L.takeWhile (\x -> not (contains "Summary" (text x))) a1
  let a3 = L.sortOn (rva) a2
  w <- L.filter (\rec -> testRVA rvaMin rvaMax rec) a3
  return(w)

-- Take a list of input strings, one per input line, and create
-- a Rec structure out of each string if there are four tokens
-- present. Otherwise the line is dropped. Some lines may lead to
-- invalid records, but the 'text' field is used to filter them out.
-- Recursion must be used here, while mutability of variables is
-- used in the R version in getrvamap.R, and in the Python version
-- in getrvamap.py.
vecToList :: [String] -> [Rec]
vecToList [] = []
vecToList (str:sx) = if length sl == 4
                    then
                      (Rec (sl!!0) (sl!!1) (sl!!2) (sl!!3) str):(vecToList sx)
                    else (vecToList sx)
                      where
                        sl = filter (/= "") $ S.splitOn " " str

-- Leading "0x" is optional
hexStrToInt :: String -> Integer -- bigint
hexStrToInt s = fetch $ readHex (chop0x s)
  where
        fetch :: [(a,b)] -> a
        fetch [(x,_)] = x
        chop0x :: String -> String
        chop0x s = if L.isPrefixOf "0x" s then
                     tail (tail s)
                   else
                      s

-- Includes "0x" prefix
intToHexStr :: Integer -> String
intToHexStr i = showHex i ""

-- With the exception of main, printRec and exitWithErrorMessage, all
-- functions are pure: same input always yields that same output,
-- no side-effects.
printRec :: Rec -> IO ()
printRec rec = do
  putStrLn $ (rva rec) ++ " " ++ (name rec)

exitWithErrorMessage :: String -> ExitCode -> IO a
exitWithErrorMessage str e = hPutStrLn stderr str >> exitWith e

main :: IO ()
main = do
  args <- getArgs
  if length args /= 3 then
    die "Usage: getrvamap <dllfile> <virualAddr> <baseAddr>"
  else
    return ()
  
  let va = hexStrToInt (args!!1) :: Integer
  let ba = hexStrToInt (args!!2) :: Integer
  let rva = va - ba :: Integer
  let delta = hexStrToInt "1000" :: Integer
  let rvaMin = rva - delta :: Integer
  let rvaMax = rva + delta :: Integer

  cmdOutStr <- readProcess "dumpbin" ["/exports", args!!0 ] ""

  putStrLn $ "DLL symbols near RVA = " ++ "0x" ++ (showHex rva "")
             ++ " (delta = " ++ "0x" ++ showHex delta "" ++ ")"
  putStrLn $ "RVA      " ++ "Name"
  sequence_ $ map printRec
            $ (processdo rvaMin rvaMax)
            $ vecToList
            $ S.splitOneOf "\r\n" cmdOutStr

  return ()
