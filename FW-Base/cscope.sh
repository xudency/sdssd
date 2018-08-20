#!/bin/bash

find . -name "*.h" -o -name "*.c" -o -name "*.cc" > cscope.files

cscope -Rbkq -i cscope.files

ctags -R   
