#!/bin/sh

TESTS=( \
	linkedlist \
	message \
	mqtt \
	blastpy \
	blastpit \
	)

for TEST in "${TESTS[@]}"
do
	if [[ -x "t_${TEST}_x" ]]
	then
		echo "Running ${TEST} test..."
		sh -c ./t_${TEST}_x
		if [ $? -ne 0 ]
		then
			exit 1
		fi
	fi
	if [[ -x "t_${TEST}.py" ]]
	then
		sh -c ./t_${TEST}.py
		if [ $? -ne 0 ]
		then
			exit 1
		fi
	fi
done
