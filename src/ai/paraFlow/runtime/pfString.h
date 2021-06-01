/* Built in string handling. */

#ifndef PFSTRING_H

struct _pf_string *_pf_string_new(char *s, int size);
/* Wrap string buffer of given size. */

struct _pf_string *_pf_string_new_empty(int bufSize);
/* Create string with given bufSize, but no actual
 * contents. */

struct _pf_string *_pf_string_dupe(char *s, int size);
/* Clone string of given size and wrap string around it. */

struct _pf_string *_pf_string_from_const(char *s);
/* Wrap string around zero-terminated string. */

void _pf_string_make_independent_copy(_pf_Stack *stack);
/* Make it so that string on stack is an indepent copy of
 * what is already on stack.  As an optimization this routine
 * can do nothing if the refCount is exactly 1. */

struct _pf_string *_pf_string_cat(_pf_Stack *stack, int count);
/* Create new string that's a concatenation of the above strings. */

void _pf_strcat(_pf_String a, char *b, int bSize);
/* Concatenate b onto the end of a. */

_pf_Bit _pf_stringSame(_pf_String a, _pf_String b);
/* Comparison between two strings not on the stack. */

void _pf_string_substitute(_pf_Stack *stack);
/* Create new string on top of stack based on all of the
 * substitution parameters already on the stack.  The
 * first param is the string with the $ signs. The rest
 * of the parameters are strings  to substitute in, in the
 * same order as the $'s in the first param.  */

struct _pf_string *_pf_string_from_long(_pf_Long ll);
/* Wrap string around Long. */

struct _pf_string *_pf_string_from_double(_pf_Double d);
/* Wrap string around Double. */

void _pf_cm_string_dupe(_pf_Stack *stack);
/* Return duplicate of string */

void _pf_cm_string_start(_pf_Stack *stack);
/* Return start of string */

void _pf_cm_string_rest(_pf_Stack *stack);
/* Return rest of string (skipping up to start) */

void _pf_cm_string_middle(_pf_Stack *stack);
/* Return middle of string */

void _pf_cm_string_end(_pf_Stack *stack);
/* Return end of string */

void _pf_cm_string_upper(_pf_Stack *stack);
/* Uppercase existing string */

void _pf_cm_string_lower(_pf_Stack *stack);
/* Lowercase existing string */

void _pf_cm_string_append(_pf_Stack *stack);
/* Append to end of string. */

void _pf_cm_string_find(_pf_Stack *stack);
/* Find occurence of substring within string. */

int _pf_strcmp(_pf_Stack *stack);
/* Return comparison between strings.  Cleans them off
 * of stack.  Does not put result on stack because
 * the code generator may use it in different ways. */


#endif /* PFSTRING_H */
