#!/bin/bash
CUR_DIR="$(realpath "$(dirname $0)")"
cd ${CUR_DIR}
make -C .. clean
make -C ..

EXECUTABLE=../build/release/result
TESTNAME=binary_expr/test

RED='\033[0;31m'
NC='\033[0m'
GREEN='\033[0;32m'

for i in $(seq 1 5)
do
    "${EXECUTABLE}" "${TESTNAME}${i}" &> "${TESTNAME}_temp${i}"
    printf ${RED}
    if diff "${TESTNAME}_temp${i}" "${TESTNAME}_out${i}"; then
        printf "${GREEN}${TESTNAME}${i} - good\n${NC}"
    else
        printf "${RED}${TESTNAME}${i} - failed\n${NC}" 
    fi
done

rm ${TESTNAME}_temp*