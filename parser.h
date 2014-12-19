/*  AUTHOR: Jon Carelli
    DECEMBER 2014
    SIMPLE PL/0 Compiler
    SEE WORD DOCUMENT FOR GRAMMAR
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOL_TABLE_SIZE 1000
#define EXIT_FAILURE 1

typedef struct symbol {
	int kind; 		// const = 1, var = 2, proc = 3
	char name[12];	// name up to 11 chars
	int val; 		// number (ASCII value)
	int level; 	// L level
	int addr; 		// M address
} symbol;

typedef enum {
    LIT = 1, OPR = 2, LOD = 3,
    STO = 4, CAL = 5, INC = 6,
    JMP = 7, JPC = 8, SIO_1 = 9,
    SIO_2 = 10, SIO_3 = 11
} vm_type;

// Symbol Table for storing symbol structs
symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];
int symbolCounter = 0;

// Array of instructions for the code, and a counter to keep track
instruction code[MAX_SYMBOL_TABLE_SIZE];
int code_size = 0;

// Level global
int level = 0;

char ident[12];

// Array that stores the table of characters
char inputArray[MAX_SYMBOL_TABLE_SIZE][12];

// Global counter to keep track of where we're at in the symbol table.
int globalInputCounter = 0;

// Keeping track of the current level and stack
int stackCounter = 0;

// Global token string.
char token[12];

int parserPrintFlag = 0;

int numParams = 0;

// Keeps track of the address for variables.
int address = 0;

int stack_size = 0;

FILE* input;
FILE* output;

char *getToken(void);
int block ();
int statement ();
int condition ();
int expression ();
int term ();
int factor ();
void GET_TOKEN ();
int VAR_DECL (int space);
int gen (int op, int l, int m);
int find ();
int symbolLevel (int i);
int symbolType (int i);
int symbolAddr (int i);
void code_generation();
int tokenize (char *token);
void ERROR (int type);
int PARAM_BLOCK ();
void PARAM_LIST ();
void updateStackSize (int op, int l, int m);

int parser (int flag) {

    int i;
    int inputCounter = 0;
    char temp[12];
    parserPrintFlag = flag;

    input = fopen("lexemelist.txt", "r");
    output = fopen("mcode.txt", "w");

    // Storing the code in an array
    while (fscanf(input, "%s ", temp) != EOF) {
        strcpy(inputArray[inputCounter], temp);
		inputCounter++;
    }

    // Main procedure.
    GET_TOKEN ();
    block();

    // Checking for period
    if(tokenize(token) != periodsym){
        ERROR (9);
    }

    printf("No errors. Program is syntactically correct.\n");
    // Generate HALT
    //gen(SIO_3, 0, 3);

    code_generation ();

    fclose(input);
    fclose(output);
    system("PAUSE");
}

int block () {
    level++;
    int space = 4;
    int prev_sx = symbolCounter;
    int jmpaddr = gen(JMP, 0, 0);
    // Token = Const
    if (tokenize(token) == constsym) {
        do {
            symbol newConst;
            newConst.kind = 1;

            GET_TOKEN ();

            // ERROR
            if (tokenize(token) != identsym) {
                ERROR (4);
            }

            // Storing the identifier
            strcpy(newConst.name, getToken());
            GET_TOKEN ();

            // ERROR
            if (tokenize(token) != eqlsym) {
                ERROR (3);
            }

            GET_TOKEN ();

            // ERROR
            if (tokenize(token) != numbersym){
                ERROR(2);
            }

            GET_TOKEN ();

            // Converting the token into an integer with atoi and storing the value.
			newConst.val = atoi(token);

            //store the sumbol in the symbol table and increment the counter
			symbol_table[symbolCounter] = newConst;
            symbol_table[symbolCounter].level = level;
			symbolCounter++;

            GET_TOKEN ();

        } while (tokenize(token) == commasym); //run until we meet something that isn't a comma

        // ERROR
        if (tokenize(token) != semicolonsym) {
            ERROR (5);
        }

        GET_TOKEN ();
    }

    // Token = Var
    if (tokenize(token) == varsym){
        space += VAR_DECL (space);
    }

    // Token == Procedure
    while (tokenize(token) == procsym){
        symbol newProc;
        newProc.kind = 3;
        GET_TOKEN ();

        if (tokenize(token) != identsym) {
            ERROR (4);
        }

        // Add the procedure.
        strcpy(newProc.name, getToken());
        newProc.level = level;
        GET_TOKEN ();
        newProc.addr = code_size;
        symbol_table[symbolCounter] = newProc;
        symbolCounter++;
        // Add the return variable.
        strcpy(newProc.name, "return");
        newProc.level = level+1;
        newProc.kind = 2;
        newProc.addr = 0;
        symbol_table[symbolCounter] = newProc;
        symbolCounter++;

        numParams = PARAM_BLOCK ();

        if (tokenize(token) != semicolonsym) {
            ERROR (5);
        }

        GET_TOKEN ();
        block();

        if (tokenize(token) != semicolonsym) {
            ERROR (5);
        }

        GET_TOKEN ();
    }

    code[jmpaddr].m = code_size;
    space = space+numParams;
    gen(INC, 0, space);
    numParams = 0;
    statement ();
    gen(OPR, 0, 0);
    symbolCounter = prev_sx;
    level--;
}


int statement () {

    int token_val;
    // ident
    if(tokenize(token) == identsym){

        GET_TOKEN ();

        // Checking if the identifier is in the symbol table.
        token_val = find();
        if(token_val == -1){
            ERROR (11);
        }

        // Checking if the token is a variable, if constant or procedure, throw error.
        if(symbolType(token_val) != 2){
            ERROR (12);
        }

        GET_TOKEN ();

        // ERROR
        if(tokenize(token)!= becomessym) {
            ERROR (26);
        }

        GET_TOKEN ();
        expression();
        gen(STO, level - symbolLevel(token_val), symbolAddr(token_val));

    }

    // begin
    else if(tokenize(token)== beginsym){

        GET_TOKEN ();
        statement();

        while(tokenize(token) == semicolonsym){
            GET_TOKEN ();
            statement();
        }

        // ERROR, begin must end with end
        if(tokenize(token) != endsym){
            ERROR (27);
        }

        GET_TOKEN ();
    }

    else if(tokenize(token) == ifsym){
        int ctemp;
        char tempToken[12];
        GET_TOKEN ();
        condition();

        // ERROR
        if(tokenize(token) != thensym){
            ERROR (16);
        }

        GET_TOKEN ();
        ctemp = code_size;
        gen(JPC, 0, 0);
        statement();
        code[ctemp].m = code_size;
        strcpy(tempToken, token);
        GET_TOKEN ();

        // If its not an else, decrement token back to the one before
        if (tokenize(token) != elsesym) {
            strcpy(token, tempToken);
            globalInputCounter--;
        }

        // If its an else, handle it
        if (tokenize(token) == elsesym){
            code[ctemp].m += 1;
            ctemp = code_size;
            gen(JMP, 0, 0);
            GET_TOKEN();
            statement ();
            code[ctemp].m = code_size;
        }
    }

    if (tokenize(token) == whilesym){
        int cx1 = code_size;
        GET_TOKEN ();
        condition();
        int cx2 = code_size;
        gen(JPC, 0, 0);

        // ERROR
        if(tokenize(token) != dosym){
            ERROR (18);
        }

        GET_TOKEN ();
        statement();
        gen(JMP, 0, cx1);
        code[cx2].m = code_size;
    }

    if (tokenize(token) == writesym){

        GET_TOKEN ();

        // ERROR
        if(tokenize(token) != identsym){
            ERROR (28);
        }

        GET_TOKEN ();
        token_val = find();

        if(token_val == -1)
            ERROR (11);

        // Checking if the token is a variable, if constant or procedure, throw error.
        if(symbolType(token_val) != 2)
            ERROR (12);

        gen(LOD, level - symbolLevel(token_val), symbol_table[token_val].addr);
        gen(SIO_1, 0, 1);
        GET_TOKEN ();
    }

    if (tokenize(token) == readsym){

        GET_TOKEN ();

        if(tokenize(token) != identsym){
            ERROR (28);
        }

        GET_TOKEN ();
        token_val = find();

        // Checking if token is in the symbol table
        if(token_val == -1)
            ERROR (11);

        // Checking if the token is a variable, if constant or procedure, throw error.
        if(symbolType(token_val) != 2)
            ERROR (12);

        gen(SIO_2, 0, 2);
        gen(STO, level - symbolLevel(token_val), symbol_table[token_val].addr);
        GET_TOKEN ();
    }

    if (tokenize(token) == callsym){

        GET_TOKEN ();

        // ERROR, call must be followed by an identifier
        if(tokenize(token) != identsym){
            ERROR (14);
        }

        GET_TOKEN ();
        token_val = find();

        if(token_val == -1)
            ERROR (11);

        // Checking if the token is a procedure, if constant or variable, throw error.
        if(symbolType(token_val) != 3)
            ERROR (15);

        PARAM_LIST ();
        gen(CAL, level - symbolLevel(token_val), symbolAddr(token_val));
        // Get functional value.
        gen(INC, 0, 1);
        GET_TOKEN ();
    }
}

int condition () {

    if(tokenize(token) == oddsym){
        GET_TOKEN ();
        expression();
        gen(OPR, 0, 6);
    }

    else {

        expression();

        // ERROR
        if ((tokenize(token) != eqlsym) && (tokenize(token) != neqsym) &&
            (tokenize(token) != gtrsym) && (tokenize(token) != lessym) &&
            (tokenize(token) != geqsym) && (tokenize(token) != leqsym)) {

            ERROR(20);
        }

        if(tokenize(token) == eqlsym){
            GET_TOKEN ();
            expression();
            gen(OPR, 0, 8);
        }

        if(tokenize(token) == neqsym){
            GET_TOKEN ();
            expression();
            gen(OPR, 0, 9);
        }

        if(tokenize(token) == gtrsym){
            GET_TOKEN ();
            expression();
            gen(OPR, 0, 12);
        }

        if(tokenize(token) == lessym){
            GET_TOKEN ();
            expression();
            gen(OPR, 0, 10);
        }

        if(tokenize(token) == geqsym){
            GET_TOKEN ();
            expression();
            gen(OPR, 0, 13);
        }

        if(tokenize(token) == leqsym){
            GET_TOKEN ();
            expression();
            gen(OPR, 0, 11);
        }
    }
}

int expression () {

    if ((tokenize(token) == plussym) || (tokenize(token) == minussym)) {

        if (tokenize(token) == plussym)
            gen(OPR, 0, 2);

        else
            gen(OPR, 0, 3);

        GET_TOKEN ();
    }

    term();

    while ((tokenize(token) == plussym) || (tokenize(token) == minussym)){
        if(tokenize(token) == plussym) {
            GET_TOKEN ();
            term();
            gen(OPR, 0, 2);
        }
        else {
            GET_TOKEN ();
            term();
            gen(OPR, 0, 3);
        }
    }
}

int term (){

    factor();

    while ((tokenize(token) == multsym) || (tokenize(token) == slashsym)){
        if (tokenize(token) == multsym) {
            GET_TOKEN ();
            factor ();
            gen(OPR, 0, 4);
        }
        else {
            GET_TOKEN ();
            factor ();
            gen(OPR, 0, 5);
        }
    }
}

int factor () {

    int token_sym;

    if(tokenize(token) == identsym){

        // Find the token in the symbol table.
        GET_TOKEN ();
        token_sym = find ();

        // variable
        if (symbolType(token_sym) == 2){
            gen(LOD, level - symbolLevel(token_sym), symbolAddr(token_sym));
        }

        // constant
        else if (symbolType(token_sym) == 1){
            gen(LIT, 0, symbol_table[token_sym].val);
        }

        // Something went wrong
        else
            ERROR(30);

        // get next token
        GET_TOKEN ();
    }

    else if (tokenize(token) == numbersym) {

        // Generate literal
        GET_TOKEN ();
        gen(LIT, 0, atoi(token));

        // get next token
        GET_TOKEN ();
    }

    else if(tokenize(token) == lparentsym){

        GET_TOKEN ();
        expression();

        // ERROR
        if (tokenize(token) != rparentsym){
            ERROR (22);
        }
        GET_TOKEN ();
    }

    else if(tokenize(token) == callsym){

        GET_TOKEN ();

        // ERROR, call must be followed by an identifier
        if(tokenize(token) != identsym){
            ERROR (14);
        }

        GET_TOKEN ();
        int token_val = find();

        if(token_val == -1)
            ERROR (11);

        // Checking if the token is a procedure, if constant or variable, throw error.
        if(symbolType(token_val) != 3)
            ERROR (15);

        PARAM_LIST ();
        gen(CAL, level - symbolLevel(token_val), symbolAddr(token_val));
        // Get functional value.
        gen(INC, 0, 1);
    }

    // ERROR - identifier, ), or number expected.
    else {
        ERROR (29);
    }
}

char *getToken(void) {

    // Gets the next token from the input table and returns it
    // while incrementing the global counter.
    char temp[12];
    strcpy(temp, inputArray[globalInputCounter]);
    globalInputCounter++;
    return temp;
}

void GET_TOKEN (){
    strcpy(token, getToken());
}

int VAR_DECL (int space) {

        int retVal = 0;
        do {

            // Creating new Var
            symbol newVar;
            newVar.kind = 2;

            // Getting next token
            GET_TOKEN ();

            // ERROR
            if (tokenize(token) != identsym) {
                ERROR (4);
            }

            // Storing the identifier, and creating a new token
            strcpy(newVar.name, getToken());
            newVar.addr = space+retVal;
			address++;
			newVar.level = level;

            //add symbol to symbol table
			symbol_table[symbolCounter] = newVar;
			symbolCounter++;
            GET_TOKEN ();
            retVal++;

        } while (tokenize(token) == commasym);

        if (tokenize(token) != semicolonsym) {
            ERROR (5);
        }

        /*TODO FIX THIS*/
        //gen(INC, 0, address);
        GET_TOKEN ();
        return retVal;
}

