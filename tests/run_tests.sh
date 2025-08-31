#!/bin/bash
CUR_DIR="$(realpath "$(dirname $0)")"
cd ${CUR_DIR}
make -C .. clean
make -C ..

EXECUTABLE=../build/release/enma
TESTNAME=test

RED='\033[0;31m'
NC='\033[0m'
GREEN='\033[0;32m'

for DIR in $(ls -d */)
do
    DIR=${DIR%/}
    echo ===$DIR===
    for FILE in $(find  ${DIR} -name ${TESTNAME}'[0-9]*' | sort)
    do
        NUMBER=${FILE#${DIR}/${TESTNAME}}
        "${EXECUTABLE}" "${FILE}" &> "${DIR}/${TESTNAME}_temp${NUMBER}"
        printf ${RED}
        if diff "${DIR}/${TESTNAME}_temp${NUMBER}" "${DIR}/${TESTNAME}_out${NUMBER}"; then
            printf "${GREEN}${DIR}/${TESTNAME}${NUMBER} - good\n${NC}"
        else
            printf "${RED}${DIR}/${TESTNAME}${NUMBER} - failed\n${NC}" 
        fi
    done

    rm ${DIR}/${TESTNAME}_temp*
done