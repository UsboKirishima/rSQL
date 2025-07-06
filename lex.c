#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Error codes */
#define RSQL_ERR 0
#define RSQL_OK  1

/* General keyword code */
#define RSQL_KEYWORD  0

/* Logic operators */
#define RSQL_AND_OP 0xf000
#define RSQL_OR_OP  0xf001
#define RSQL_NOT_OP 0xf002

/* Comparison operators */
#define RSQL_ET_OP 0xf003 /* =  | 'Equal To' operator */
#define RSQL_NE_OP 0xf004 /* != | 'Not Equal To' operator */
#define RSQL_GT_OP 0xf005 /* >  | 'Greater Than' operator */
#define RSQL_GE_OP 0xf006 /* >= | 'Greater or Equal Than' operator */
#define RSQL_LT_OP 0xf007 /* <  | 'Less Than' operator */
#define RSQL_LE_OP 0xf008 /* <= | 'Less or Equal Than', oeprator */

#define RSQL_BEETWEEN_OP 0xf009
#define RSQL_LIKE_OP     0xf00a
#define RSQL_IN_OP       0xf00b
#define RSQL_IS_NULL_OP  0xf00c

/* Aritmetic operators */
#define RSQL_EQ_OP  0xf00d /* Is the = operator, but used in expressions */
#define RSQL_SUB_OP 0xf00e /* - | Subtraction */
#define RSQL_MUL_OP 0xf00f /* * | Multiplication */
#define RSQL_ADD_OP 0xf010 /* + | Addition */
#define RSQL_DIV_OP 0xf011 /* / | Division */

#define RSQL_EOF        0x1001 /* \0 | End Of File */
#define RSQL_COMMA      0x1002 /* ,  | Comma */
#define RSQL_SEMICOLON  0x1003 /* ;  | Semicolon */
#define RSQL_UNKNOWN    0x1004
#define RSQL_IDENTIFIER 0x1005

struct token_t {
        int type;
        char text[64];
};

struct lexer_t {
        const char *input;
        size_t pos;
        struct token_t current_token;
};

void lexSkipWhiteSpace(struct lexer_t *lexer) {
        while (isspace(lexer->input[lexer->pos])) {
                lexer->pos++;
        }
}

int lexMatchKeyword(const char* input, size_t pos, const char* keyword) {
        size_t len = strlen(keyword);
        for (size_t i = 0; i < len; i++) {
                if (toupper(input[pos+i]) != keyword[i]) {
                        return 0;
                }
        }
        /* Check non alphanumeric after keyword */
        char next = input[pos+len];
        if (isalnum(next) || next == '_') return 0;
        return 1;
}

void lexNextToken(struct lexer_t *lexer) {
        lexSkipWhiteSpace(lexer);
        char c = lexer->input[lexer->pos];

        if (c == '\0') {
                lexer->current_token.type = RSQL_EOF;
                strcpy(lexer->current_token.text, "EOF");
                return;
        }

        /* Punctuation */
        if (c == ',') {
                lexer->current_token.type = RSQL_COMMA;
                strcpy(lexer->current_token.text, ",");
                lexer->pos++;
                return;
        }
        if (c == ';') {
                lexer->current_token.type = RSQL_SEMICOLON;
                strcpy(lexer->current_token.text, ";");
                lexer->pos++;
                return;
        }

        /* Identifier (alphanumeric + _) */
        if (isalpha(c) || c == '_') {
                size_t start = lexer->pos;
                while (isalnum(lexer->input[lexer->pos]) || lexer->input[lexer->pos] == '_') {
                        lexer->pos++;
                }

                size_t len = lexer->pos - start;
                if (len >= sizeof(lexer->current_token.text))
                        len = sizeof(lexer->current_token.text) - 1;
                
                strncpy(lexer->current_token.text, lexer->input + start, len);
                lexer->current_token.text[len] = '\0';
                lexer->current_token.type = RSQL_IDENTIFIER;
                return;
        }

        /* Unknown token */
        lexer->current_token.type = RSQL_UNKNOWN;
        lexer->current_token.text[0] = c;
        lexer->current_token.text[1] = '\0';
        lexer->pos++;
}