int PARAM_BLOCK (int space) {
    int addr = 4;
    int retVal = 0;
    if (tokenize(token) != lparentsym) ERROR (31);
    GET_TOKEN ();

    if (tokenize(token) == identsym) {

        // Add a new parameter to the symbol table.
        symbol newVar;
        newVar.kind = 2;
        strcpy(newVar.name, getToken());
        newVar.level = level+1;
        newVar.addr = addr;
        addr++;
        symbol_table[symbolCounter] = newVar;
        symbolCounter++;
        GET_TOKEN ();
        retVal++;

        while (tokenize(token) == commasym) {
            GET_TOKEN ();

            if (tokenize(token) != identsym) {
                ERROR (35);
            }
            // Add new parameter to symbol table.
            symbol newVar;
            newVar.kind = 2;
            strcpy(newVar.name, getToken());
            newVar.level = level+1;
            newVar.addr = addr;
            addr++;
            symbol_table[symbolCounter] = newVar;
            symbolCounter++;
            GET_TOKEN ();
            retVal++;

        }
    }
    if (tokenize(token) != rparentsym) ERROR (32);
    GET_TOKEN ();
    return retVal;
}

void PARAM_LIST () {
    GET_TOKEN ();
    int params = 0;

    if (tokenize(token) != lparentsym) ERROR (33);

    GET_TOKEN ();

    if (tokenize(token) != rparentsym) {

        expression ();
        params++;
    }

    while (tokenize(token) == commasym) {

        GET_TOKEN ();
        expression ();
        params++;
    }

    while (params > 0) { // Save results into param slots

        gen(STO, 0, stack_size+4-1);
        params--;

    }

    if (tokenize(token) != rparentsym) ERROR (34);

    GET_TOKEN ();

}

