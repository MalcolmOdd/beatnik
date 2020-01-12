#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <curses.h>
#endif
const int SCORES[] = { 1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 5, 1, 3, 1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10 };

const int FN_START = 5;

typedef struct
{
  int * data;
  int cur;
  int size;
} INT_VECTOR;

typedef struct
{
  char * const name;
  void (*function)(INT_VECTOR *program, INT_VECTOR *stack);
  int count;
} FUNCTION;

/* Functions to manipulate INT_VECTOR.
 * The vector is auto-expanding. It can be used a a stack or as a program stream.
 * The difference is that in stack mode, the space beyond cur is considered as not used
 *  but in the program stream mode, it contains future instructions.
 */
void push(INT_VECTOR *vector, int value)
{
  if (vector->cur >= vector->size)
  {
    if (vector->size > 0)
      vector->size *= 2;
    else vector->size = 16;
    vector->data = (int *)realloc(vector->data, vector->size * sizeof(int));
  }
  vector->data[vector->cur++] = value;
}

int pop(INT_VECTOR *vector)
{
  if (vector->cur <= 0)
    return 0;
  return vector->data[--vector->cur];
}

int getNext(INT_VECTOR *vector)
{
  if (vector->cur >= vector->size)
    return -1;
  return vector->data[vector->cur++];
}

void initVector(INT_VECTOR *vector)
{
  vector->data = NULL;
  vector->size = 0;
  vector->cur = 0;
}

void freeVector(INT_VECTOR *vector)
{
    if (vector->data)
    {
        free(vector->data);
        vector->data = NULL;
    }
    vector->size = 0;
}

/* Functions that will be registered as pointers to handle Beatnik commands */

void pushNext(INT_VECTOR *program, INT_VECTOR *stack)
{
  push(stack, getNext(program));
}

void popDiscard(INT_VECTOR *program, INT_VECTOR *stack)
{
  pop(stack);
}

void add(INT_VECTOR *program, INT_VECTOR *stack)
{
  push(stack, pop(stack) + pop(stack));
}

void input(INT_VECTOR *program, INT_VECTOR *stack)
{
  push(stack, getch());
}

void output(INT_VECTOR *program, INT_VECTOR *stack)
{
  putchar(pop(stack));
}

void substract(INT_VECTOR *program, INT_VECTOR *stack)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a - b);
}

void swap(INT_VECTOR *program, INT_VECTOR *stack)
{
  int a, b;
  a = pop(stack);
  b = pop(stack);
  push(stack, a);
  push(stack, b);
}

void duplicate(INT_VECTOR *program, INT_VECTOR *stack)
{
  int a = pop(stack);
  push(stack, a);
  push(stack, a);
}

void forwardOnZero(INT_VECTOR *program, INT_VECTOR *stack)
{
  int nbr = getNext(program);
  if (!pop(stack))
  {
    program->cur += nbr;
  }
}

void forwardOnNonZero(INT_VECTOR *program, INT_VECTOR *stack)
{
  int nbr = getNext(program);
  if (pop(stack))
  {
    program->cur += nbr;
  }
}

void backOnZero(INT_VECTOR *program, INT_VECTOR *stack)
{
  int nbr = getNext(program);
  if (!pop(stack))
  {
    program->cur -= nbr;
  }
}

void backOnNonZero(INT_VECTOR *program, INT_VECTOR *stack)
{
  int nbr = getNext(program);
  if (pop(stack))
  {
    program->cur -= nbr;
  }
}

void stop(INT_VECTOR *program, INT_VECTOR *stack)
{
  program->cur = program->size;
}

const FUNCTION functions [] =
{
  { "Push", pushNext, 2 },
  { "Pop", popDiscard, 1 },
  { "Add", add, 1 },
  { "Input", input, 1 },
  { "Output", output, 1 },
  { "Substract", substract, 1 },
  { "Swap", swap, 1 },
  { "Duplicate", duplicate, 1 },
  { "ForwardOnZero", forwardOnZero, 2 },
  { "ForwardOnNonZero", forwardOnNonZero, 2 },
  { "BackOnZero", backOnZero, 2 },
  { "BackOnNonZero", backOnNonZero, 2 },
  { "Stop", stop, 1 }
};

