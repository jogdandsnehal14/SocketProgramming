#!/bin/sh
g++ -std=c++14 -o client client.cpp
g++ -std=c++14 -o server server.cpp -lpthread

chmod 700 client
chmod 700 server
