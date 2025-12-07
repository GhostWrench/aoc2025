#!/bin/bash

usage() {
    echo ""
    echo "Usage: $0 [optional arguments] [day#]"
    echo ""
    echo "optional arguments"
    echo "------------------"
    echo "  -h|--help        Print help and exit"
    echo "  -e|--example     Run with the example data"
    echo "  -d|--debug       Run the debug version"
    echo "  -v|--valgrind    Run the program under valgrind"
    echo "  -t|--time        Run the program with a timer"
    echo ""
}

cfg="release"
vg=0
time=0
data="input.dat"

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -e|--example)
            data="example.dat"
            shift
            ;;
        -d|--debug)
            cfg="debug"
            shift
            ;;
        -v|--valgrind)
            vg=1
            shift
            ;;
        -t|--time)
            time=1
            shift
            ;;
        *)
            break
            ;;
    esac
done
    
cmd="output/$cfg/day$1 data/day$1/$data"

if [ $time -eq 1 ]; then
    time $cmd
elif [ $vg -eq 1 ]; then
    valgrind --leak-check=yes $cmd
else
    $cmd
fi

exit 0
