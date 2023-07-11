#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>

//The limiting factor for the arrays here is the Windows C compiler, cl. Cl helpfully tells us: "total size of array must not exceed 0x7fffffff bytes" (which happens to be INT_MAX)
//gcc is fine with UINT_MAX length (not bytes, number of elements). Exceeding that: "error: size of array ‘array’ is too large"
//and TCC works at some point lower than INT_MAX but higher than INT_MAX/2, (unclear if bytes or length) so we have chosen to ignore it here. "error: internal error: relocation failed"

//cl also started giving me "LINK : fatal error LNK1248: image size (C0025000) exceeds maximum allowable size (80000000)" errors at some point, or random "Access is denied." errors when attempting runs, so I adjusted down based on that.
unsigned char tape[INT_MAX/2];

unsigned char instructions[USHRT_MAX];

unsigned short instruction_arguments[USHRT_MAX]; //this is an unsigned short so that it can store indices to other instructions

int main(int argc, char **argv){
  char debug = 0;
  char file_index_in_argv = 1;
  #define eprintf(...) (fprintf(stderr, __VA_ARGS__),fflush(stderr))
  #define dprintf(...) (debug&&eprintf(__VA_ARGS__))
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
     printf("Input file %s can't be opened for reading, probably doesn't exist.\n", argv[file_index_in_argv]);
     if(!strcmp(argv[file_index_in_argv],"-d")){
       puts("Hey! It looks like you've specified -d without a flown script file, causing -d to be interpreted as the file name. You probably didn't mean to do that, especially because I can't find that file. Let me remind you:");
       puts("USAGE: input | flown [-d] script.fln | output");
       return -3; //script file "-d", specifically, not found
     }
     return -2; //script file not found
  }
  dprintf("%s\n", "parse");
  int c;
  enum parsing_state {index, instruction, argument};
  enum parsing_state state = index;
  int index_buffer = 0;
  char new_word = 1;
  char header_done = 0;
  unsigned int highest_instruction_index = 1;
  char iffysetty = 0;
  while( (c=fgetc(file)) != EOF ){
    if(!header_done&&(c<33||c>126)){continue;} else {header_done=1;} //to avoid unicode BOMs we now ignore everything until the first conventional printing ascii character.
    resume:
    //dprintf("\n\n c = %d, index_buffer %u", c, index_buffer);

    if (c=='#' && !(state==argument && instructions[index_buffer]=='f' && !iffysetty) ) { //ignore remarks, but also allow literal #s for ifs
      do{c=fgetc(file);}while(!(c=='\n'||c==EOF)); //eat characters until end of line or file
      if(c=='\n'){goto resume;} //then resume with the newline if applicable
    } else if(c=='\n') { //this is a very heavily line-oriented parser now I guess.
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
        index_buffer = index_buffer*10 + (c-48); //technically, you _can_ have non-number labels... they'll be converted to numbers though!
         //track the highest instruction. We do this each digit parse step, it's fine:
        if(index_buffer>highest_instruction_index){highest_instruction_index = index_buffer;}
        iffysetty = 0; //at this point, it's impossible that we've set the argument for any upcoming ifs
      }
      if(state==instruction){ 
        if (new_word){
          //dprintf("\n\n index_buffer %u", index_buffer);
          instructions[index_buffer] = tolower(fgetc(file)); //this is all we need. The second character is the cryptic N U E I F or O.
        }
      }
      if (state==argument) {
        if (new_word && instructions[index_buffer] == ('f')){ //instr was IF
          iffysetty = 1;
          new_word = 0;
          int lc = tolower(c); //I use tolower to strip case of these things for comparisons. But the original needs to be preserved in the argument in case it's semantically important for the user's application
          instruction_arguments[index_buffer] = c;
          if (lc=='b' || lc=='e' || lc=='n' || lc=='s') {//could be a special character
            int c2 = fgetc(file); //only gonna check one.
            int lc2 = tolower(c2);
            dprintf("%c%c\n",lc, lc2);
            if(lc=='b' && lc2=='l'){instruction_arguments[index_buffer] = '\0';}
            else if(lc=='e' && lc2=='o'){dprintf("eof tho\n");instruction_arguments[index_buffer] = (unsigned char)EOF;}
            else if(lc=='n' && lc2=='l'){instruction_arguments[index_buffer] = '\n';}
            else if(lc=='s' && lc2=='p'){instruction_arguments[index_buffer] = ' ';}
            else {instruction_arguments[index_buffer] = c; c = c2; goto resume;}
          }
        } else if (instructions[index_buffer] == 'o') { //instr was GO
          instruction_arguments[index_buffer] = instruction_arguments[index_buffer]*10 + (c-48);
        } else {
          //printf("invalid argument instruction %u: %c", index_buffer, c); 1.0/0.0; //the parsing is not actually tight enough to use this error message.
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
  for(unsigned int i = 1; i<=highest_instruction_index; i++){ //we used to stop at sizeof(instructions)/sizeof(instructions[0]) but this caused a little lag.
    instructions[i] && !skip && dprintf("%04u: %c(%d). tapehead (position %u) on the letter %u, \n", i, instructions[i], instruction_arguments[i], tapehead, tape[tapehead]); //print out the program for debug purposes
    //this would probably make way more sense if I tracked the rightmost point the tapehead has ever gone, and print the tape up to and only up to that point, but I couldn't be bothered.
    if(instructions[i] && !skip) { for(int t=1; tape[t] ; t++ ){dprintf("%c",tape[t]);}dprintf("\n"); }
    if(instructions[i] && skip){ skip--; continue; }
    switch (tolower(instructions[i])){ //this is case insensitive
      case 'n':
        tape[tapehead] = getchar();
        dprintf("Getchar! '%c', %d\n", tape[tapehead], tape[tapehead]);
      break;
      case 'u':
        putchar(tape[tapehead]);
      break;
      case 'e':
        if(!tapehead){eprintf("You tried to go off the left end of the tape at %d", i);fflush(stdin);}
        tapehead--;
      break;
      case 'i':
        tapehead++;
      break;
      case 'f':
        if(tape[tapehead] == instruction_arguments[i]){
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