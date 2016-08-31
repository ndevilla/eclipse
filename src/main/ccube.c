/*----------------------------------------------------------------------------*/
/**
   @file    ccube.c
   @author  Nicolas Devillard
   @date    Nov 07, 1996
   @version	$Revision: 1.24 $
   @brief   cube calculator
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: ccube.c,v 1.24 2002/11/19 10:35:33 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/19 10:35:33 $
	$Revision: 1.24 $
 */

/*-----------------------------------------------------------------------------
  								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#include <regex.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
  								Defines
 -----------------------------------------------------------------------------*/

#define ARITHM_STANDARD		0
#define ARITHM_POLISH		1

#define SEPARATOR		    " "
#define MAX_OP			    100 
#define AREXPSZ             (MAX_OP * (FILENAMESZ+1))

#define OPT_STANDARD		1001
#define OPT_POLISH			1002

/*-----------------------------------------------------------------------------
   								New types
 -----------------------------------------------------------------------------*/

/* 
 * An arithmetic expression is composed of FITS files, floating-point
 * double precision numbers, and operators.
 * The following types help encapsulate all into a forward linked-list
 * representing a stack. Stack elements are abstract types which can be
 * images, numbers, operators, or brackets if working in std notation.
 */

typedef union _ITEM_VALUE_ {
	cube_t *	cubept ;
	double		f ;
	char	op ;
} itemValue ;

/*
 * Following is a list of labels to identify the object type
 */
typedef enum _ITEM_TYPE_ { 
	image,
	number,

	/* 
	 * left and right brackets are no operators, they are used to
	 * convert standard arithmetic expressions into PRN ones.
	 */
	left_bracket,
	right_bracket, 
	/* a right bracket cannot be pushed on the stack with
	 * the current algorithm. However, the definition is
	 * included, just in case...
	 */

	operator,
	unknown
} itemType ;

/*
 * An item is a struct containing a label for its type, and a value
 */
typedef struct _ITEM_ {
	itemValue	v ;
	itemType	t ;
} item ;

/*
 * This is one stack element: it contains an item, and a pointer
 * to the next item.
 */
typedef struct _STACK_ {
	item *				data ;
	struct _STACK_ *	next ;
} stack ;


/*-----------------------------------------------------------------------------
   							Global variables	
 -----------------------------------------------------------------------------*/

/* First file name found to preserve FITS header information in the output. */
char firstName[FILENAMESZ+1] ;

/*-----------------------------------------------------------------------------
  							Function prototypes
 -----------------------------------------------------------------------------*/

static void 	push(item *value, stack *sp) ;
static item *	pop(stack *sp) ;
static itemType	gettokentype(char *token) ;
static item *	applyOperator(item *first, item *second, char operator) ;
static item *	ParseExpression(char *expr, int arithm) ;
static item * 	topstack(stack *sp) ;
static int		priority(char op) ;
static char *   tokenizeExpression(char *arexp) ; 
static void 	strip_blanks(char *exp) ;

static void usage(char *pname) ;
static char prog_desc[] = "cube computer" ;

