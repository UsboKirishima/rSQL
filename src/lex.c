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
#include "lex.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const keyword_entry_t keywords[] = {
    {"CREATE", CREATE_KW},
    {"DROP", DROP_KW},
    {"DELETE", DELETE_KW},
    {"TRUNCATE", TRUNCATE_KW},
    {"UPDATE", UPDATE_KW},
    {"ALTER", ALTER_KW},
    {"SELECT", SELECT_KW},
    {"INSERT", INSERT_KW},
    {"DATABASE", DATABASE_KW},
    {"TABLE", TABLE_KW},
    {"FROM", FROM_KW},
    {"WHERE", WHERE_KW},
    {"AND", AND_KW},
    {"OR", OR_KW},
    {"NOT", NOT_KW},
    {"BETWEEN", BETWEEN_KW},
    {"LIKE", LIKE_KW},
    {"IN", IN_KW},
    {"IS", IS_KW},
    {"NULL", NULL_KW},
    {"INTO", INTO_KW},
    {"VALUES", VALUES_KW},
    {NULL, 0} /* Sentinel */
};

static const single_char_token_t single_char_tokens[] = {
    {',', RSQL_COMMA, ","},  {';', RSQL_SEMICOLON, ";"},
    {'(', RSQL_LPAREN, "("}, {')', RSQL_RPAREN, ")"},
    {'+', RSQL_ADD_OP, "+"}, {'-', RSQL_SUB_OP, "-"},
    {'*', RSQL_MUL_OP, "*"}, {'/', RSQL_DIV_OP, "/"},
    {'\0', 0, NULL} /* Sentinel */
};

static const multi_char_op_t multi_char_ops[] = {
    {">=", RSQL_GE_OP}, {"<=", RSQL_LE_OP}, {"!=", RSQL_NE_OP},
    {"=", RSQL_ET_OP},  {">", RSQL_GT_OP},  {"<", RSQL_LT_OP},
    {NULL, 0} /* Sentinel */
};

void lexSkipWhiteSpace(struct lexer_t *lexer) {
    while (isspace(lexer->input[lexer->pos])) {
        lexer->pos++;
    }
}

int lexLookupKeyword(const char *text) {
    for (int i = 0; keywords[i].text != NULL; i++) {
        if (strcasecmp(text, keywords[i].text) == 0) {
            return keywords[i].type;
        }
    }
    return RSQL_IDENTIFIER;
}

int lexMatchOperator(struct lexer_t *lexer) {
    /* Try multi-character operators first (longest match) */
    for (int i = 0; multi_char_ops[i].text != NULL; i++) {
        const char *op = multi_char_ops[i].text;
        size_t len = strlen(op);

        if (strncmp(lexer->input + lexer->pos, op, len) == 0) {
            strncpy(lexer->current_token.text, op, len);
            lexer->current_token.text[len] = '\0';
            lexer->current_token.type = multi_char_ops[i].type;
            lexer->pos += len;
            return 1;
        }
    }
    return 0;
}

int lexMatchSingleChar(struct lexer_t *lexer) {
    char c = lexer->input[lexer->pos];

    for (int i = 0; single_char_tokens[i].text != NULL; i++) {
        if (c == single_char_tokens[i].ch) {
            lexer->current_token.type = single_char_tokens[i].type;
            strcpy(lexer->current_token.text, single_char_tokens[i].text);
            lexer->pos++;
            return 1;
        }
    }
    return 0;
}

void lexParseIdentifier(struct lexer_t *lexer) {
    size_t start = lexer->pos;

    while (isalnum(lexer->input[lexer->pos]) ||
           lexer->input[lexer->pos] == '_') {
        lexer->pos++;
    }

    size_t len = lexer->pos - start;
    if (len >= sizeof(lexer->current_token.text))
        len = sizeof(lexer->current_token.text) - 1;

    strncpy(lexer->current_token.text, lexer->input + start, len);
    lexer->current_token.text[len] = '\0';

    /* Check if it's a keyword */
    lexer->current_token.type = lexLookupKeyword(lexer->current_token.text);
}

void lexParseNumber(struct lexer_t *lexer) {
    size_t start = lexer->pos;
    int dot_seen = 0;

    while (isdigit(lexer->input[lexer->pos]) ||
           (!dot_seen && lexer->input[lexer->pos] == '.')) {
        if (lexer->input[lexer->pos] == '.')
            dot_seen = 1;
        lexer->pos++;
    }

    size_t len = lexer->pos - start;
    if (len >= sizeof(lexer->current_token.text))
        len = sizeof(lexer->current_token.text) - 1;

    strncpy(lexer->current_token.text, lexer->input + start, len);
    lexer->current_token.text[len] = '\0';

    lexer->current_token.type = RSQL_NUMERIC_LITERAL;
}

