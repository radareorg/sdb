#!/bin/sh
valgrind $@ 2>&1 | grep "in use"|cut -d : -f2 | awk '{print "'$2' "$1}' 
