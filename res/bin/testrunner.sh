#!/bin/sh

PROJECT_ROOT=$(git rev-parse --show-toplevel)
FIGLET_FONTS_DIR=${HOME}/usr/share/fonts/figlet-fonts

if [ -z ${1+x} ]
then
	SRCDIR=${PROJECT_ROOT}/src/libblastpit
else
	SRCDIR=$1
fi

VERBOSE="-v"

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

echo -e "\nChecking for unit tests in ${SRCDIR}..."
pushd ${SRCDIR} > /dev/null
CTESTS=$(ls -1t t_*.c* | sed -e 's/t_//' -e 's/\.c.*$//')
PYTESTS=$(ls -1t t_*.py | sed -e 's/t_//' -e 's/\.py$//')
TESTS="${CTESTS} ${PYTESTS}"
TESTS=$(echo -e "${TESTS// /\\n}" | sort -u)
popd > /dev/null

if [[ "$1" == "-valgrind" ]]
then
	VALGRIND="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes --expensive-definedness-checks=yes --gen-suppressions=all --error-exitcode=1 --exit-on-first-error=yes --suppressions=res/valgrind/valgrind.supp"
fi

for TEST in ${TESTS}
do
	if [[ -x "t_${TEST}_x" ]]
	then
		echo -e "${BLUE}\nRunning [t_${TEST}_x] ${VERBOSE}${WHITE}"
		echo
		if [ -z "${VALGRIND}" ]
		then
			./t_${TEST}_x ${VERBOSE}
		else
			${VALGRIND} ./t_${TEST}_x ${VERBOSE}
		fi
		if [ $? -ne 0 ]
		then
			set_colour $RED
			print_large "FAILED" "Bloody"
			set_colour $WHITE
			exit 1
		fi
	fi
	if [[ -x "${SRCDIR}/t_${TEST}.py" ]]
	then
		echo -e "${MAGENTA}\n[t_${TEST}.py]${WHITE}"
		echo
		ASAN_OPTIONS=detect_leaks=0 LD_PRELOAD=$(clang -print-file-name=libclang_rt.asan-x86_64.so) PYTHONPATH=${PYTHONPATH}:${PROJECT_ROOT}/build python3 ${SRCDIR}/t_${TEST}.py
		# ASAN_OPTIONS=suppressions=${PROJECT_ROOT}/.asan_suppressions LD_PRELOAD=$(clang -print-file-name=libclang_rt.asan-x86_64.so) PYTHONPATH=${PYTHONPATH}:${PROJECT_ROOT}/build python3 ${SRCDIR}/t_${TEST}.py
		if [ $? -ne 0 ]
		then
			set_colour $RED
			print_large "FAILED" "Bloody"
			set_colour $WHITE
			# exit 1
		fi
	fi
done

# set_colour $GREEN
# print_large "PASSED" "DOS_Rebel"
# set_colour $WHITE
