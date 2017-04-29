#! /bin/sh
#
# trace.sh
# Copyright (C) 2017 Juan Francisco Martínez <juan.martinez[AT]bsc[dot]es>
#
# Distributed under terms of the MIT license.
#

ELF_LIB=/home/jmartinez/Repositorios/libelf/lib
DPC_HOME=/home/jmartinez/Repositorios/apa-prefetcher
PIN_HOME=/home/jmartinez/Packages/pin-2.13-65163-gcc.4.4.7-linux

PINTOOL_HOME=${DPC_HOME}/pintool/

N_WARMUP_INSTR=100
N_TRACE_INSTR=200000

PIN_ARGS="-ifeellucky"
PINTOOL_ARGS=" -o $2 -s ${N_WARMUP_INSTR} -t ${N_TRACE_INSTR}"

echo "*** Tracing $1 ***"
echo "PIN_HOME=${PIN_HOME}"

LD_LIBRARY_PATH=${ELF_LIB}:${LD_LIBRARY_PATH}\
    ${PIN_HOME}/pin ${PIN_ARGS}\
    -t ${PINTOOL_HOME}/dpc2_tracer.so ${PINTOOL_ARGS} -- $1

echo "*** Done ***"
