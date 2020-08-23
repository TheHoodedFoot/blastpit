#!/bin/sh

IFS=$(echo -en "\n\b")

THEORYDIR=$(git rev-parse --show-toplevel)/doc/theory
mkdir -p ${THEORYDIR}

EBOOKSDIR=/mnt/tank/data/library/eBooks
EBOOKS=( \
	"Programming/C/21st Century C [2013 Ben Klemens; O'Reilly].pdf" \
	"Programming/Theory/Algorithms-JeffE.pdf" \
	"Programming/Theory/Clean Architecture_ A Craftsman - Robert C. Martin.pdf" \
	"Programming/Theory/Head.First.Software.Development.pdf" \
	"Programming/Testing/Kent Beck - Test-Driven Development by Example.pdf" \
	"Programming/C/Object_Oriented_C.pdf" \
	"Programming/C/Pragmatic.Test.Driven.Development.for.Embedded.C.Apr.2011.pdf" \
	"Programming/Theory/Prentice Hall - Clean Code - A Handbook of Agile Software Craftsmanship.pdf" \
	"Programming/Theory/Clean_Code_Ebook/The Clean Coder, Robert C. Martin, 2011.pdf" \
	"Programming/C/Understanding_and_using_C_pointers.pdf" \
	)

for ebook in "${EBOOKS[@]}"
do
	ln -vs ${EBOOKSDIR}/$ebook ${THEORYDIR}/
done
