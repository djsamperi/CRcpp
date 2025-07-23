# Checks that R and ninja are present on a UNIX system.
envproblem=0

# Check PATH for R (cmd return status 0 means TRUE or OK!)
if ! $(which R > /dev/null); then
    envproblem=1
    echo "--"
    echo "-- PROBLEM: Could not find R in PATH."
    echo "-- Often fixed using"
    echo "-- export PATH=<path-to-R>:\$PATH"
    echo "--"
fi

# Check PATH for ninja
if ! $(which ninja > /dev/null); then
    envproblem=1
    echo "--"
    echo "-- PROBLEM: CMake requires ninja on non-Windows systems."
    echo "-- Typical Ubuntu install cmd: sudo apt-get install ninja-build"
    echo "-- Typical MacOS install cmd: brew install ninja"
    echo "--"
fi

exit $envproblem
