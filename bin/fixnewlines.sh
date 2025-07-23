# fixnewlines.sh
#
# Files saved under Windows often have lines terminated
# with carriage return ('\r') newline ('\n'), while the
# UNIX convention is to terminate lines with newline.
# The extra carriage returns will cause most UNIX shells
# to choke (complain about syntax errors when the script
# looks fine). This script removes any carriage returns
# that may be present in files with names that end in
# .sh, .txt, .md, .cpp, .h. It also makes shell
# scripts (.sh) executable.
for f in *.sh *.txt *.md *.cpp *.c *.h
do
    if [[ "$f" != "fixnewlines.sh" && "${f::1}" != "*" ]]; then
	echo "Fixing line endings for $f"
    	tr -d '\r' < $f > $f.tmp
    	mv $f.tmp $f
        if [ "${f#*.}" = "sh" ]; then
            # Turn on x bit for shell scripts
            chmod +x $f
        fi
    fi
done
