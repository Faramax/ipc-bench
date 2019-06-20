#!/bin/bash

echo domain
./domain/domain $*

echo fifo
./fifo/fifo $*

echo mmap
./mmap/mmap $*

echo mq
./mq/mq $*

echo pipe
./pipe/pipe $*

echo shm
./shm/shm $*

echo signal
./signal/signal $*

echo tcp
./tcp/tcp $*
