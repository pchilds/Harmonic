#!/bin/sh
./gdp.sh
if test -z `which autoreconf`; then
echo "*** No autoreconf found, please install it ***"
exit 1
else
autoreconf -vif || exit $?
fi
