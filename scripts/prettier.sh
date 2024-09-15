#!/bin/bash

find .. -name "*.cpp" -o -name "*.hpp" | uncrustify -c /home/logi/.config/uncrustify/default_cpp.cfg -F - --no-backup
