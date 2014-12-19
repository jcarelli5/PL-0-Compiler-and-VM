Jon Carelli
December 2014
Simple PL/0 Compiler with function calls and returns
PLEASE SEE WORD DOCUMENT FOR GRAMMAR.

TO COMPILE ON EUSTIS:

gcc compile.c -o compile

/////////////////////////////////////////////////////////////////////////////////////

*** // WINDOWS // ***

'compile' runs the program

COMMANDS:
	-l print the list of lexemes/tokens (scanner output) to the screen.
	-a print the generated assembly code (parser/codegen output) to the screen.
	-v print the virtual machine execution trace (VM output) to the screen. 

EXAMPLES:
compile -l -a -v
compile -l
compile -l -v

/////////////////////////////////////////////////////////////////////////////////////

*** // LINUX // ***

'./compile' runs the program

COMMANDS:
	-l print the list of lexemes/tokens (scanner output) to the screen.
	-a print the generated assembly code (parser/codegen output) to the screen.
	-v print the virtual machine execution trace (VM output) to the screen. 

EXAMPLES:
./compile -l -a -v
./compile -l
./compile -l -v

/////////////////////////////////////////////////////////////////////////////////////

INCLUDED SAMPLE PROGRAM:

var num;
procedure factorial(x);
	begin
		/* Base case */
		if x = 0 then return := 1
		/* Recursive step */
		else return := x * call factorial(x-1);
	end;
begin
	num := call factorial(3);
	write num;
end.


