# beatnik
Beatnik esoteric language interpreter written in C. This uses curses for keyboard capture on Linux. It uses console I/O on Windows.

In the Beatnik language, every word is scored as a Scrabble word, and the resulting number is interpreted as a command to be executed on a stack machine:
 - 5 Consume and push the score of the next word onto the stack
 - 6 Pop the top value in the stack, discard it
 - 7 Pop the two top values of the stack, add them and push the result on the stack
 - 8 Input a character from the user and push it on the stack
 - 9 Pop the top value from the stack and output it as ASCII
 - 10 Pop the two top values from the stack, subtract the first from the second and push the result on the stack
 - 11 Swap the two top values from the stack
 - 12 Duplicate the top value on the stack
 - 13 Pop the top value in the stack. If it is 0 then skip ahead by the word count indicated by the following word
 - 14 Pop the top value in the stack. If it is not 0 then skip ahead by the word count indicated by the following word
 - 15 Pop the top value in the stack. If it is 0 then skip back by the word count indicated by the following word
 - 16 Pop the top value in the stack. If it is not 0 then skip back by the word count indicated by the following word
 - 17 Stop the program

Other scores are ignored but might express comments in future versions.
