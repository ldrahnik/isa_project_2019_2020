#!/bin/bash

# Name: Drahník Lukáš
# Project: ISA: DNS resolver
# Date: 14.11.2019
# Email: <xdrahn00@stud.fit.vutbr.cz>
# File: tests.sh

TEST_DIRECTORY=`dirname $0`
PORT=1024

# 1 (A type)
$TEST_DIRECTORY/server/server -e -p $PORT > $TEST_DIRECTORY/logs/server_log &
$TEST_DIRECTORY/../dns -s 127.0.0.1 -p $PORT -t 1 www.fit.vutbr.cz > /dev/null 2>&1
if diff $TEST_DIRECTORY/logs/server_log $TEST_DIRECTORY/ref/1 >/dev/null; then
    echo "*******TEST 1 PASSED";
else
    echo "TEST 1 FAILED";
fi

# 1.1 (server as hostname)
$TEST_DIRECTORY/server/server -e -p $PORT > $TEST_DIRECTORY/logs/server_log &
$TEST_DIRECTORY/../dns -s localhost -p $PORT www.fit.vutbr.cz > /dev/null 2>&1
if diff $TEST_DIRECTORY/logs/server_log $TEST_DIRECTORY/ref/1 >/dev/null; then
    echo "*******TEST 1.1 PASSED";
else
    echo "TEST 1.1 FAILED";
fi

# 2 (recursion)
$TEST_DIRECTORY/server/server -e -p $PORT > $TEST_DIRECTORY/logs/server_log &
$TEST_DIRECTORY/../dns -r -s localhost -p $PORT  -t 1 www.fit.vutbr.cz > /dev/null 2>&1
if diff $TEST_DIRECTORY/logs/server_log $TEST_DIRECTORY/ref/2 >/dev/null; then
    echo "*******TEST 2 PASSED";
else
    echo "TEST 2 FAILED";
fi

# 3 (AAAA type)
$TEST_DIRECTORY/server/server -e -p $PORT > $TEST_DIRECTORY/logs/server_log &
$TEST_DIRECTORY/../dns -6 -r -s localhost -p $PORT  -t 1 www.fit.vutbr.cz > /dev/null 2>&1
diff $TEST_DIRECTORY/logs/server_log $TEST_DIRECTORY/ref/3
if diff $TEST_DIRECTORY/logs/server_log $TEST_DIRECTORY/ref/3 >/dev/null; then
    echo "*******TEST 3 PASSED";
else
    echo "TEST 3 FAILED";
fi

# 4 (PTR type, IPv4)
$TEST_DIRECTORY/server/server -e -p $PORT > $TEST_DIRECTORY/logs/server_log &
$TEST_DIRECTORY/../dns -x -r -s localhost -p $PORT -t 1 147.229.9.23 > /dev/null 2>&1
if diff $TEST_DIRECTORY/logs/server_log $TEST_DIRECTORY/ref/4 >/dev/null; then
    echo "*******TEST 4 PASSED";
else
    echo "TEST 4 FAILED";
fi

# 5 (PTR type, IPv6)
$TEST_DIRECTORY/server/server -e -p $PORT > $TEST_DIRECTORY/logs/server_log &
$TEST_DIRECTORY/../dns -x -r -s localhost -p $PORT  -t 1 4321:0:1:2:3:4:567:89ab > /dev/null 2>&1
if diff $TEST_DIRECTORY/logs/server_log $TEST_DIRECTORY/ref/5 >/dev/null; then
    echo "*******TEST 5 PASSED";
else
    echo "TEST 5 FAILED";
fi  
