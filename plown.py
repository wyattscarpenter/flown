# I'm completely unsure about the wisdom of this form for plown to have. I've also lost interest in plown in general. In fact, if I were to create a language that compiles down to flown, it would probably just be a normal language that happens to have flown as a backnd, not a closely-coupled language like plown might be. Regardless, I reluctantly mark this file TODO in case I want to come back to it later. (Once plown is complete, this branch will probably be merged into the main branch, in git.). But it should be done more-or-less after every other task I might want to do!

# Plown is a very small programming language that compiles to flown. It consists mostly of vector-if statements and loop constructs. It can be run in python (note: will this be true? edit: no. But maybe some day) or in plown itself; both self-hosted and not-self-hosted, which makes bootstrapping easier.
# The plown compiler should be piped all of its input and outputs all of its output to system out, like flown or any good unix program should. It can also stderr.
# It's actually more of a preprocessor, and its outputs can be slipped into a flown script...

# Unless you enable the Plown runtime, which has different assumptions. 
# The "Plown runtime" also requires you pass in the bytes 0-255 to your plown-flown program, so it can use all the letters.
# Plown has print statements, but because of how implementation works (using the plown runtime mentioned just now) this will also reset the tapehead to the... wait, actually, plown will probably need meta-cell architecture anyway, so maybe we can just scan back to the dirty bit or something?

# Eh....
INITIAL_OFFSET = 0
MAXIMUM_OFFSET = 1000

current_offset = INITIAL_OFFSET

plown_initialize_runtime = "read*256"

# Something like this probably...
while (input_line):


  if current_offset > MAXIMUM_OFFSET:
    print("current_offset > MAXIMUM_OFFSET", stderr)
    exit(0xFF)

# What syntax would be useful? Probably something like
# VECTOR-IF 'a' eof 'b' THEN () ELSE ()
# and we compute and then print the instruction blocks inside those parens



print("operation completed successfully", stderr)
exit(0)