#!/bin/sh

# echo "testrunner.sh: $0 $1 $2"

PROJECT_ROOT=$(git rev-parse --show-toplevel)
FIGLET_FONTS_DIR=${HOME}/usr/share/fonts/figlet-fonts

SRCDIRS=("${PROJECT_ROOT}/src/libblastpit" "${PROJECT_ROOT}/src/scaffolding")
BUILDDIR=${PROJECT_ROOT}/build

VERBOSE=""

if [[ "$1" == "-v" ]]
then
	echo "Using verbose mode"
	VERBOSE="-v"
elif [[ "$1" == "-msan" ]]
then
	echo "Running memory sanitizer"
elif [[ "$1" == "-asan" ]]
then
	echo "Running address sanitizer"
elif [[ "$1" == "--no-python" ]]
then
	echo "Disabling Python tests"
elif [[ "$1" == "--long" ]]
then
	echo "Running long tests"
	VERBOSE="-v -l"
else
	if [ ! -z ${1+x} ]
	then
		SRCDIRS=($1)
		BUILDDIR=$1
	fi
fi


if [[ "$1" == "-msan" ]]
then
	echo "Running memory sanitizer"
elif [[ "$1" == "-asan" ]]
then
	echo "Running address sanitizer"
elif [[ "$1" == "--no-python" ]]
then
	echo "Disabling Python tests"
elif [[ "$1" == "--long" ]]
then
	echo "Running long tests"
	VERBOSE="-v -l"
else
	if [ ! -z ${1+x} ]
	then
		SRCDIRS=($1)
		BUILDDIR=$1
	fi
fi


# Black        0;30     Dark Gray     1;30
# Red          0;31     Light Red     1;31
# Green        0;32     Light Green   1;32
# Brown/Orange 0;33     Yellow        1;33
# Blue         0;34     Light Blue    1;34
# Purple       0;35     Light Purple  1;35
# Cyan         0;36     Light Cyan    1;36
# Light Gray   0;37     White         1;37

# RED='\033[0;31m'
# YELLOW='\033[1;33m'
# NC='\033[0m' # No Color

RED="\e[31m"
GREEN="\e[32m"
BLUE="\e[34m"
MAGENTA="\e[35m"
WHITE="\e[39m"
YELLOW="\e[33m"


function print_large()
{ # Usage: print_large "<text>" "<font>"

	if [[ $(command -v figlet) ]]
	then
		if [[ -f ${FIGLET_FONTS_DIR}/$2.flf ]]
		then
			echo $1 | figlet -d ${FIGLET_FONTS_DIR} -f $2
		else
			echo "Figlet font file ${FIGLET_FONTS_DIR}/$2.flf not found"
			echo $1 | figlet
		fi
	else
		echo -e $1
	fi
}

function set_colour()
{
	echo -e $1
}

NUM_SRCDIRS=${#SRCDIRS[@]}
for (( i=0; i<${NUM_SRCDIRS}; i++));
do

	if [[ "$1" == "-v" ]]
	then
		echo -e "${YELLOW}\nChecking for unit tests in ${SRCDIRS[i]}...${WHITE}\\n"
	fi
	pushd ${SRCDIRS[i]} > /dev/null
	CTESTS=$(ls -1t ut_*.c* 2>/dev/null | sed -e 's/ut_//' -e 's/\.c.*$//' ) >/dev/null 2>&1
	PYTESTS=$(ls -1t ut_*.py 2>/dev/null | sed -e 's/ut_//' -e 's/\.py$//') >/dev/null 2>&1
	popd > /dev/null

	TESTS="${CTESTS} ${PYTESTS}"
	TESTS=$(echo -e "${TESTS// /\\n}" | sort -u)
	if [[ "$1" == "-v" ]]
	then
		echo "Tests found: ${TESTS}"
	fi

	if [[ "$2" == "-valgrind" ]]
	then
		VALGRIND="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes --expensive-definedness-checks=yes --gen-suppressions=all --error-exitcode=1 --exit-on-first-error=yes --suppressions=res/valgrind/valgrind.supp"
	fi

	for TEST in ${TESTS}
	do
		if [[ -x "${BUILDDIR}/ut_${TEST}_x" ]]
		then
			if [[ "$1" == "-v" ]]
			then
				echo -e "${BLUE}\nRunning [ut_${TEST}_x] ${VERBOSE}${WHITE}"
				echo
			fi
			if [ -z "${VALGRIND}" ]
			then
				${BUILDDIR}/ut_${TEST}_x ${VERBOSE}
			else
				${VALGRIND} ${BUILDDIR}/ut_${TEST}_x ${VERBOSE}
			fi
			if [ $? -ne 0 ]
			then
				if command -v failed &> /dev/null
				then
					failed
				else
					set_colour $RED
					print_large "FAILED" "Bloody"
					set_colour $WHITE
				fi
				exit 1
			fi
		fi

	#######################################################################
	#                                Note                                 #
	#######################################################################

	# Python can cause wierd failures with the address sanitizer, so it is best
	# to do all memory checking in the c/c++ tests and to run any Python tests
	# with sanitizing disabled.

		if [[ "$1" == "-msan" || "$1" == "-asan" || "$1" == "--no-python" ]]
		then
			continue
		fi

		if [[ -x "${SRCDIRS[i]}/ut_${TEST}.py" ]]
		then
			if [[ "$1" == "-v" ]]
			then
				echo -e "${MAGENTA}\n[ut_${TEST}.py]${WHITE}"
				echo
			fi
			# With ASAN_OPTIONS=detect_leaks=1 python tests will fail to complete
			# With it set to zero we will still see things but testing will continue

			PYTHONPATH=${PYTHONPATH}:${PROJECT_ROOT}/build python3 ${SRCDIRS[i]}/ut_${TEST}.py

			# UBSAN_OPTIONS=print_stacktrace=1 ASAN_OPTIONS=detect_leaks=0 LD_PRELOAD=$(clang -print-file-name=libclang_rt.asan-x86_64.so) PYTHONPATH=${PYTHONPATH}:${PROJECT_ROOT}/build python3 ${SRCDIR}/t_${TEST}.py

			# ASAN_OPTIONS=suppressions=${PROJECT_ROOT}/res/.asan_suppressions LD_PRELOAD=$(clang -print-file-name=libclang_rt.asan-x86_64.so) PYTHONPATH=${PYTHONPATH}:${PROJECT_ROOT}/build python3 ${SRCDIR}/t_${TEST}.py
			if [ $? -ne 0 ]
			then
				if command -v failed &> /dev/null
				then
					failed
				else
					set_colour $RED
					print_large "FAILED" "Bloody"
					set_colour $WHITE
				fi
				exit 1
			fi
		fi
	done

done

# set_colour $GREEN
# print_large "PASSED" "DOS_Rebel"
# set_colour $WHITE
