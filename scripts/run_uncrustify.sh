#!/bin/bash

find .. -type f -name "*.cpp" -not -path "../build/*" -exec uncrustify -c /home/logi/.config/uncrustify/default_cpp.cfg --no-backup {} \;

find .. -type f -name "*.hpp" -not -path "../build/*" -exec uncrustify -c /home/logi/.config/uncrustify/default_cpp.cfg --no-backup {} \;