#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>

//The limiting factor on this memory array here is the Windows C compiler, cl. Cl helpfully tells us: "total size of array must not exceed 0x7fffffff bytes" (which happens to be INT_MAX)
//gcc is fine with UINT_MAX length (not bytes, number of elements). Exceeding that: "error: size of array ‘array’ is too large"
//and TCC works at some point lower than INT_MAX but higher than INT_MAX/2, (unclear if bytes or length) so we have chosen to ignore it here. "error: internal error: relocation failed"
unsigned char tape[INT_MAX];

unsigned char instructions[INT_MAX/4];

unsigned int instruction_arguments[INT_MAX/8];

int main(int argc, char **argv){
  char debug = 0;
  char file_index_in_argv = 1;
  #define eprintf(...) (fprintf(stderr, __VA_ARGS__),fflush(stderr))
  #define dprintf(...) (debug&&fprintf(stderr, __VA_ARGS__),fflush(stderr))
  //printf("u: %u d: %d (unsigned char)u: %u c: %c\n", EOF, EOF, (unsigned char)EOF, EOF); //good for remembering what EOF is (-1).
  //printf("%x INT_MAX, %x sizeof(tape)\n", INT_MAX, sizeof(tape)); //confirm these are the same
  //tape[INT_MAX/4-1] = 0xFF; //uh oh, this is a linker error for me on gcc
  if(argc==3){
    if(!strcmp(argv[1],"-d")){ //check for a debug flag
      debug = 1;
      file_index_in_argv = 2;
    }
  }else if(argc!=2){
    puts("USAGE: input | flown [-d] script.fln | output");
    return -1; //bad arguments
  }
  dprintf("%s", "open the file\n");
  FILE *file = fopen(argv[file_index_in_argv],"r");
  if(!file){
     puts("input file");
     puts(argv[file_index_in_argv]);
     puts("can't be opened for reading, probably doesn't exist.");
     return -2;
  }
  dprintf("%s\n", "parse");
  unsigned char c;
  enum parsing_state {index, instruction};
  enum parsing_state state = index;
  int index_buffer = 0;
  char new_word = 1;
  char header_done = 0;
  unsigned int highest_instruction_index = 1;
  //TODO: I recently messed up all the parsing chasing that sweet sweet # high. Lost ability to think halfway through.
  while((c=fgetc(file))!= (unsigned char)EOF){
    if(!header_done&&(c<48||c>57)){continue;} else {header_done=1;} //to avoid unicode BOMs we now ignore anything but 0-9 until the first 0-9.
    resume:
    dprintf("\n\n c = %d, index_buffer %u", c, index_buffer);

    if (c=='#') {
      do{c=fgetc(file);}while(!(c=='\n'||c==(unsigned char)EOF)); //eat characters until end of line or file
      if(c=='\n'){goto resume;} //then resume with the newline if applicable
    } else if(c=='\n') { //a new line will reset the parser state
      state = index;
      index_buffer = 0;
      new_word = 1;
    } else if(c==' '){
      if(!new_word){
        state = ((int)state + 1 ) % 2;
        new_word = 1;
      }
    } else {
      new_word = 0;
      if(state==index){
        index_buffer = (index_buffer*=10) + (c-48); //technically, you _can_ have non-number labels... they'll be converted to numbers though!
        //track the highest instruction. We do this each digit parse step, it's fine:
        if(index_buffer>highest_instruction_index){highest_instruction_index = index_buffer;}
      }
      if(state==instruction){ 
        dprintf("\n\n index_buffer %u", index_buffer);
        instructions[index_buffer] = fgetc(file); //this is all we need. The second character is the somewhat cryptic N U E I F or O.
        
        if(instructions[index_buffer] == ('f') || instructions[index_buffer] == ('g')){
        //now we get the argument by eating characters until we get what we want:
          do{c=fgetc(file);}while(!(c==' ')); //eat characters until end of word
          while(c==' '){c=fgetc(file);} //eat all the space characters
          c=fgetc(file); //the character of interest
          if(instructions[index_buffer] == ('f')){ //instr was IF
            if ((c=='b' || c=='e' || c=='n' || c=='s')) {//could be a special character
              char c2 = fgetc(file); //only gonna check one extra letter.
              if(c2==' '||c2=='\n'){instruction_arguments[index_buffer] = c; c =c2; goto resume;}
              if(c=='b' && c2=='l'){instruction_arguments[index_buffer] = '\0';}
              if(c=='e' && c2=='o'){instruction_arguments[index_buffer] = EOF;}
              if(c=='n' && c2=='l'){instruction_arguments[index_buffer] = '\n';}
              if(c=='s' && c2=='p'){instruction_arguments[index_buffer] = ' ';}
            } else {
              instruction_arguments[index_buffer] = c;
            }
          } else if (instructions[index_buffer] == ('o')) { //instr was GO
            while(c!=' '){
              instruction_arguments[index_buffer] = (instruction_arguments[index_buffer]*=10) + (c-48);
              c=fgetc(file);
            }
          }
        /*eprintf("%s: invalid argument instruction %u: %c", argv[file_index_in_argv], index_buffer, c); return c; //intentionally trigger a crash */
        }
      }
    }
  }
	fclose(file);
  dprintf("execute\n");
  unsigned int tapehead = 1;
  tape[0] = EOF;
  unsigned char skip = 0;
  for(unsigned int i = 1; i<=highest_instruction_index; i++){ //we used to stop at sizeof(instructions)/sizeof(instructions[0]) but this caused a little lag.
    instructions[i] && !skip && dprintf("%04u: %c argument: %03d. tapehead (position %03u) on the letter %03u, \n", i, instructions[i], instruction_arguments[i], tapehead, tape[tapehead]); //print out the program for debug purposes
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
        if(!tapehead){eprintf("You tried to go off the left end of the tape at %d\n", i);return i;}
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