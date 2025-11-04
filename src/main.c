#include <stdio.h>
#include "args_parser.h"
#include <windows.h>

int main(int argc, char *argv[]) {
    GameParams params;

    parse_args(argc, argv, &params);
    print_params(&params);
    
    Sleep(1000);
    return 0;
}