void updateStackSize (int op, int l, int m) {

    if ((op == LIT) || (op == LOD)) stack_size++;
    else if ((op == SIO_1)) stack_size--;
    else if ((op == SIO_2)) stack_size++;
    else if ((op == STO) || (op == JPC)) stack_size--;
    else if ((op == INC)) stack_size += m;
    else if ((op == OPR)){
        if (m == 0) { stack_size = 0; }
        else if ((m != 1) || (m != 6)) { stack_size--; }
    }
}

void code_generation () {
    int i;
    if (parserPrintFlag == 1){
        printf("////////////////////////\n");
        printf("Generated Assembly Code: \n");
        printf("////////////////////////\n\n");
    }
    for (i = 0; i < code_size; i++){
        fprintf(output, "%d %d %d\n", code[i].op, code[i].l, code[i].m);
        if (parserPrintFlag == 1){
            printf("%d %d %d\n", code[i].op, code[i].l, code[i].m);
        }
    }
    if (parserPrintFlag == 1){
        printf("\n");
    }
}

int gen(int op, int l, int m){

    int pos = code_size;
    code[code_size].op = op;
    code[code_size].l = l;
    code[code_size].m = m;
    updateStackSize(op, l, m);
    code_size++;
    return pos;
}

// Returns the symbol type(kind) const = 1, var = 2
int symbolType (int i){
    return symbol_table[i].kind;
}

