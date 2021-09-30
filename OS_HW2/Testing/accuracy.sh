#!/bin/bash

STUDENT_ID=$1
YOUR_OUTPUT_DIR=$2
ANSWER_DIR="answer"
TESTCASE_DIR="testcase"
OUTPUT_DIR="output"
total_errors=0
total_frames=3000
file_num=5

if [ -z ${STUDENT_ID} ] || [ -z ${YOUR_OUTPUT_DIR} ]; then
  echo "Usage: $0 <student id> <your output path>"
  exit 1
fi

if [ -d ${STUDENT_ID} ]; then
  rm -rf ${STUDENT_ID}
fi

mkdir ${STUDENT_ID} && mkdir ${STUDENT_ID}/${OUTPUT_DIR}
cp -r ${ANSWER_DIR} ${STUDENT_ID}/
mv ${YOUR_OUTPUT_DIR}/0[1-${file_num}].txt ./${STUDENT_ID}/${OUTPUT_DIR}/

if [ $? -ne 0 ]; then
    exit 1
fi

cd ${STUDENT_ID}

for i in $( seq  ${file_num} ) 
do
    errors=$( diff -y --suppress-common-lines ${ANSWER_DIR}/0${i}.txt \
            ${OUTPUT_DIR}/0${i}.txt | wc -l )
    total_errors=$(( ${total_errors} + ${errors} ))
    if [ ${errors} -ne 0 ]; then
        echo -e "0${i}.mp4: \033[1;31m$errors errors found!\033[m"
    else
        echo -e "0${i}.mp4: \033[1;32mAll correct!\033[m"
    fi
done

accuracy=$( awk "BEGIN{ correct_num = (${total_frames} - ${total_errors}); \
            accuracy = (correct_num / ${total_frames}); print accuracy; exit}" )
criterion=$( echo ${accuracy} '>=' 0.5 | bc -l )

if [  ${criterion} -eq 1 ]; then
    echo -e "\033[1;32mAccuracy: ${accuracy}\033[m"
else
    echo -e "\033[1;31mAccuracy: ${accuracy}\033[m"
fi
