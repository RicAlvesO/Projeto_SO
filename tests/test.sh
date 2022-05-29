#!/bin/bash

function server(){
    sleep 0.5
    read -a words < ./nohup.out
    spid=$((${words[2]}))
    if [ "${words[1]}" == pid: ] ; then
        echo -e "Server Start: Passed!"
    else
        echo -e "Server Start: Failed!"
    fi
}
function close(){
    sleep 0.5
    last = "$(tail -2 ./nohup.out | head -1)"
    expected_tast="$( echo -e "gestor terminou de executar")"
    if [ "${last}" == "${expected_last}" ]; then
        echo "Close: Passed!"
    else
        echo "Close: Failed!"
    fi
}

function help(){
    help="$(./bin/sdstore | tr '\0' '\n')"
    expected_help="$( echo -e "./sdstore status\n#Gives information about the current server status.\n\n./sdstore proc-file <priority> input-filename output-filename transformation-id-1 transformation-id-2 ...\n\n#Submit file prossessing and storage requests acording to the command given.\nThe <priority> flag is an integer and CAN be used to boost the priority of a given command.")"
    if [ "${help}" == "${expected_help}" ]; then
        echo "Help: Passed!"
    else
        echo "Help: Failed!"
    fi
}

function status(){
    stat="$(./bin/sdstore status | tr '\0' '\n')"
    expected_stat="$( echo -e "transf nop: 0/3 (running/max)\ntransf gdecompress: 0/2 (running/max)\ntransf gcompress: 0/2 (running/max)\ntransf encrypt: 0/2 (running/max)\ntransf decrypt: 0/2 (running/max)\ntransf bdecompress: 0/4 (running/max)\ntransf bcompress: 0/4 (running/max)")"
    if [ "${stat}" == "${expected_stat}" ]; then
        echo "Status: Passed!"
    else
        echo "Status: Failed!"
    fi
}

function np(){
    np="$(./bin/sdstore proc-file tests/test_i.txt tests/test_o.txt nop | tr '\0' '\n')"
    expected_np="$( echo -e "processing\nconcluded (bytes-input: 98, bytes-output: 98)")"
    if  [ "${stat}" == "${expected_stat}" ] && cmp --silent -- tests/test_i.txt tests/test_o.txt ; then
        echo "NOP: Passed!"
    else
        echo "NOP: Failed!"
    fi
}

function bcompress(){
    np="$(./bin/sdstore proc-file tests/test_i.txt tests/test_o.txt bcompress | tr '\0' '\n')"
    expected_np="$( echo -e "processing\nconcluded (bytes-input: 98, bytes-output: 111)")"
    if [ "${stat}" == "${expected_stat}" ] && cmp --silent -- tests/test_bc.txt tests/test_o.txt ; then
        echo "BCOMPRESS: Passed!"
    else
        echo "BCOMPRESS: Failed!"
    fi
}

function bdecompress(){
    np="$(./bin/sdstore proc-file tests/test_bc.txt tests/test_o.txt bdecompress | tr '\0' '\n')"
    expected_np="$( echo -e "processing\nconcluded (bytes-input: 111, bytes-output: 98)")"
    if [ "${stat}" == "${expected_stat}" ] && cmp --silent -- tests/test_bdc.txt tests/test_o.txt ; then
        echo "BDECOMPRESS: Passed!"
    else
        echo "BDECOMPRESS: Failed!"
    fi
}

function gcompress(){
    np="$(./bin/sdstore proc-file tests/test_i.txt tests/test_o.txt gcompress | tr '\0' '\n')"
    cat tests/test_o.txt > tests/test_gc.txt
    expected_np="$( echo -e "processing\nconcluded (bytes-input: 98, bytes-output: 98)")"
    if  [ "${stat}" == "${expected_stat}" ] && cmp --silent -- tests/test_gc.txt tests/test_o.txt ; then
        echo "GCOMPRESS: Passed!"
    else
        echo "GCOMPRESS: Failed!"
    fi
}

function gdecompress(){
    np="$(./bin/sdstore proc-file tests/test_gc.txt tests/test_o.txt gdecompress | tr '\0' '\n')"
    expected_np="$( echo -e "processing\nconcluded (bytes-input: 98, bytes-output: 98)")"
    if [ "${stat}" == "${expected_stat}" ] && cmp --silent -- tests/test_gdc.txt tests/test_o.txt ; then
        echo "GDECOMPRESS: Passed!"
    else
        echo "GDECOMPRESS: Failed!"
    fi
}

function encrypt(){
    np="$(./bin/sdstore proc-file tests/test_i.txt tests/test_o.txt encrypt | tr '\0' '\n')"
    expected_np="$( echo -e "processing\nconcluded (bytes-input: 98, bytes-output: 130)")"
    if [ "${stat}" == "${expected_stat}" ] && cmp --silent -- tests/test_e.txt tests/test_o.txt ; then
        echo "ENCRYPT: Passed!"
    else
        echo "ENCRYPT: Failed!"
    fi
}

function decrypt(){
    np="$(./bin/sdstore proc-file tests/test_e.txt tests/test_o.txt decrypt | tr '\0' '\n')"
    expected_np="$( echo -e "processing\nconcluded (bytes-input: 130, bytes-output: 98)")"
    if [ "${stat}" == "${expected_stat}" ] && cmp --silent -- tests/test_de.txt tests/test_o.txt ; then
        echo "DECRYPT: Passed!"
    else
        echo "DECRYPT: Failed!"
    fi
}

echo -e "nop 3\nbcompress 4\nbdecompress 4\ngcompress 2\ngdecompress 2\nencrypt 2\ndecrypt 2" > ./in/test.conf
nohup ./bin/sdstored ./in/test.conf ./libs & > nohup.out
sleep 0.5
echo -e "\nTESTS:"
server
help
status
np
bcompress
bdecompress
gcompress
gdecompress
kill ${spid}
close