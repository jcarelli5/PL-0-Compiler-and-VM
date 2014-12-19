/*  AUTHOR: Jon Carelli
    DECEMBER 2014
    SIMPLE PL/0 Compiler
    SEE WORD DOCUMENT FOR GRAMMAR
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACK_HEIGHT 5000
#define MAX_CODE_LENGTH 5000
#define MAX_LEXI_LEVELS 3

typedef struct {
    int op;
    int l;
    int m;
} instruction;

typedef enum {
    FALSE,
    TRUE
} boolean;

FILE* input;
FILE* output;

int SP = 0;
int BP = 1;
int PC = 0;
int IR = 0;

int AR[100];
int numAR = 0;
int stack [MAX_STACK_HEIGHT];
int maxStack = 0;
int HALT = TRUE;
int vmPrintFlag = 0;


void printAssembly (instruction a[], int l);
void fetchAndExecute(instruction i);
void opPrint(instruction i);
void oprFunct(int L, int M);
int base(int l, int base);
void printStack();

int vm (int flag) {

    vmPrintFlag = flag;

    memset(stack, 0, sizeof(stack));
    int nInstruction = 0;
    instruction instructions[MAX_CODE_LENGTH];

    // File Management
    input = fopen("mcode.txt", "r");
    output = fopen("stacktrace.txt", "w");

    // Reading input, storing variables into structs.
    while (fscanf(input, "%d %d %d", &instructions[nInstruction].op,
                  &instructions[nInstruction].l, &instructions[nInstruction].m) != EOF) {
        nInstruction++;
    }
    // Printing the Assembly
    printAssembly(instructions, nInstruction);

    // Formatting
    fprintf(output,"\t\t\t\tpc\tbp\tsp\tstack\n");
    fprintf(output,"Initial value\t\t\t%d\t%d\t%d\t\n", PC, BP, SP);

    // Fetching and Executing each operation
    do {
        fetchAndExecute(instructions[PC]);
        printStack();
        fprintf(output, "\n");
    } while (BP != 0 && HALT);

    fclose(output);
    fclose(input);

    // Printing to console
    if (vmPrintFlag == 1) {
        int c;
        input = fopen("stacktrace.txt", "r");
        printf("////////////////////////\n");
        printf("Stack Trace: \n");
        printf("////////////////////////\n\n");
        if (input) {
            while ((c = getc(input)) != EOF)
                putchar(c);
            fclose(input);
        }
        printf("\n\n");
    }

    return 0;
}

// Prints the original assembly
void printAssembly (instruction a[], int L) {

	int i;
	// Formatting
    fprintf(output, "Line\tOP\tL\tM\t\n");

    for (i=0; i< L; i++) {

        fprintf(output, "%d\t", i);

        switch (a[i].op) {
            case (1):
                fprintf(output, "lit");
                break;

            case (2):
                fprintf(output, "opr");
                break;

            case (3):
                fprintf(output, "lod");
                break;

            case (4):
                fprintf(output, "sto");
                break;

            case (5):
                fprintf(output, "cal");
                break;

            case (6):
                fprintf(output, "inc");
                break;

            case (7):
                fprintf(output, "jmp");
                break;

            case (8):
                fprintf(output, "jpc");
                break;

            case (9):
                fprintf(output, "sio");
                break;

            case (10):
                fprintf(output, "sio");
                break;

            case (11):
                fprintf(output, "sio");
                break;
        }

        // Formatting
        fprintf(output,"\t%d\t%d\n", a[i].l, a[i].m);
    }
}

// Fetch and Execute
void fetchAndExecute(instruction i) {
    opPrint(i);
    PC++;
    switch (i.op) {

        // LIT
        case (1):
            SP++;
            stack[SP] = i.m;
            break;

        // OPR
        case (2):
            oprFunct(0, i.m);
            break;

        // LOD
        case (3):
            SP++;
            stack[SP] = stack[base(i.l, BP) + i.m];
            break;

        // STO
        case (4):
            stack[base(i.l, BP) + i.m] = stack[SP];
            SP--;
            break;

        // CAL
        case (5):
            stack[SP + 1] = 0;
            stack[SP + 2] = base(i.l, BP);
            stack[SP + 3] = BP;
            stack[SP + 4] = PC;
            BP = (SP + 1);
            PC = i.m;

            AR[numAR] = BP;
            numAR++;

            break;

        // INC
        case (6):
            SP = SP + i.m;
            break;

        // JMP
        case (7):
            PC = i.m;
            break;

        // JPC
        case (8):
            if (stack[SP] == 0)
                PC = i.m;
            SP--;
            break;

        // SIO 0, 1
        case (9):
            printf("Top of Stack: %d\n", stack[SP]);
            SP--;
            break;

        // SIO 0, 2
        case (10):
            SP++;
            scanf("%d", &stack[SP]);
            break;

        // SIO 0, 3
        case (11):
            HALT = FALSE;
            break;
    }
}

// Function to aid in printing
void opPrint(instruction i) {
    fprintf(output, "%d\t", PC);

    switch (i.op) {

        // LIT
        case (1):
            fprintf(output, "lit");
            break;

        // OPR
        case (2):
            fprintf(output, "opr");
            break;

        // LOD
        case (3):
            fprintf(output, "lod");
            break;

        // STO
        case (4):
            fprintf(output, "sto");
            break;

        // CAL
        case (5):
            fprintf(output, "cal");
            break;

        // INC
        case (6):
            fprintf(output, "inc");
            break;

        // JMP
        case (7):
            fprintf(output, "jmp");
            break;

        // JPC
        case (8):
            fprintf(output, "jpc");
            break;

        // SIO 0, 1
        case (9):
            fprintf(output, "sio");
            break;

        // SIO 0, 2
        case (10):
            fprintf(output, "sio");
            break;

        // SIO 0, 3
        case (11):
            fprintf(output, "sio");
            break;
    }

    fprintf(output,"\t%d\t%d\t", i.l, i.m);

}

void oprFunct(int L, int M){

    switch(M) {
        // RETURN
        case (0):
            SP = BP - 1;
            PC = stack[SP + 4];
            BP = stack[SP + 3];
            break;

        // NEGATIVE
        case (1):
            stack[SP] = -(stack[SP]);
            break;

        // ADD
        case (2):
            SP = SP - 1;
            stack[SP] = stack[SP] + stack[SP + 1];
            break;

        // SUBTRACTION
        case (3):
            SP = SP - 1;
            stack[SP] = stack[SP] - stack[SP + 1];
            break;

        // MULTIPLY
        case (4):
            SP = SP - 1;
            stack[SP] = stack[SP] * stack[SP + 1];
            break;

        // DIVIDE
        case (5):
            SP = SP - 1;
            stack[SP] = stack[SP] / stack[SP + 1];
            break;

        // ODD
        case (6):
            stack[SP] = stack[SP] % 2;
            break;

        // MODULAS
        case (7):
            SP = SP - 1;
            stack[SP] = stack[SP] % stack[SP + 1];
            break;

        // EQUAL
        case (8):
            SP = SP - 1;
            stack[SP] = (stack[SP] == stack[SP + 1]);
            break;

        // NOT EQUAL
        case (9):
            SP = SP - 1;
            stack[SP] = (stack[SP] != stack[SP + 1]);
            break;

        // LESS THAN
        case (10):
            SP = SP - 1;
            stack[SP] = (stack[SP] < stack[SP + 1]);
            break;

        // LESS THAN OR EQUAL
        case (11):
            SP = SP - 1;
            stack[SP] = (stack[SP] <= stack[SP + 1]);
            break;

        // GREATER THAN
        case (12):
            SP = SP - 1;
            stack[SP] = (stack[SP] > stack[SP + 1]);
            break;

        // GREATER THAN OR EQUAL
        case (13):
            SP = SP - 1;
            stack[SP] = (stack[SP] >= stack[SP + 1]);
            break;
    }
}

// Given base function
int base(int l, int base) {
    int b1; //find base L levels down
    b1 = base;
    while (l > 0) {
        b1 = stack[b1 + 1];
        l--;
    }
    return b1;
}

// This function prints the stack for final output.
void printStack() {

    fprintf(output, "%d\t%d\t%d\t", PC, BP, SP);

    if (BP == 0)
        return;

    else {
        int i;
        for (i = 1; i <= SP; i++) {
            int j;
            for (j = 0; j < numAR; j++) {

                // Printing next AR
                if (AR[j] == i) {
                    fprintf(output, "| ");
                }
            }
            fprintf(output, "%d ", stack[i]);
        }
    }
}
