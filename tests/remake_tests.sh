# !/bin/bash
# deletes all the tests!!!
# must only be called after significant changes!!!

ANSWER='n'

echo "Do you really want to overwrite answers? (y/n): "
read ANSWER 

if [ "${ANSWER}" != 'y' ]; then
    exit
fi

CUR_DIR="$(realpath "$(dirname $0)")"
cd ${CUR_DIR}
make -C .. clean
make -C ..

EXECUTABLE=../build/release/result
TESTNAME=test
RED='\033[0;31m'
NC='\033[0m'

set +e
for DIR in $(ls -d */)
do

    DIR=${DIR%/}
    echo ===$DIR===
    for FILE in $(find  ${DIR} -name ${TESTNAME}'[0-9]*' | sort)
    do
        NUMBER=${FILE#${DIR}/${TESTNAME}}
        "${EXECUTABLE}" "${FILE}" &> "${DIR}/${TESTNAME}_out${NUMBER}"
        printf "${DIR}/${TESTNAME}_out${NUMBER}:${NC}\n"
        cat "${DIR}/${TESTNAME}_out${NUMBER}"
        printf "\n"
    done
done


exit 0