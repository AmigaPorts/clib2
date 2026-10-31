#!/bin/sh
# Fail if a symbol is defined in more than one member of a library archive.
# Which of the two definitions a program gets then depends on what else it
# links, and when it needs both members, it does not link at all.
#
#	check-duplicate-symbols.sh NM ARCHIVE...

nm=$1
shift
status=0

if [ $# -eq 0 ]
then
	echo "no archives to check" >&2
	exit 1
fi

# The 68k soft-float libm has these both as aliases of __eqdf2 in
# math_eqdf2.o and in members of their own; all of them call IEEEDPCmp().
known="___gedf2 ___gtdf2 ___ledf2 ___ltdf2 ___nedf2"

for archive
do
	# "member.o:" lines name the member; then "value type name", where
	# T, D, B, R and the like are definitions, and weak (W, V) and common
	# (C) symbols may be defined more than once.
	duplicates=$("$nm" -g --defined-only "$archive" 2>/dev/null | awk -v known="$known" '
		BEGIN { split(known, k); for (i in k) skip[k[i]] = 1 }
		/:$/ { member = substr($1, 1, length($1) - 1); next }
		NF == 3 && $2 ~ /^[A-UX-Z]$/ && $2 != "C" && !($3 in skip) {
			if ($3 in first)
				print $3 " in " first[$3] " and " member
			else
				first[$3] = member
		}')

	if [ -n "$duplicates" ]
	then
		echo "$archive:"
		echo "$duplicates" | sed 's/^/	/'
		status=1
	fi
done

exit $status
