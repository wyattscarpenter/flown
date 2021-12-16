# FLOWN

FLOWN is a minimal and somewhat bizarre programming language based on FLOW https://en.wikipedia.org/wiki/FLOW_(programming_language)

Much less user friendly than the original, FLOWN adds a Turing-machine-like tape, because it's weird that FLOW is extremely limited in possible applications due to its memory model. On the other hand, FLOWN is also weirdly extremely limited in possible applications due to its memory model.

There are six statements in FLOWN
```
IN
OUT
LEFT
RIGHT
IF (character)
GO (index)
```

Conceptually, FLOWN uses as its working memory (but not its instruction memory) an infinite symbol tape (note: for implementation reasons it's more like an INT_MAX symbol tape) like a Turing machine's. There is a "tape head" whose position is known as the "current tape cell". The tape head starts at the left of the tape. The tape starts filled with the numerical value 0. Conceptually, you start at the "first" tape cell on the right, and the "zeroth" tape cell has an EOF in it, for convenience. Going off the edge of the tape is undefined in this specification, and I haven't checked what happens. Also, going to the zeroth tape cell is undefined; that's right, I'm taking away the convenience immediately, because it made me nervous.

`IN` reads a symbol from standard input to the current tape cell (note: this will overwrite any current contents).
`OUT` writes a symbol from the current tape cell to standard output (note: the symbol is not erased from the tape cell).
`LEFT` advances the tape head one cell to the left (note: advancing left off the left end of the tape is Undefined Behavior).
`RIGHT` advances the tape head one cell to the right (note: advancing right off the right end of the tape is Undefined Behavior).
`IF` (character) executes the next (by numerical index, skipping empty instructions) statement if the value of the current tape cell is equal to (character); else, execution jumps to the overnext instruction. The character is written out literally in source code, except for these special characters: nl = newline character ('\n'), sp = space character (' '), eof = end-of-file indicator (EOF), blank = null character ('\0').
`GO` (index) jumps to the index and begins executing at that index.

NOTE: the eof character is equal to 0xFF aka 255 aka a full byte, so reading this byte will look exactly like having read the end of the input. Also, the blank character is equal to 0x00 aka 0 aka an empty byte, so writing this byte will look exactly like having written nothing. You can absolutely forbid these characters from appearing in your inputs, or you can write in clever little loops to ignore them yourself.

All statements must be numbered a la basic. The program begins executing statement 0. After executing a (non-GO) statement (or ignoring an empty statement), the statement counter increments by 1 and executes (or ignores, as appropriate) the statement currently referenced by the statement counter. Execution ends when the statement counter is set to a number greater than all defined statements.

There is at this time one preprocessor, or METAFLOWN, statement: REMARK. This lets you leave comments about the code. The entire remark, beginning at the R and up to the end of the line, is ignored.

Here are some example programs:

cat.fln (just outputs its input, like piping to the unix utility cat)
```
10 in
20 if eof
25 go 100
30 out
40 go 10
```

rev.fln (outputs the input reversed)
```
10 in
20 if eof
25 go 100
30 right
40 go 10

100 left
110 out
120 if eof
125 go 200
140 go 100
```

tac.fln (prints full lines in reverse order)
```
REMARK read in entire input
10 in
20 if eof
25 go 100
30 right
40 go 10

REMARK seek left to beginning of string
100 left
101 if eof
102 goto 200
101 if nl
102 goto 200
103 goto 100

REMARK print
REMARK care is taken not to actually write an EOF out
200 right
201 if eof
202 goto 300
210 out
211 if nl
212 goto 300
213 goto 200

REMARK seek left to trailing nl of previous line (if it's the beginning of the input, exit)
300 left
301 if eof
302 goto 1000
301 if nl
302 goto 100
303 goto 300


REMARK exit
1000
```

110.fln (implements rule110. requires an input stream of the initial state in 1s and 0s, then runs forever until externally halted.)
```
TODO
```
It would probably make the programming language better if we had something like the IT variable from FLOW and let you write arbitrary characters to the tape, but whatever. You may notice that it is (usually) impossible for this programming language to output any characters that aren't in the input; this is, uh, a security feature.

The parser of this program is fairly brittle and unhelpful because I didn't want to spend a lot of time on this. If I had to do it over again, I would probably do it in a more modern language. I chose to do it in C because of how easy it is to work with plain byte arrays in C, but I think the better convenience functions / string handling of modern languages would have been worth the loss of conceptual banebonedness. Also, if this were a REAL programming language, making real executables would be an advantage of C, but as it stands I probably should have done this in javascript anyway just so I could have a web-based demo for this toy language.

FLOWN is trivially Turing-complete. Here's why:
* The language (though not any particular implementation) has access to an infinite memory tape. The fact that this tape is left-ended does not make it any less infinitely long.
* You can arbitrarily extend the number of symbols each "cell" holds by using multiple (say, contiguous) memory cells to emulate more possibilities.
* You can read the input onto the tape to set the tape up how you like.
* You can always write as many eofs to the tape as you want after the input is exhausted. Using the multiple cell "cells" idea from before, you can use this to emulate an arbitrary number of symbols.
* The language has conditional branches.
* The FLOWN program is the finite state table.

Giving the program an infinite supply of eofs after the input is done was an interesting but necessary (therefore I don't feel bad about it—my hand was forced! Albeit forced by my other design decisions.) design choice. Firstly, I hate look-before-you-leap programming, so in FLOWN one has to write the eof character to the tape to check it. rev.fln, above, shows the computational/theoretical/practical necessity of presenting the eof symbol instead of just, say, crashing on attempt to read after input end, which I admit I would find pretty funny. I think technically you could get away with crashing if you had a non-deterministic FLOWN machine, or NFLOWN, the same way a Nondeterministic Pushdown Automata (NPDA) can recognize all even-length palindromes but a Deterministic Pushdown Automata can't.
