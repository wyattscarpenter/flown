#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>

char debug = 0;
#define dprintf(...) (debug&&fprintf(stderr, __VA_ARGS__))
//note: no metaflown support yet. Maybe abandon that and go with c preprocessor?

//The limiting factor here is the Windows C compiler, cl. Cl helpfully tells us: "total size of array must not exceed 0x7fffffff bytes" (which happens to be INT_MAX)
//gcc is fine with UINT_MAX length (not bytes, number of elements). Exceeding that: "error: size of array ‘array’ is too large"
//and TCC works at some point lower than INT_MAX but higher than INT_MAX/2, (unclear if bytes or length) so we have chosen to ignore it here. "error: internal error: relocation failed"
unsigned char tape[INT_MAX];

unsigned char instructions[INT_MAX/4];

unsigned int instruction_arguments[INT_MAX/8];

int main(int argc, char **argv){
  //printf("u: %u d: %d (unsigned char)u: %u c: %c\n", EOF, EOF, (unsigned char)EOF, EOF); //good for remembering what EOF is (-1).
  //printf("%x INT_MAX, %x sizeof(tape)\n", INT_MAX, sizeof(tape)); //confirm these are the same
  //tape[INT_MAX/4-1] = 0xFF; //uh oh, this is a linker error for me on gcc
  if(argc!=2){
    puts("USAGE: input | flown script.fln | output");
    return -1; //bad arguments
  }
  dprintf("%s", "read the file\n");
  FILE *file = fopen(argv[1],"r");
  if(!file){
     puts("input file");
     puts(argv[1]);
     puts("can't be opened for reading, probably doesn't exist.");
     return -2;
  }
  dprintf("%s\n", "parse");
  unsigned char c;
  enum parsing_state {index, instruction, argument};
  enum parsing_state state = index;
  int index_buffer = 0;
  char new_word = 1;
  char header_done = 0;
  while((c=fgetc(file))!= (unsigned char)EOF){
    if(!header_done&&(c<48||c>57)){continue;} else {header_done=1;} //to avoid unicode BOMs and shebangs, we now ignore anything but 0-9 until the first 0-9.
    resume:
    //dprintf("\n\n c = %d, index_buffer %u", c, index_buffer);
    if(c=='\n') { //this is a very heavily line-oriented parser now I guess.
      state = index;
      index_buffer = 0;
      new_word = 1;
    } else if(c==' '){
      if(!new_word){
        state = ((int)state + 1 ) % 3;
        new_word = 1;
      }
    } else {
      if(state==index){
        index_buffer = (index_buffer*=10) + (c-48); //technically, you _can_ have non-number labels... they'll be converted to numbers though!
      }
      if(state==instruction){ 
        if (new_word){
          //dprintf("\n\n index_buffer %u", index_buffer);
          instructions[index_buffer] = fgetc(file); //this is all we need. The second character is the unilluminating N U E I F or O.
        }
      }
      if (state==argument) {
        if (new_word && (c=='b' || c=='e' || c=='n' || c=='s')) {//could be a special character
          char c2 = fgetc(file); //only gonna check one.
          if(c2==' '||c2=='\n'){instruction_arguments[index_buffer] = c; c =c2; goto resume;}
          if(c=='b' && c2=='l'){instruction_arguments[index_buffer] = '\0';}
          if(c=='e' && c2=='o'){instruction_arguments[index_buffer] = EOF;}
          if(c=='n' && c2=='l'){instruction_arguments[index_buffer] = '\n';}
          if(c=='s' && c2=='p'){instruction_arguments[index_buffer] = ' ';}
        } else {
          if (new_word && instructions[index_buffer] == ('f')){ //instr was IF
            instruction_arguments[index_buffer] = c;
          } else if (instructions[index_buffer] == ('o')) { //instr was GO
            instruction_arguments[index_buffer] = (instruction_arguments[index_buffer]*=10) + (c-48);
          } else {
            //printf("invalid argument instruction %u: %c", index_buffer, c); 1.0/0.0; //the parsing is not actually tight enough to use this error message.
          }
        }
      }
      new_word = 0;
    }
  }
	fclose(file);
  dprintf("execute\n");
  unsigned int tapehead = 1;
  tape[0] = EOF;
  unsigned char skip = 0;
  for(unsigned int i = 1; i<=sizeof(instructions)/sizeof(instructions[0]); i++){
    instructions[i] && !skip && dprintf("%04u: %c %03d. tapehead (position %03u) on the letter %03u, \n", i, instructions[i], instruction_arguments[i], tapehead, tape[tapehead]); //print out the program for debug purposes
    if(instructions[i] && skip){ skip--; continue; }
    switch (tolower(instructions[i])){ //this is case insensitive
      case 'n':
        tape[tapehead] = getchar();
        dprintf("Getchar! %c, %d\n", tape[tapehead], tape[tapehead]);
      break;
      case 'u':
        putchar(tape[tapehead]);
      break;
      case 'e':
        if(!tapehead){printf("You tried to go off the left end of the tape at %d", i);fflush(stdin);}
        tapehead--;
      break;
      case 'i':
        tapehead++;
      break;
      case 'f':
        if(tape[tapehead] == (unsigned char)instruction_arguments[i]){
          //no action is needed in this case, execution will continue normally
        } else { skip = 1; } //can't just increment the instruction counter twice, sadly; have to remember the skip.
      break;
      case 'o':
        i = instruction_arguments[i];
        i--; //just set this up right for the imminent for loop increment
      break;
    }
  }
}