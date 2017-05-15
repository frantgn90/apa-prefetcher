#! /bin/sh
#
# simall.sh
# Copyright (C) 2017 Juan Francisco Martínez <juan.martinez[AT]bsc[dot]es>
#
# Distributed under terms of the MIT license.
#

TRACES_HOME="./traces"
SIM_BIN=spp_prefecher/dpc2sim_spp
SIM_FLAGS=("-small_llc" "-low_bandwidth" "-scramble_loads")

for trace in $TRACES_HOME/*.gz
do
    echo
    echo
    echo "########## Simulating $trace ##########"
    echo "=============== With default conf ==============="
    echo "zcat $trace | ./$SIM_BIN -hide_heartbeat"
    echo 
    zcat $trace | ./$SIM_BIN -hide_heartbeat

    for config in ${SIM_FLAGS[@]}
    do
        echo "=============== With conf $config ==============="
        echo "zcat $trace | ./$SIM_BIN -hide_heartbeat $config"
        echo
        zcat $trace | ./$SIM_BIN -hide_heartbeat $config
    done
    echo "########## Done ##########"
done