/*-----------------------------------------------------------------------------
  							    Main
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	char	    outname[FILENAMESZ+1] ;
	char	    arithmetic_expression[AREXPSZ];
	char    *   saved_exp ;
	char    *	tokenizedExp ;
	item    *	result ;
	int		    arithm ; 
	history *   hs ;
	int		    c ;

    /* Initialize */
    arithm = ARITHM_POLISH ;

    /* Test inputs */
	if (argc<2) usage(argv[0]) ;

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

			{"standard",0, 0, OPT_STANDARD},
            {"polish",  0, 0, OPT_POLISH},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "Lhps",
                        long_options,
                        &option_index) ;
        if (c==-1) break ;

        switch(c)
        {

        /* Standard option: display license undocumented option */
            case OPT_LICENSE:
            case 'L':
            eclipse_display_license() ;
            return 0 ;

        /* Standard option : help */
            case OPT_HELP:
            case 'h':
            usage(argv[0]) ;
            break ;

        /* Standard option: version */
            case OPT_VERSION:
            print_eclipse_version() ;
            return 0 ;

        /* Local options */
			case OPT_STANDARD:
			case 's':
			arithm = ARITHM_STANDARD;
			break ;

			case OPT_POLISH:
			case 'p':
			arithm = ARITHM_POLISH;
			break ;

            default:
            usage(argv[0]) ;
            break ;
        }
    }
 
	if ((argc-optind) < 1) {
		e_error("missing arguments") ;
		return -1 ;
	}

	/* Initialize eclipse environment */
	eclipse_init();

	if (arithm == ARITHM_STANDARD) {
		e_comment(0, "tokenizing expression...") ;
		tokenizedExp = tokenizeExpression(argv[optind]) ;
		if (tokenizedExp == NULL) {
			e_error("in parsing expression: aborting") ;
			return -1 ;
		}
		strncpy(arithmetic_expression, tokenizedExp, AREXPSZ) ;
		free(tokenizedExp) ;
	} else {
		strncpy(arithmetic_expression, argv[optind], AREXPSZ) ;
	}

	optind++ ;

    /* Get arguments    */
    /* argc - optind is the number of remaining arguments       */
    /* argv[optind] is the first argument which is no option    */
    /* nor option argument.                                     */
    if ((argc-optind) >= 1) strncpy(outname, argv[optind], FILENAMESZ) ;
    else strncpy(outname, "comp.fits", FILENAMESZ) ;
 
    saved_exp = strdup(arithmetic_expression);
	result = ParseExpression(arithmetic_expression, arithm) ;
	if (result == NULL) {
		e_error("in computation: aborting") ;
        free(saved_exp);
		return -1 ;
	}

	if (result->t == number) printf("%g\n", result->v.f) ;
	else {
		hs = history_new();
		history_add(hs, "--- eclipse ccube");
		history_add(hs, saved_exp);
		cube_save_fits_hdrcopy_wh(result->v.cubept, outname, firstName, hs) ;
		history_del(hs);
		cube_del(result->v.cubept) ;
	}

    /* Free and return */
	free(saved_exp) ;
	free(result) ;
	if (debug_active()) xmemory_status() ;
	return 0 ;
}


/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Push a value on top of the stack
  @param	value	Value to push onto the stack
  @param	sp		Stack pointer
  @return	void
  Push a value on top of the stack.
 */
