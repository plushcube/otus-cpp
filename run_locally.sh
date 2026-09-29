#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

BINARY_HOME=${BINARY_HOME:-${SCRIPT_DIR}/.build}
INPUT_HOME=${INPUT_HOME:-${SCRIPT_DIR}/input}
INPUT=${INPUT_HOME}/AB_NYC_2019.csv
OUTPUT=${OUTPUT:-${SCRIPT_DIR}/output}

for binary in mapper reducer_mean reducer_variance; do
    if [ ! -x "${BINARY_HOME}/${binary}" ]; then
        echo "${binary} not found in ${BINARY_HOME}, build it first: ./self_check.sh" >&2
        exit 1
    fi
done

cat "${INPUT}" | "${BINARY_HOME}/mapper" | sort -k1 > "${OUTPUT}"

printf 'mean price: %s\n' "$("${BINARY_HOME}/reducer_mean" < "${OUTPUT}")"
printf 'price variance: %s\n' "$("${BINARY_HOME}/reducer_variance" < "${OUTPUT}")"
