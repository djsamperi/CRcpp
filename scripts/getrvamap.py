# getrvamap.py -- find names in DLL given RVA (Visual Studio IDE)

# This version was the first, with no attempt made to do things in the
# functional style. It is also the fastest.

## Test case from docs (virtual addresses depend on run context)
## va = 7ffac3d43cb7
## ba = 7ffac3cb0000
## rva = va - ba = 0x93cb7

## Usage:
##   Open Visual Studio CMD window (so dumpbin is in PATH).
##   python getrvamap.py "c:/Program Files/R/R-4.2.2/bin/x64/R.dll" 7ffac3d43cb7 7ffac3cb0000

import subprocess
import sys

if len(sys.argv) != 4:
    sys.exit("Usage: " + argv[0] + " <path-to-dll> <virtual-addr> <base-addr>")

dllfile = sys.argv[1]

va = int("0x"+sys.argv[2],16) # hex virtual address
base = int("0x"+sys.argv[3],16) # hex base address
rva = va - base

delta = int("0x1000",16)
rva_min = rva - delta
rva_max = rva + delta

result = subprocess.check_output(
    "dumpbin /exports \"" + dllfile + "\"", shell=False)

# split into lines and convert from byte-string to string (UTF-8)
list1 = []
for l in result.split(b"\r\n"):
    list1.append(str(l,'UTF-8'))

# Strip off noise at start/end of the list.
list2 = []
done=False
start=False
for s in list1:
    if(s.strip() == "Summary"):
        done = True
    if(s.find("RVA") != -1):
        start = True
    if(start and (not done)):
        list2.append(s)

# More cleanup
list2 = list2[2:]

# Sort on RVA hex value (same as lexicographic order).
# Invalid lines will have key 00000000 so will appear before
# the valid data.
list2.sort(key=lambda line: "00000000" if len(line.split()) != 4 \
           else line.split()[2])

# Output RVA,Name when RVA is close to computed RVA.
print("DLL symbols near RVA = "+hex(rva)+" (delta = "+hex(delta)+")")
print("RVA     ","Name")
for s in list2:
    t = s.split()
    if len(t) == 4: # Valid records must have 4 fields
        rva_test = int("0x"+t[2],16)
        if(rva_test > rva_min and rva_test < rva_max):
            print(t[2],t[3])


