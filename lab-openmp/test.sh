#!/bin/bash

for i in 0 1 2 3 4 5; do
    echo "GEOMEAN=$i"
    GEOMEAN=$i ./a.out emacs
    echo ""
done
