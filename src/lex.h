/*
 * Copyright 2025 Davide Usberti <usbertibox@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LEX_H
#define _LEX_H

#include <stddef.h>

/* Error codes */
#define RSQL_ERR 0
#define RSQL_OK 1

/* General keyword code */
#define RSQL_KEYWORD 0

/* Logic operators */
#define RSQL_AND_OP 0xf000
#define RSQL_OR_OP 0xf001
#define RSQL_NOT_OP 0xf002

/* Comparison operators */
#define RSQL_ET_OP 0xf003 /* =  | 'Equal To' operator */
#define RSQL_NE_OP 0xf004 /* != | 'Not Equal To' operator */
#define RSQL_GT_OP 0xf005 /* >  | 'Greater Than' operator */
#define RSQL_GE_OP 0xf006 /* >= | 'Greater or Equal Than' operator */
#define RSQL_LT_OP 0xf007 /* <  | 'Less Than' operator */
#define RSQL_LE_OP 0xf008 /* <= | 'Less or Equal Than' operator */

#define RSQL_BETWEEN_OP 0xf009
#define RSQL_LIKE_OP 0xf00a
#define RSQL_IN_OP 0xf00b
#define RSQL_IS_NULL_OP 0xf00c

/* Arithmetic operators */
#define RSQL_EQ_OP 0xf00d  /* Is the = operator, but used in expressions */
#define RSQL_SUB_OP 0xf00e /* - | Subtraction */
#define RSQL_MUL_OP 0xf00f /* * | Multiplication */
#define RSQL_ADD_OP 0xf010 /* + | Addition */
#define RSQL_DIV_OP 0xf011 /* / | Division */

/* Punctuation and special tokens */
#define RSQL_EOF 0x1001       /* \0 | End Of File */
#define RSQL_COMMA 0x1002     /* ,  | Comma */
#define RSQL_SEMICOLON 0x1003 /* ;  | Semicolon */
#define RSQL_LPAREN 0x1004    /* (  | Left Parenthesis */
#define RSQL_RPAREN 0x1005    /* )  | Right Parenthesis */
#define RSQL_UNKNOWN 0x1006
#define RSQL_IDENTIFIER 0x1007

#define RSQL_TICK 0x1008 /* '  | Tick' */
#define RSQL_STRING_LITERAL 0x1009
#define RSQL_NUMERIC_LITERAL 0x100a

/* Keywords */
#define CREATE_KW 0x2001
#define DROP_KW 0x2002
#define DELETE_KW 0x2003
#define TRUNCATE_KW 0x2004
#define UPDATE_KW 0x2005
#define ALTER_KW 0x2006
#define SELECT_KW 0x2007
#define INSERT_KW 0x2008
#define DATABASE_KW 0x2009
#define TABLE_KW 0x200a
#define FROM_KW 0x200b
#define WHERE_KW 0x200c
#define AND_KW 0x200d
#define OR_KW 0x200e
#define NOT_KW 0x200f
#define BETWEEN_KW 0x2010
#define LIKE_KW 0x2011
#define IN_KW 0x2012
#define IS_KW 0x2013
#define NULL_KW 0x2014
#define INTO_KW 0x2015
#define VALUES_KW 0x2016

/* Maximum token text length */
#define RSQL_MAX_TOKEN_LENGTH 64

struct token_t {
    int type; /* Token type (one of the constants above) */
    char text[RSQL_MAX_TOKEN_LENGTH]; /* Token text content */
};

struct lexer_t {
    const char *input;            /* Input string to tokenize */
    size_t pos;                   /* Current position in input */
    struct token_t current_token; /* Current token */
};

typedef struct {
    const char *text;
    int type;
} keyword_entry_t;

typedef struct {
    char ch;
    int type;
    const char *text;
} single_char_token_t;

typedef struct {
    const char *text;
    int type;
} multi_char_op_t;

/* Function declarations */

/* Initialize the lexer with input string */
void lexInitialize(struct lexer_t *lexer, const char *input);

/* Get the next token from the input */
void lexNextToken(struct lexer_t *lexer);

/* Skip whitespace characters in the input */
void lexSkipWhiteSpace(struct lexer_t *lexer);

/* Lookup a keyword in the keyword table */
int lexLookupKeyword(const char *text);

/* Try to match multi-character operators */
int lexMatchOperator(struct lexer_t *lexer);

/* Try to match single character tokens */
int lexMatchSingleChar(struct lexer_t *lexer);

/* Parse an identifier or keyword */
void lexParseIdentifier(struct lexer_t *lexer);

/* Get human-readable token type name for debugging */
const char *lexGetTokenTypeName(int type);

/* Check if current token is of specified type */
static inline int lexIsToken(struct lexer_t *lexer, int type) {
    return lexer->current_token.type == type;
}

/* Check if current token is a keyword */
static inline int lexIsKeyword(struct lexer_t *lexer) {
    return lexer->current_token.type >= 0x2000 &&
           lexer->current_token.type < 0x3000;
}

/* Check if current token is an operator */
static inline int lexIsOperator(struct lexer_t *lexer) {
    return lexer->current_token.type >= 0xf000 &&
           lexer->current_token.type < 0x1000;
}

/* Check if current token is punctuation */
static inline int lexIsPunctuation(struct lexer_t *lexer) {
    return lexer->current_token.type >= 0x1000 &&
           lexer->current_token.type < 0x2000;
}

/* Get current token text */
static inline const char *lexGetTokenText(struct lexer_t *lexer) {
    return lexer->current_token.text;
}

/* Get current token type */
static inline int lexGetTokenType(struct lexer_t *lexer) {
    return lexer->current_token.type;
}

/* Check if lexer has reached end of input */
static inline int lexIsEOF(struct lexer_t *lexer) {
    return lexer->current_token.type == RSQL_EOF;
}

#endif /* _LEX_H */