void lexParseString(struct lexer_t *lexer) {
    lexer->pos++; // Skip initial quote
    size_t start = lexer->pos;

    while (lexer->input[lexer->pos] != '\0' &&
           lexer->input[lexer->pos] != '\'') {
        lexer->pos++;
    }

    size_t len = lexer->pos - start;
    if (len >= sizeof(lexer->current_token.text))
        len = sizeof(lexer->current_token.text) - 1;

    strncpy(lexer->current_token.text, lexer->input + start, len);
    lexer->current_token.text[len] = '\0';

    lexer->current_token.type = RSQL_STRING_LITERAL;

    if (lexer->input[lexer->pos] == '\'') {
        lexer->pos++; // Skip closing quote
    } else {
        // Unterminated string — could set RSQL_UNKNOWN or flag an error
    }
}

void lexNextToken(struct lexer_t *lexer) {
    lexSkipWhiteSpace(lexer);
    char c = lexer->input[lexer->pos];

    /* End of file */
    if (c == '\0') {
        lexer->current_token.type = RSQL_EOF;
        strcpy(lexer->current_token.text, "EOF");
        return;
    }

    /* Try to match multi-character operators */
    if (lexMatchOperator(lexer)) {
        return;
    }

    /* Try to match single character tokens */
    if (lexMatchSingleChar(lexer)) {
        return;
    }

    /* String literal */
    if (c == '\'') {
        lexParseString(lexer);
        return;
    }

    /* Numeric literal */
    if (isdigit(c)) {
        lexParseNumber(lexer);
        return;
    }

    /* Identifier or keyword */
    if (isalpha(c) || c == '_') {
        lexParseIdentifier(lexer);
        return;
    }

    /* Unknown token */
    lexer->current_token.type = RSQL_UNKNOWN;
    lexer->current_token.text[0] = c;
    lexer->current_token.text[1] = '\0';
    lexer->pos++;
}

/* Utility function to initialize lexer */
void lexInitialize(struct lexer_t *lexer, const char *input) {
    lexer->input = input;
    lexer->pos = 0;
    lexer->current_token.type = RSQL_UNKNOWN;
    lexer->current_token.text[0] = '\0';
}

/* Utility function to get token type name for debugging */
const char *lexGetTokenTypeName(int type) {
    switch (type) {
    case RSQL_EOF:
        return "EOF";
    case RSQL_COMMA:
        return "COMMA";
    case RSQL_SEMICOLON:
        return "SEMICOLON";
    case RSQL_LPAREN:
        return "LPAREN";
    case RSQL_RPAREN:
        return "RPAREN";
    case RSQL_IDENTIFIER:
        return "IDENTIFIER";
    case RSQL_UNKNOWN:
        return "UNKNOWN";
    case CREATE_KW:
        return "CREATE";
    case DROP_KW:
        return "DROP";
    case DELETE_KW:
        return "DELETE";
    case INTO_KW:
        return "INTO";
    case VALUES_KW:
        return "VALUES";
    case SELECT_KW:
        return "SELECT";
    case RSQL_ET_OP:
        return "EQUAL";
    case RSQL_NE_OP:
        return "NOT_EQUAL";
    case RSQL_GT_OP:
        return "GREATER";
    case RSQL_GE_OP:
        return "GREATER_EQUAL";
    case RSQL_LT_OP:
        return "LESS";
    case RSQL_LE_OP:
        return "LESS_EQUAL";
    case RSQL_ADD_OP:
        return "PLUS";
    case RSQL_SUB_OP:
        return "MINUS";
    case RSQL_MUL_OP:
        return "MULTIPLY";
    case RSQL_DIV_OP:
        return "DIVIDE";
    case RSQL_STRING_LITERAL:
        return "STRING_LITERAL";
    case RSQL_NUMERIC_LITERAL:
        return "NUMERIC_LITERAL";
    default:
        return "UNKNOWN_TYPE";
    }
}

/* Example usage and test function */
#ifdef TEST_LEXER
int main() {
    struct lexer_t lexer;
    const char *test_input =
        "CREATE TABLE users (id >= 10 AND name != 'test');";

    lexInitialize(&lexer, test_input);

    printf("Tokenizing: %s\n\n", test_input);

    do {
        lexNextToken(&lexer);
        printf("Token: %-15s Type: %s\n", lexer.current_token.text,
               lexGetTokenTypeName(lexer.current_token.type));
    } while (lexer.current_token.type != RSQL_EOF);

    return 0;
}
#endif