// Returns the symbol level
int symbolLevel (int i){
    return symbol_table[i].level;
}

// Returns the symbol address
int symbolAddr (int i){
    return symbol_table[i].addr;
}

// Finds the location of the identifier in the symbol table.
// If there its not in the symbol table, then it returns -1
int find(){
    int i;
    for(i=symbolCounter-1; i >= 0; i--){
        if(strcmp(token, symbol_table[i].name) == 0){
            return i;
        }
    }
    return -1;
}

// This function turns token strings into integers, mostly for readability purposes.
int tokenize (char *token){

    if(strcmp(token, "1") == 0){
        return nulsym;
    }
    // ident
    else if(strcmp(token, "2") == 0){
        return identsym;
    }
    // number
    else if(strcmp(token, "3") == 0){
        return numbersym;
    }
    // plus
    else if(strcmp(token, "4") == 0){
        return plussym;
    }
    // minus
    else if(strcmp(token, "5") == 0){
        return minussym;
    }
    // mult
    else if(strcmp(token, "6") == 0){
        return multsym;
    }
    // slash
    else if(strcmp(token, "7") == 0){
        return slashsym;
    }
    // odd
    else if(strcmp(token, "8") == 0){
        return oddsym;
    }
    // eql
    else if(strcmp(token, "9") == 0){
        return eqlsym;
    }
    // neq
    else if(strcmp(token, "10") == 0){
        return neqsym;
    }
    // less
    else if(strcmp(token, "11") == 0){
        return lessym;
    }
    // leq
    else if(strcmp(token, "12") == 0){
        return leqsym;
    }
    // gtr
    else if(strcmp(token, "13") == 0){
        return gtrsym;
    }
    // geq
    else if(strcmp(token, "14") == 0){
        return geqsym;
    }
    // lparent
    else if(strcmp(token, "15") == 0){
        return lparentsym;
    }
    // rparent
    else if(strcmp(token, "16") == 0){
        return rparentsym;
    }
    // comma
    else if(strcmp(token, "17") == 0){
        return commasym;
    }
    // semicolon
    else if(strcmp(token, "18") == 0){
        return semicolonsym;
    }
    // period
    else if(strcmp(token, "19") == 0){
        return periodsym;
    }
    // becomes
    else if(strcmp(token, "20") == 0){
        return becomessym;
    }
    // begin
    else if(strcmp(token, "21") == 0){
        return beginsym;
    }
    // end
    else if(strcmp(token, "22") == 0){
        return endsym;
    }
    // if
    else if(strcmp(token, "23") == 0){
        return ifsym;
    }
    // then
    else if(strcmp(token, "24") == 0){
        return thensym;
    }
    // while
    else if(strcmp(token, "25") == 0){
        return whilesym;
    }
    // do
    else if(strcmp(token, "26") == 0){
        return dosym;
    }
    // call
    else if(strcmp(token, "27") == 0){
        return callsym;
    }
    // const
    else if(strcmp(token, "28") == 0){
        return constsym;
    }
    // var
    else if(strcmp(token, "29") == 0){
        return varsym;
    }
    // proc
    else if(strcmp(token, "30") == 0){
        return procsym;
    }
    // write
    else if(strcmp(token, "31") == 0){
        return writesym;
    }
    // read
    else if(strcmp(token, "32") == 0){
        return readsym;
    }
    // else
    else if(strcmp(token, "33") == 0){
        return elsesym;
    }

    return 0;
}

