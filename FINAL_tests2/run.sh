#!/bin/bash

# num=1
# var=$( cat ../examples/example${num}_command )
# echo $var
# ./cacheSim "../examples/example${num}_trace" ${var} > 1.txt
# gdb --args ./cacheSim "../examples/example${num}_trace" ${var}

BLACK=`tput setaf 0`
RED=`tput setaf 1`
GREEN=`tput setaf 2`
YELLOW=`tput setaf 3`
BLUE=`tput setaf 4`
MAGENTA=`tput setaf 5`
CYAN=`tput setaf 6`
WHITE=`tput setaf 7`

    echo "${CYAN}This Year Segel Tests: "
for num in {1..3}
do
    var=$( cat ./examples/example${num}_command )
    ./cacheSim "./examples/example${num}_trace" ${var} > ${num}.txt
    #valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cacheSim "./examples/example${num}_trace" ${var} > ${num}leak.txt 2>&1
    diff ${num}.txt ./examples/example${num}_output
    if cmp ${num}.txt ./examples/example${num}_output;
        then
    echo "${GREEN}test ${num} passed";
        else
        echo "${RED}test ${num} failed";
    fi
done

echo
echo
    echo "${CYAN}Last Year Segel Tests: "
for num in {1..20}
do
    var=$( cat ./test2/example${num}_command )
    ./cacheSim "./test2/example${num}_trace" ${var} > ${num}.txt
    #valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cacheSim "./test2/example${num}_trace" ${var} > ${num}leak.txt 2>&1
    diff ${num}.txt ./test2/example${num}_output
    if cmp ${num}.txt ./test2/example${num}_output;
    then
    echo "${GREEN}test ${num} passed";
    else
        echo "${RED}test ${num} failed";
    fi
done
echo
echo
    echo "${CYAN}Nimrod Miss Tests: "
for num in {0..99}
do
    var=$( cat ./last/example${num}_command )
    ./cacheSim "./last/miss_example${num}_trace" ${var} > my_miss${num}.txt
   # valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cacheSim "./test2/example${num}_trace" ${var} > ${num}leak.txt 2>&1
    diff my_miss${num}.txt ./last/miss${num}.txt
    if cmp my_miss${num}.txt ./last/miss${num}.txt;
    then
    echo "${GREEN}test ${num} passed";
    else
        echo "${RED}test ${num} failed";
    fi
done

    echo "${CYAN}Nimrod Hit Tests: "
for num in {0..99}
do
    var=$( cat ./last/example${num}_command )
    ./cacheSim "./last/hit_example${num}_trace" ${var} > my_hit${num}.txt
   # valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./cacheSim "./test2/example${num}_trace" ${var} > ${num}leak.txt 2>&1
    diff my_hit${num}.txt ./last/hit${num}.txt
    if cmp my_hit${num}.txt ./last/hit${num}.txt;
    then
    echo "${GREEN}test ${num} passed";
    else
        echo "${RED}test ${num} failed";
    fi
done

