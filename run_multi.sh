#!/bin/bash

START=55001
END=56000
BIN=./build/release/Server/tcp_server

for ((port=START; port<=END; port++)); do
    echo "Starting tcp_server on port $port"
    $BIN $port &
done

wait
