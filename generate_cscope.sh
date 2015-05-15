#!/bin/sh
find /home/wu/workspace/kernel/linux-2.6.22.with_yaffs20070816/ -name "*.h" -o -name "*.c" -o -name "*.cpp" > cscope.files
cscope -bkq -i cscope.files

