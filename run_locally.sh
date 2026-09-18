#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

BINARY_HOME=${BINARY_HOME:-${SCRIPT_DIR}/.build}
INPUT_HOME=${INPUT_HOME:-${SCRIPT_DIR}/input}
INPUT=${INPUT_HOME}/AB_NYC_2019.csv
OUTPUT=${OUTPUT:-${SCRIPT_DIR}/output}

if [ ! -x "${BINARY_HOME}/mapper" ] || [ ! -x "${BINARY_HOME}/reducer" ]; then
    echo "mapper/reducer not found in ${BINARY_HOME}, build them first: ./self_check.sh" >&2
    exit 1
fi

cat "${INPUT}" | "${BINARY_HOME}/mapper" | sort -k1 | "${BINARY_HOME}/reducer" > "${OUTPUT}"
cat "${OUTPUT}"
