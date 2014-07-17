#!/bin/sh
if gnome-doc-prepare --version < /dev/null > /dev/null 2>&1; then
gnome-doc-prepare -f --copy
else
touch confsupp/gnome-doc-utils.make
fi