// ERROR function
void ERROR (int type) {

    switch(type){

        case 1:
            printf("Use = instead of :=. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 2:
            printf("= must be followed by a number. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 3:
            printf("Identifier must be followed by =. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 4:
            printf("const, var, procedure must be followed by an identifier. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 5:
            printf("Semicolon or comma missing. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 6:
            printf("Semicolon or comma missing. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 7:
            printf("Statement expected. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 8:
            printf("Incorrect symbol after statement part in block. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 9:
            printf("Period expected. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 10:
            printf("Semicolon between statements missing. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 11:
            printf("Undeclared identifier. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 12:
            printf("Assignment to constant or procedure is not allowed. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 13:
            printf("Assignment operator expected. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 14:
            printf("call must be followed by an identifier. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 15:
            printf("Call of a constant or variable is meaningless. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 16:
            printf("then expected. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 17:
            printf("Semicolon or } expected. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 18:
            printf("do expected. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 19:
            printf("Incorrect symbol following statement. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 20:
            printf("Relational operator expected. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 21:
            printf("Expression must not contain a procedure identifier. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 22:
            printf("Right parenthesis missing. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 23:
            printf("The preceding factor cannot begin with this symbol. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 24:
            printf("An expression cannot begin with this symbol. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 25:
            printf("This number is too large. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 26:
            printf(":= missing in statement.\n");

			system("PAUSE");
            exit(EXIT_FAILURE);

        case 27:
            printf("begin must be closed with end.\n");

			system("PAUSE");
            exit(EXIT_FAILURE);

        case 28:
            printf("missing identifier.\n");

			system("PAUSE");
            exit(EXIT_FAILURE);

        case 29:
            printf("identifier, (, or number expected. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 30:
            printf("Variable or Constant missing. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 31:
            printf("Procedures must have parameters. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 32:
            printf("Bad procedure declaration. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 33:
            printf("Missing parameter list at call. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 34:
            printf("Bad calling formatting. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

        case 35:
            printf("Parameter missing. \n");

            system("PAUSE");
            exit(EXIT_FAILURE);

    }
}