/* Scrabble scoring function */
int getCharValue(char c)
{
  int i = toupper(c) - 'A';
  if (i < 0 || i >= (sizeof(SCORES)/sizeof(SCORES[0])))
    return 0;
  return SCORES[i];
}

/**
 * Parse a program from the given stream and add it in the given vector
 * @param program the Vector to add the instructions to
 * @param in The stream to parse
 */
void parse(INT_VECTOR *program, FILE *in)
{
  int c = 0;
  int val;
  int sum = 0;
  while ((c = getc(in)) != EOF)
  {
    val = getCharValue(c);
    if (val == 0 && sum != 0)
    {
      push(program, sum);
      sum = 0;
      continue;
    }
    sum += val;
  }
  if (sum != 0)
  {
      push(program, sum);
  }
  program->size = program->cur;
  program->cur = 0;
}

/**
 * Execute a program
 * @param program the instruction Vector to execute
 */
void executeProgram(INT_VECTOR *program)
{
  INT_VECTOR stack;
  int c;
  initVector(&stack);
  program->cur = 0;
  while (program->cur < program->size)
  {
    c = getNext(program) - FN_START;
    if (c >= 0 && c < (sizeof(functions)/sizeof(functions[0])))
    {
      if (functions[c].function)
        functions[c].function(program, &stack);
    }
  }
  freeVector(&stack);
}

/**
 * Lists the instructions of a program
 * @param program the instruction Vector to list
 */
void listProgram(INT_VECTOR *program)
{
  int c;
  int i;
  while (program->cur < program->size)
  {
    c = getNext(program) - FN_START;
    if (c >= 0 && c < (sizeof(functions)/sizeof(functions[0])))
    {
      printf("%s", functions[c].name);
      for (i = 1; i < functions[c].count; ++i)
        printf(" %d", getNext(program));
      printf("\n");
    }
  }
}

/**
 * Print program usage in the given stream
 * @param out The stream to write to
 */
void showUsage(FILE *out)
{
    fprintf(out, "Beatnik language interpreter by MalcolmOdd\n");
    fprintf(out, "Usage: beatnik [-l] [-h] [<filename>]\n");
    fprintf(out, "   -l: List disassembly\n");
    fprintf(out, "   -h: Show this help\n");
    fprintf(out, "   If a file name is specified, it will be parsed. Otherwise, standard input will be used\n");
}

/**
 * The program entry point
 * @param argc The argument count
 * @param argv The argument values. Argument 0 is the program being executed.
 */
int main(int argc, char* argv[])
{
  FILE *in;
  int i;
  int option = 0;
  const int OPT_LIST = 1;
  char *fileName = NULL;
  INT_VECTOR program;
#ifndef _WIN32
    initscr();
#endif
  initVector(&program);
  for (i = 1; i < argc; ++i)
  {
    if (argv[i][0] == '-')
    {
      switch(tolower(argv[i][1]))
      {
        case 'l': option |= OPT_LIST; break;
        case 'h': showUsage(stdout); return 0; 
        default:
          fprintf(stderr, "Error: Invalid option %s\n", argv[1]);
          showUsage(stderr);
          return -1;
      }        
    }
    else
    {
      if (fileName)
      {
        fprintf(stderr, "Error: More than one file name was specified\n");
        return -1;
      }
      fileName = argv[i];
    }
  }
  if (fileName) 
  {
      in = fopen(fileName, "r");
      if (!in)
      {
        fprintf(stderr, "Error: Cannot open %s\n", fileName);
        showUsage(stderr);
        return -1;
      }
  }
  else
  {
    in = stdin;
  }
  parse(&program, in);
  if (option & OPT_LIST)
  {
    listProgram(&program);
  }
  else
  {
    executeProgram(&program);
  }
  freeVector(&program);
  if (in != stdin)
  {
    fclose(in);
  }
#ifndef _WIN32
    endwin();
#endif
  return 0;
}

