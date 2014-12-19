/*  AUTHOR: Jon Carelli
    DECEMBER 2014
    SIMPLE PL/0 Compiler
    SEE WORD DOCUMENT FOR GRAMMAR
*/

#include <stdio.h>
#include "vm.h"
#include "scanner.h"
#include "parser.h"

int main (int argc, char *argv[]) {
    int scannerFlag = 0;
    int parserFlag = 0;
    int vmFlag = 0;

    int i;
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0)
            scannerFlag = 1;

        if (strcmp(argv[i], "-a") == 0)
            parserFlag = 1;

        if (strcmp(argv[i], "-v") == 0)
            vmFlag = 1;
    }

    scanner(scannerFlag);
    parser (parserFlag);
    vm (vmFlag);

/*  -l  prints the list of lexemes/tokens (scanner output) to the screen.
	-a  prints the generated assembly code (parser/codegen output) to the screen.
	-v  prints the virtual machine execution trace (VM output) to the screen.
*/
}
