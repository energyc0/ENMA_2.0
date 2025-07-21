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
TESTNAME=binary_expr/test
RED='\033[0;31m'
NC='\033[0m'

set +e
for i in $(seq 1 5)
do
    "${EXECUTABLE}" "${TESTNAME}${i}" &> "${TESTNAME}_out${i}"
    printf "${RED}${TESTNAME}_out${i}:${NC}\n"
    cat "${TESTNAME}_out${i}"
    printf "\n"
done


exit 0