/*----------------------------------------------------------------------------*/
static void push(
        item    *   value,
        stack   * 	sp)
{
	stack   *   new ;
	stack   *	mov ;

	if (sp == NULL) {
		e_error("unexpected NULL stack: aborting") ;
		exit(-1) ;
	}	
	new = malloc(sizeof(stack));
	/* Go to last stack element */
	mov = sp ;
	while (mov->next != NULL) mov = mov->next ;
	/* initialize element to be appended with relevant information	*/
	new->next = NULL ;
	new->data = value ;
	/* append new element at the end of stack	*/
	mov->next = new ; 
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Pop a value from the stack.
  @param	sp	Stack pointer
  @return	1 pointer to last pushed stack value.
  Pop the latest value pushed on the stack. The stack is modified.
 */
/*----------------------------------------------------------------------------*/
static item * pop(stack * sp)
{
	stack   *	mov ;
	item    *	value ;

	if (sp == NULL) {
		e_error("unexpected NULL stack") ;
		exit(-1) ;
	}

	/* Only one element in stack means the stack is empty	*/
	if (sp->next == NULL) {
		e_error("empty stack: syntax error in expression") ;
		exit(-1) ;
	}	

	/* find the just-before-last element (at least 2 elements in stack) */
	mov = sp ;
	while (mov->next->next != NULL) mov = mov->next ;
	/* Fill up the value to return with last element's information	*/
	value = mov->next->data ;
	/* Delete last element */
	free(mov->next) ;
	mov->next = NULL ;
	return value ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Return a copy of the value on top of the stack.
  @param	sp	Stack pointer
  @return	1 pointer to a newly allocated object.
  Returns a copy of the value currently on top of the stack, without
  modifying the stack. 
 */
/*----------------------------------------------------------------------------*/
static item * topstack(stack * sp)
{
	item    *   value ;
	stack   *	mov ;

	if (sp == NULL) {
		e_error("unexpected NULL stack: aborting") ;
		exit(-1) ;
	}

	/* Only one element means the stack is empty	*/
	if (sp->next == NULL) return NULL ;

	/* go to last element	*/
	mov = sp ;
	while (mov->next != NULL) mov = mov->next ;
	value = malloc(sizeof(item));
	/* copy information into newly allocated stack element	*/
	value->t = mov->data->t ;
	value->v = mov->data->v ;
	return value ;
}	


/*----------------------------------------------------------------------------*/
/**
  @brief	Returns the priority of a given operation.
  @param	op	Operation to identify.
  @return	1 int.

  Possible operations are:
    - Addition: "+" priority 1
    - Subtraction: "-" priority 1
    - Multiplication: "*" priority 2
    - Division: "/" priority 2
    - Logarithm: "l" priority 3
    - Exponent: "^" priority 4

  The returned int indicates the priority of the operator. The bigger the
  returned integer, the higher the priority.
 */
/*----------------------------------------------------------------------------*/
static int priority(char op)
{
	int	p ;

	switch(op) {
		/* addition and subtration have the same priority	*/
		case '+': p = 1 ; break ;
		case '-': p = 1 ; break ;
		/* multiplication and division have the same priority	*/
		case '*': p = 2 ; break ;
		case '/': p = 2 ; break ;
		/* logarithm	*/
		case 'l': p = 3 ; break ;
		/* exponentiation	*/
		case '^': p = 4 ; break ;
		/* default: lowest possible priority (should not happen)	*/
		default: p=0 ; break ;
	}
	return p ;
}	


/*----------------------------------------------------------------------------*/
/**
  @brief	Find out if a token is an operator or an operand.
  @param	token	Token to examine.
  @return	itemType identifying the type of token.

  Examine a token to see if it is an operator or an operand.
  Possible return values are:
    - unknown
    - left_bracket
    - right_bracket
    - image
    - number
    - operator
 */
/*--------------------------------------------------------------------------*/
static itemType gettokentype(char * token) 
{
	int i ;

	if (token == NULL) {
		e_error("internal: cannot get token") ;
		return unknown ;
	}

	if (token[0] == '(') return left_bracket ;
	if (token[0] == ')') return right_bracket ;

	/*
	 * Try to match an image:
	 * first, match if the first character is an arobas '@'
	 * second, match if the token contains 'fits' or 'FITS'
	 * Matching in this order allows identifiers such as '@file.fits'
	 */
	if (strstr(token, "@") != NULL) {
		strncpy(token, token+1, (int)strlen(token)-1) ;
		token[(int)strlen(token)-1] = (char)0 ;
		return image ;
	}

	if ((strstr(token, "fits") != NULL) || (strstr(token, "FITS") != NULL)) {
		return image ;
	}

	/* No image nor bracket, if it contains a digit, it is a double */
	i=0 ;
	while ((token[i] != '\0') && (token[i] != ' ')) {
		if (isdigit(token[i])) return number ; 
		i++ ;
	}

	/* No number, no image, it should be an operator */
	if ((token[0] == '+') || (token[0] == '-') || (token[0] == '*') ||
		(token[0] == '/') || (token[0] == '^') || (token[0] == 'l')) {
		return operator ;
	}

	e_error("unrecognized token: [%s]", token) ;
	return unknown ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Operates arithmetic between images and numbers.
  @param	first	First operand.
  @param	second	Second operand.
  @param	op		Operator.
  @return	1 pointer to a newly allocated item.
  Performs the requested operation between the two provided operands and
  returns a pointer to a newly allocated item containing the result.
 */
/*----------------------------------------------------------------------------*/
static item * applyOperator(
        item    *   first,
        item    *   second,
        char 	    op)
{
	item    *   out ;
	int			status ;

	if (first==NULL || second==NULL) return NULL ;

	out = malloc(sizeof(item)) ;
	if ((first->t == number) && (second->t == number)) {
        /* Two NUMBERS */
		out->t = number ;
		switch(op) {
            case '+': out->v.f = first->v.f + second->v.f ; break ;
			case '-': out->v.f = first->v.f - second->v.f ; break ;
			case '*': out->v.f = first->v.f * second->v.f ; break ;
            case '/':
                if (fabs((double)second->v.f) <= 1e-40) {
				    e_error("error: division by zero requested: aborting") ;
				    free(out) ;
				    return NULL;
                }
			    out->v.f = first->v.f / second->v.f ;
			    break ;
			case 'l':
                if ((fabs((double)first->v.f) <= 1e-40) ||
				    (fabs((double)second->v.f) <= 1e-40)) {
				    e_error("logarithm requested on negative or zero value") ;
				    free(out) ;
				    return NULL ;
			    }
			    out->v.f = log((double)first->v.f) / log((double)second->v.f) ;
			    break ;
            case '^':
			    out->v.f = pow((double)first->v.f, (double)second->v.f);
                break ;
			default:
                e_error("unrecognized operator: %c", op) ;
			    free(out) ;
			    return NULL ;
        }
	} else if ((first->t == image) && (second->t == image)) {
        /* Two CUBES */
		out->t = image ;
		if (op!='+' && op!='-' && op!='*' && op!='/') {
			e_error("operation %c is invalid between cubes", op);
			free(out) ;
			return NULL ;
		}
		/* Modify the first cube in the stack */
		if (cube_op(&(first->v.cubept), second->v.cubept, op)!=0) {
			e_error("error during cube arithmetic: aborting") ;
			free(out);
			return NULL ;
		}
		out->v.cubept = first->v.cubept ;
		/* No need for the second one anymore: deallocate it */
		cube_del(second->v.cubept) ;
	} else if ((first->t == image) && (second->t == number)) {
		/* A CUBE and a NUMBER */
		out->t = image ;
		if (op!='+' && op!='-' && op!='*' && op!='/' && op!='^') {
			e_error("unrecognized operation: %c", op) ;
			free(out) ;
			return NULL ;
		}
		if (cube_cst_op(first->v.cubept, second->v.f, op)!=0) {
			e_error("error during cube arithmetic: aborting") ;
			free(out) ;
			return NULL ;
		}
		out->v.cubept = first->v.cubept ;
	} else if ((first->t == number) && (second->t == image)) {
		/* a NUMBER and a CUBE */
		out->t = image ;
		switch(op) {
			case '+':
			case '*':
			case '^':
			status = cube_cst_op(second->v.cubept, first->v.f, op) ;
			out->v.cubept = second->v.cubept ;
			break ;

			/* Subtraction and division need two passes */
			case '-':
			status =  cube_cst_op(second->v.cubept, -1, '*') ;
			status += cube_cst_op(second->v.cubept, first->v.f, '+') ;
			out->v.cubept = second->v.cubept ;
			break ;

			case '/':
			status =  cube_cst_op(second->v.cubept, -1, '^') ;
			status += cube_cst_op(second->v.cubept, first->v.f, '*') ;
			out->v.cubept = second->v.cubept ;
			break ;

			default:
			e_error("unrecognized operation: %c", op) ;
			free(out) ;
			return NULL ;
		}
	} else {
		e_error("type identification error: aborting") ;
		free(out) ;
		return NULL ;
	}
	return out ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Resolves an arithmetic expression in polish notation.
  @param	expr	Arithmetic expression to resolve.
  @param	arithm	Flag indicating if the expression is polish or not.
  @return	1 pointer to a newly allocated item, result of the expression.
  This is the main processing function. It takes in input a character
  string containing an arithmetic expression and resolves it down to a
  single result (if the expression is valid).
  Pass a flag indicating if the give expression is in standard arithmetic
  format, in which case it will be converted to polish before resolving.
 */
/*----------------------------------------------------------------------------*/
static item * ParseExpression(
        char    *   expr,
        int         arithm)
{
	
    char    *   token ;
	item    *	pushvalue ;
	item    *	pop1, 
		    *	pop2 ;	
	int	 	    tokc ;
	int	 	    count ;
	char	    tokv[MAX_OP][FILENAMESZ+1] ; 
	stack	    opStack,
			    polishStack,
			    finalStack ;
	stack   *   mov ;
	int		    SolvedOp ;
	item	*   cur ;
	int		    firstFileName ;
	double	    read_f ;

    /* Initialize */
    SolvedOp = 0 ;
    firstFileName = 1 ;
    
	/* Get first all the tokens into an array: due to a bug in strtok() */
	tokc = 0 ;
	token = strtok(expr, SEPARATOR) ;
	while (token != NULL) {
		strncpy(tokv[tokc], token, FILENAMESZ) ;
		tokc++ ;
		token = strtok(NULL, SEPARATOR) ;
	}

	/* Initialize all stack pointers	*/
	opStack.next = polishStack.next = finalStack.next = NULL ;
	opStack.data = polishStack.data = finalStack.data = NULL ;

	/* Switch according to expression type: standard or polish	*/

	if (arithm == ARITHM_STANDARD) {
		/*
		 * the input arithmetic expression is a standard arithmetic expression 
         * which has already been parsed to separate arguments by spaces.
		 * What is done here is: parse the arithmetic elements one by one and 
         * accumulate them on a stack to be solved by the polish stack solver.
		 */
		for (count=0 ; count<tokc ; count++) {
			token = tokv[count] ;
			switch(gettokentype(token)) {

				case image:
				/* if this is the first seen file name, store it for later */
				if (firstFileName) {
					strncpy(firstName, token, FILENAMESZ) ;
					firstFileName = 0 ;
				}
				/* Simple operands are just pushed on the polish stack */
				pushvalue = malloc(sizeof(item)) ;
				pushvalue->t = image ;
				pushvalue->v.cubept = cube_load(token) ;
				if (pushvalue->v.cubept == NULL) {
					e_error("cannot load %s: aborting", token) ;
					return NULL ;
				}
				push(pushvalue, &polishStack) ;
				break ;

				case number:
				/* Simple operands are just pushed on the polish stack */
				pushvalue = malloc(sizeof(item)) ;
				pushvalue->t = number ;
				sscanf(token, "%lg", &read_f) ; 
				pushvalue->v.f = (double)read_f ;
				push(pushvalue, &polishStack) ;
				break ;

				case left_bracket:
				/* Left brackets are just pushed on the operator stack */
				pushvalue = malloc(sizeof(item)) ;
				pushvalue->t = left_bracket ;
				sscanf(token, "%c", &(pushvalue->v.op)) ;
				push(pushvalue, &opStack) ;
				break ;

				case right_bracket:
				/* 
				 * Pop out everything out of opStack and push it on the polish 
                 * stack, until a left bracket is found
				 */
				cur = pop(&opStack) ;
				while (cur->t != left_bracket) {
					push(cur, &polishStack) ;
					cur = pop(&opStack) ;
					if (cur == NULL) {
						e_error("error in parsing arithmetic expression\n"
								"missing parenthese") ;
						exit(-1) ;
					}	
				}
				free(cur) ;
				break ;

				case operator:
				/*
				 * The whole intelligence of converting a standard arithmetic 
                 * expression into a polish stack goes here.
				 */
				SolvedOp = 0 ;
				while (!SolvedOp) {
					cur = topstack(&opStack) ;
					if (cur == NULL) {
						pushvalue = malloc(sizeof(item)) ;
						pushvalue->t = operator ;
						sscanf(token, "%c", &(pushvalue->v.op)) ;
						push(pushvalue, &opStack) ;
						SolvedOp = 1 ;
					} else if (cur->t == left_bracket) {
						pushvalue = malloc(sizeof(item)) ;
						pushvalue->t = operator ;
						sscanf(token, "%c", &(pushvalue->v.op)) ;
						push(pushvalue, &opStack) ;
						SolvedOp = 1 ;
						free(cur) ;
					} else if (cur->t == operator) {
						if (priority(cur->v.op) >= priority(token[0])) {
							push(pop(&opStack), &polishStack) ;
							free(cur) ;
						} else {
							pushvalue = malloc(sizeof(item)) ;
							pushvalue->t = operator ;
							sscanf(token, "%c", &(pushvalue->v.op)) ;
							push(pushvalue, &opStack) ;
							SolvedOp = 1 ;
							free(cur) ;
						}
					} else {
						e_error("error in operator stack: aborting") ;
						exit(-1) ;
					}	
				}
				break ;

				case unknown:
				default:
				e_error("in arithmetic expression: exiting") ;
				exit(-1) ;
			}
		}	

		/* Now pop every remaining operator on opStack onto polishStack */
		while (opStack.next != NULL) push(pop(&opStack), &polishStack) ;
	
    } else {

		/* Polish stack processing: push first everything onto polishStack */
		for (count=0 ; count<tokc ; count++) {
			token = tokv[count] ;
			switch(gettokentype(token)) {

				case image:
				/* if this is the first seen file name, store it for later */
				if (firstFileName) {
					strncpy(firstName, token, FILENAMESZ) ;
					firstFileName = 0 ;
				}

				/* Simple operands are just pushed on the polish stack */
				pushvalue = malloc(sizeof(item)) ;
				pushvalue->t = image ;
				pushvalue->v.cubept = cube_load(token) ;
				if (pushvalue->v.cubept == NULL) {
					e_error("cannot load %s: aborting", token) ;
					return NULL ;
				}
				push(pushvalue, &polishStack) ;
				break ;

				case number:
				/* Simple operands are just pushed on the polish stack */
				pushvalue = malloc(sizeof(item)) ;
				pushvalue->t = number ;
				sscanf(token, "%lg", &read_f) ;
				pushvalue->v.f = (double)read_f ;
				push(pushvalue, &polishStack) ;
				break ;

				case operator:
				/* Simple operands are just pushed on the polish stack */
				pushvalue = malloc(sizeof(item)) ;
				pushvalue->t = operator ;
				sscanf(token, "%c", &(pushvalue->v.op)) ;
				push(pushvalue, &polishStack) ;
				break ;

				case unknown:
				default:
				e_error("in arithmetic expression: exiting") ;
				exit(-1) ;
			}
		}
	}	


	/* POLISH STACK SOLVING */
	mov = polishStack.next ;
	while (mov != NULL) {
		switch(mov->data->t) {

			/* Simple operands are pushed onto the stack	*/
			case image:
			case number:
			pushvalue = malloc(sizeof(item)) ;
			pushvalue->t = mov->data->t ;
			pushvalue->v = mov->data->v ;
			push(pushvalue, &finalStack) ;
			break ;

			/* 
			 * Operator: pop the two operands on top of the stack, apply
			 * the operator to them, and push the result onto the stack
			 */
			case operator:
			pop2 = pop(&finalStack) ;
			pop1 = pop(&finalStack) ;
			pushvalue = applyOperator(pop1, pop2, mov->data->v.op) ;
			free(pop1) ;
			free(pop2) ;
			if (pushvalue == NULL) {
				e_error("arithmetic error occurred: aborting") ;
				return NULL ;
			}
			push(pushvalue, &finalStack) ;
			break ;

			default:
			e_error("invalid item in polish arithmetic expression") ;
			return NULL ;
			break ;
		}
		mov = mov->next ;
	}

	/* free polishStack */
	while (polishStack.next != NULL) {
		pop1 = pop(&polishStack) ;
		free(pop1) ;
	}	

	/* The final result is the remaining operand on top of the stack */
	return pop(&finalStack) ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Tokenize a standard arithmetic expression.
  @param	arexp	Arithmetic expression in standard format.
  @return	1 pointer to newly allocated string.

  Tokenizes a standard arithmetic expression to help transform it into a
  polish expression. Arguments are identified and separated by blanks.

  Example:

  @code
  tok_exp = tokenizeExpression("2*(@obj-@sky)/ff.fits")
  tok_exp contains then: "2 * ( @obj - @sky ) / ff.fits"
  @endcode

  Tokens are identified by regular expressions. The returned string must be
  deallocated using free().
 */
/*--------------------------------------------------------------------------*/
static char * tokenizeExpression(char * arexp)
{
    int         sz ;
    char    *   ret ;
    char    *   cur ;
    char    *   truncexp ;
    int         beg ;
	int		    matched ;
	regex_t	    re_numb, 
                re_file, 
                re_op, 
                re_par ;
    int         i ;

	/*
	 * Let's make use of regexp
	 * we define four possible objects to be recognized:
	 * numbers, file names, operators, parentheses
	 */
	char number[] = "^[+-]?([0-9]+[.]?[0-9]*|[.][0-9]+)([eE][+-]?[0-9]+)?$" ;
	char filexp[] =
		"^[A-Za-z0-9_.]*[.](fits|FITS)$|^@[A-Za-z0-9_.]*[.]?[fits|FITS]?$" ;
	char op[] = "^[+-/*\\^l]$" ;
	char par[] = "^[\\(\\)]$" ;

    /* Compile rule for number */
    if (regcomp(&re_numb, &number[0], REG_EXTENDED|REG_NOSUB) != 0) {
		e_error("cannot compile rule for number: aborting") ;
       return NULL ;
    }
    /* Compile rule for filename */
    if (regcomp(&re_file, &filexp[0], REG_EXTENDED|REG_NOSUB) != 0) {
		e_error("cannot compile rule for filename: aborting") ;
       return NULL ;
    }
    /* Compile rule for operator */
    if (regcomp(&re_op, &op[0], REG_EXTENDED|REG_NOSUB) != 0) {
		e_error("cannot compile rule for operator: aborting") ;
       return NULL ;
    }
    /* Compile rule for parenthese */
    if (regcomp(&re_par, &par[0], REG_EXTENDED|REG_NOSUB) != 0) {
		e_error("cannot compile rule for parenthese: aborting") ;
       return NULL ;
    }

	/* Strip first white spaces in expression before starting */
	strip_blanks(arexp) ;

	/* Allocate all work strings	*/
    sz 			= (int)strlen(arexp) ;
    ret 		= calloc(2*sz +1, sizeof(char));
    cur 		= calloc(sz+1, sizeof(char));
    truncexp 	= calloc(sz+1, sizeof(char));

	/*
	 * The pattern identification is done in the following way:
	 * try to match the entire string. if it matches, store it
	 * as an identified token, else remove the rightmost character and
	 * try again to match it. Example with an initial string "(@in+2)/3"
	 * INPUT STRING:	MATCH? 
	 * [(@in+2)/3]		no
	 * [(@in+2)/] 		no
	 * [(@in+2)] 		no
	 * [(@in+2] 		no
	 * [(@in+] 			no
	 * [(@in] 			no
	 * [(] 				yes: left_bracket
	 * remove identified token and start again:
	 * [@in+2)/3] 		no
	 * [@in+2)/] 		no
	 * [@in+2)] 		no
	 * [@in+2] 			no
	 * [@in+] 			no
	 * [@in] 			yes: image (@in)
	 * remove identified token and start again:
	 * [+2)/3] 			no
	 * [+2)/] 			no
	 * [+2)]		 	no
	 * [+2] 			no
	 * [+] 				yes: operator (+)
	 * remove identified token and start again:
	 * [2)/3] 			no
	 * [2)/] 			no
	 * [2)] 			no
	 * [2] 				yes: number (2)
	 * remove identified token and start again:
	 * [)/3] 			no
	 * [)/] 			no
	 * [)] 				yes: right_bracket	
	 * remove identified token and start again:
	 * [/3] 			no
	 * [/] 				yes: operator (/)
	 * remove identified token and start again:
	 * [3] 				yes: number (3)
	 * no more characters to work on.
	 */
    beg = 0 ;
    while (beg < sz) {
      memset(truncexp, 0, sz) ;
      memcpy(truncexp, arexp+beg, sz-beg+1) ;
	  matched = 0 ;
      for (i=(int)strlen(truncexp) ; i>0 ; i--) {
            memset(cur, 0, sz) ;
            strncpy(cur, truncexp, i) ;
            /* Try to match a number */
			if ((regexec(&re_numb, cur, 0, NULL, 0) == 0) ||
            /* Try to match a file name */
				(regexec(&re_file, cur, 0, NULL, 0) == 0) ||
            /* Try to match an operator */
				(regexec(&re_op, cur, 0, NULL, 0) == 0) ||
            /* Try to match a parenthese */
				(regexec(&re_par, cur, 0, NULL, 0) == 0)) {
			/* One match found */
              sprintf(ret, "%s %s", ret, cur) ;
              beg += (int)strlen(cur) ;
			  matched = 1 ;
              break ;
			}
		}
	    if (!matched) {
			/*
			 * The whole expression has been reduced to zero characters
			 * without match found.
			 */
			e_error("in arithmetic expression: aborting") ;
			exit(-1) ;
		}
	}

	/* free regexp rules	*/
	regfree(&re_numb) ;
	regfree(&re_file) ;
	regfree(&re_op) ;
	regfree(&re_par) ;
    free(cur) ;
    free(truncexp) ;
    return ret ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Strip blanks off a character string.
  @param	exp
  @return	void
  All blanks in a character string are removed. The input string is modified.
 */
/*----------------------------------------------------------------------------*/
static void strip_blanks(char * exp)
{
	char	*	tmp ;
    int         len ;
	int			i, j ;

    len = (int)strlen(exp);
	tmp = calloc(len+1, sizeof(char)) ;
	j = 0 ;
	for (i=0 ; i<=len ; i++) {
		if (exp[i] != ' ') {
			tmp[j] = exp[i] ;
			j++ ;
		}	
	}
	strncpy(exp, tmp, len) ;
	free(tmp) ;
}

static void usage(char *pname)
{
    hello_world(pname, prog_desc) ;
	printf(
"\n"
"use : %s [options] <arithmetic_expression> [out]\n"
"\n"
"\tFITS files are identified by their .fits or .FITS extension\n"
"\tor by prefixing names with an arobas '@'\n"
"\n"
"options are :\n"
"\t[-s] or [--standard] to use standard arithmetic expressions\n"
"\t[-p] or [--polish] to use polish reverse notation\n"
"\tdefault is polish reverse notation\n"
"see man page about syntax issues\n"
"\n\n", pname);
    exit(0) ;
}

