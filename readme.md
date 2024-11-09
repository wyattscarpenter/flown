# FLOWN

FLOWN is a minimal and somewhat bizarre programming language based on FLOW https://en.wikipedia.org/wiki/FLOW_(programming_language)

Much less user friendly than the original, FLOWN adds a Turing-machine-like tape, because it's weird that FLOW is extremely limited in possible applications due to its memory model. On the other hand, FLOWN is also weirdly extremely limited in possible applications due to its memory model.

There are six statements in FLOWN
```
IN
OUT
LEFT
RIGHT
IF character
GO index
```

These statements are not case-sensitive, so, eg, 'if' works just as well as 'IF'.

Conceptually, FLOWN uses as its working memory (but not its instruction memory) an infinite symbol tape (note: for implementation reasons it's more like an INT_MAX symbol tape) like a Turing machine's. There is a "tape head" whose position is known as the "current tape cell". The tape head starts at the left of the tape. The tape starts filled with the numerical value 0. Conceptually, you start at the "first" tape cell on the right, and the "zeroth" tape cell has an EOF in it, for convenience. tac.fln, below, is a motivating example for why this is convenient. Going off the edge of the tape is undefined in this specification, and I haven't checked what happens.

`IN` reads a symbol from standard input to the current tape cell. This will overwrite any current contents.

`OUT` writes a symbol from the current tape cell to standard output. The symbol is not erased from the tape cell.

`LEFT` advances the tape head one cell to the left. Advancing left off the left end of the tape is Undefined Behavior.

`RIGHT` advances the tape head one cell to the right. Advancing right off the right end of the tape is Undefined Behavior.

`IF character` executes the next (by numerical index, skipping empty instructions) statement if the value of the current tape cell is equal to the character; else, execution jumps to the overnext instruction.

`GO index` jumps to the index and begins executing at that index.

(The attentive reader may notice that because `go` and `if` both jump, this instruction set is not minimal in number of keywords; for instance, I could have made `GOIF character index elseindex`. However, this ergonomic choice is intentional on my part; I could also have combined all instructions into a single, turing-complete instruction, which we might call `IOLRIG`. Don't believe me? In x86, the `mov` instruction is turing-complete, hence mov Doom https://github.com/xoreaxeaxeax/movfuscator/tree/master/validation/doom. See also:
"Only rank amateurs rely on general-purpose if statements. Veteran programmers prefer the elegant Turing completeness of the "Subtract and Branch if Not equal to Zero" instruction, SBNZ." —Anonymous)

Characters are written out literally in source code, except for these special characters: nl = newline character ('\n'), sp = space character (' '), eof = end-of-file indicator (EOF), blank = null character ('\0').

The eof character is equal to 0xFF aka 255 aka a full byte, so reading this byte will look exactly like having read the end of the input. Also, the blank character is equal to 0x00 aka 0 aka an empty byte, so writing this byte will look exactly like having written nothing. You can absolutely forbid these characters from appearing in your inputs, or you can write in clever little loops to ignore them yourself.

All statements must be numbered a la basic. The program begins executing statement 1. After executing a (non-GO) statement (or ignoring an empty statement), the statement counter increments by 1 and executes (or ignores, as appropriate) the statement currently referenced by the statement counter. Execution ends when the statement counter is set to a number greater than all defined statements.

There is at this time one preprocessor, or METAFLOWN, symbol: #. This lets you leave remarks (also known as comments) in the flown code. The entire remark, beginning at the # and up to the end of the line, is ignored. Note: if used as the argument in an if statement, the # will be interpreted literally, and will not trigger a remark. Example:

```
100 if # #The first hashmark will be interpreted literally, but the second starts this remark.
```

In an environment with shebangs, like posix scripting, you should be able to shebang flown scripts to wherever you keep the flown executable; in my tests it does seem to work.

I probably would have added more metaflown commands but honestly I don't want to modify the parser (which is hairy) or add another compile step. Even though adding multi-cell if would be useful, and implementing functions would be fun.

Well, OK, alright, I added one more command, ERR, which is like OUT except it prints to stderr. Even though it's just in the regular parser and works like a regular command and even requires a numerical position, like all the other commands, it's a METAFLOWN command because I don't think it's strictly necessary for the FLOWN paradigm. It just makes things easier (ish) for unix-inspired systems (all of them).

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
# read in entire input
10 in
20 if eof
25 go 100
30 right
40 go 10

# seek left to beginning of string
100 left
101 if eof
102 go 200
111 if nl
112 go 200
113 go 100

# print
# care is taken not to actually write an EOF out
200 right
201 if eof
202 go 300
210 out
211 if nl
212 go 300
213 go 200

# seek left to trailing nl of previous line (if it's the beginning of the input, exit)
300 left
301 if eof
302 go 1000
311 if nl
312 go 100
313 go 300


# exit
1000
```

124.fln (demonstrates turing-completeness by implementing rule110—well, actually, implements the mirror image version, 124, because our tape is left-bounded so that's more convenient. requires an input stream of the initial state in 1s and 0s terminated with a single newline or other separator character, then runs forever until externally halted. 1s are output as eofs, 0s and blanks, and the separator as itself.)

This is too long for the readme, see the file.

fg.fln (the first listing in the original FLOW paper is about detecting whether a text has an f or g in it. Here we output eof if true and blank if false)
```
10 in
20 if f
30 go 100
40 if g
50 go 100
60 if eof
70 go 200
80 go 10

100 in
110 if eof
111 go 150
112 go 100
150 out
160 go 300

200 right
210 out
```
evenodd.fln (the second listing in the original FLOW paper is about detecting whether a text has an even or odd number of characters. Here we output eof if odd and blank if even)
```
#even (starting with 0)
10 in
20 if eof
30 go 200
40 go 50

#odd
50 in
60 if eof
70 go 100
80 go 10

#print result for odd
100 out
110 go 300 # we already have an eof so we can just print

#print result for even
200 right #just go one right to get a blank
210 out
```

As a further exercise for the reader, you can try implementing head, based on the default behavior of the Unix utility head (trivial) and also headn and headc (difficult). Note that flown programs do not take command line arguments, so the first part of the input stream for those latter commands would have to be, say, a string of digits to represent a number, followed by a seperator character (in headn, the separator character provided by the user could even be used in lieu of the standard separator newline character for the purposes of calculating "lines" — greatly complicating the implementation, for no clear benefit. But you could!). You could also implement tail. You could also implement any other program, because FLOWN is Turing-complete. You could also implement a compiler from a more-sensible language to FLOWN, while we're at it.

It would probably make the programming language better if we had something like the IT variable from FLOW and let you write arbitrary characters to the tape, but whatever. You may notice that it is (usually) impossible for this programming language to output any characters that aren't in the input; this is, uh, a security feature.

The parser of this program is fairly brittle and unhelpful because I didn't want to spend a lot of time on this. If I had to do it over again, I would probably do it in a more modern language. I chose to do it in C because of how easy it is to work with plain byte arrays in C, but I think the better convenience functions / string handling of modern languages would have been worth the loss of conceptual barebonedness. Also, if this were a *real* programming language, making real(ish) executables could be an advantage of C, but as it stands I probably should have done this in javascript anyway just so I could have a web-based demo for this toy language.

(An easy way to make an executable out of a FLOWN program would be to embed that program as a string in the C program that implements the FLOWN interpreter (set to interpret the string), and then compile that program. This has the benefit of being easy. A more-"real" executable would compile the flown down to machine instructions (might not even be that hard! there are only a few!), which is less easy but is the right way to do things and also would give a performance benefit. The embedding trick can also be used in other programming languages, like JavaScript, but to make an executable out of JavaScript I think they use the embedding trick as well, or something like it, so we would be double-embedding, so to speak.)

FLOWN is trivially Turing-complete. Here's why:
* The language (though not any particular implementation) has access to an infinite memory tape. The fact that this tape is left-ended does not make it any less infinitely long.
* You can arbitrarily extend the number of symbols each "cell" holds by using multiple (say, contiguous) memory cells to emulate more possibilities.
* You can read the input onto the tape to set the tape up how you like.
* You can always write as many eofs to the tape as you want after the input is exhausted. Using the multiple cell "cells" idea from before, you can use this to emulate an arbitrary number of symbols.
* The language has conditional branches.
* The FLOWN program is the finite state table.

Giving the program an infinite supply of eofs after the input is done was an interesting but necessary (therefore I don't feel bad about it—my hand was forced! Albeit forced by my other design decisions.) design choice. Firstly, I hate look-before-you-leap programming, so in FLOWN one has to write the eof character to the tape to check it. rev.fln, above, shows the computational/theoretical/practical necessity of presenting the eof symbol instead of just, say, crashing on attempt to read after input end, which I admit I would find pretty funny. I think technically you could get away with crashing if you had a non-deterministic FLOWN machine, or NFLOWN, the same way a Nondeterministic Pushdown Automata (NPDA) can recognize all even-length palindromes but a Deterministic Pushdown Automata can't.

You may want to pipe the output of some flown programs to the unix utility `less`, as it will automatically display the nonprinting characters of eof and blank.

You can find the original FLOW paper documenting FLOW, the language that FLOWN is inspired by, wherever fine academic publications are provided: “FLOW: A teaching language for computer programming in the humanities” by Jef Raskin, Computers and the Humanities, Vol. 8, pp. 231-237. PERGAMON PRESS, 1974. Printed in the U.S.A.

TODO: Finish 124.fln, possibly involving an additional compiler step.

TODO: More-robust testing script.
