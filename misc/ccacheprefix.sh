#!/bin/sh

CCACHE=''
if ccache --version 2>&1 | grep '^ccache version .*$' >/dev/null 2>&1; then
	echo 'ccache'
else
	echo ''
fi