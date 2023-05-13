#!/bin/sh

# If we are running in a pipe then we don't want any output
# as it affects vim-dispatch

function runtests()
{
	cat | xargs -n1 -P$(nproc) /bin/sh -c # || exit 1
}

if [ ! -t 1 ]
then
        # We are running from the shell. Enable output
        echo "In the shell"
        runtests
else
        # We are running inside vim dispatch. Disable output
        echo "In a pipe"
        {
        	runtests
        } > /dev/null 2>&1
fi
