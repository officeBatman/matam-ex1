#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color
output="log.txt"


for i in {0..26}
do
   # execute the prog and putting output to out file
   valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./HackEnrollment tests/Test$i/students.txt tests/Test$i/courses.txt tests/Test$i/hackers.txt tests/Test$i/queues.txt tests/Test$i/out.txt >${output} 2>&1
   # with -i :
   #valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./HackEnrollment -i tests/Test$i/students.txt tests/Test$i/courses.txt tests/Test$i/hackers.txt tests/Test$i/queues.txt tests/Test$i/out.txt >${output} 2>&1
   

   # grab exit status of grep (0 if all good ,1 if not)
   
   if ! grep -q "ERROR SUMMARY: 0 errors" "$output"; then
	   echo -e -n "Test number $i result: ${RED}leaked memory${NC}"
   else
	   echo -e -n "Test number $i result: ${GREEN}didn't leak${NC}"
   fi
   
   res=$(diff tests/Test$i/expOut.txt tests/Test$i/out.txt)
   if [[ $res == "" ]]; then
	echo -e " and ${GREEN}passed${NC}"
   else
	echo -e " and ${RED}failed${NC}"
   fi

   rm $output
done