FUZZ_BINARY$(FUZZ_BINARY) = t_linkedlist_x
FUZZ_INPUTS = blastpit.i mqtt.h

CC = gcc
CXX = g++
DBG = gdb

all:	tup
	make test


.PHONY:	tup clean graph test

test:
	@echo -e "\e[33mRunning tests...\e[39m"
	@sh -c ./testrunner.sh

tup:
	tup

clean:
	rm -vf *.o *.so t_*_x *_wrap* blastpy.py tup.pdf
	rm -vrf __pycache__ .tup

graph:
	tup graph . | dot -Tpdf > tup.pdf
	xdg-open tup.pdf

fuzz:	
	make clean
	CC=afl-clang CXX=afl-clang++ tup
	echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
	mkdir -p /tmp/afl-{in,out}
	cp $(FUZZ_BINARY) /tmp/
	cp $(FUZZ_INPUTS) /tmp/afl-in/
	cd /tmp/
	afl-fuzz -i /tmp/afl-in -o /tmp/afl-out /tmp/$(FUZZ_BINARY)
