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
#include "parser.h"
#include "lex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* prototypes */
struct ast_node_t *parseStatement(struct parser_t *parser);
struct ast_node_t *parseIndentifier(struct parser_t *parser);
struct ast_node_t *parseColumnDef(struct parser_t *parser);
struct ast_node_t *parseColumnDef(struct parser_t *parser);
struct ast_node_t *parseExpression(struct parser_t *parser);
struct ast_node_t *parseIndentifier(struct parser_t *parser);
struct ast_node_t *parseCreateDatabase(struct parser_t *parser);
struct ast_node_t *parseCreateTable(struct parser_t *parser);
struct ast_node_t *parseDropTable(struct parser_t *parser);
struct ast_node_t *parseSelect(struct parser_t *parser);
struct ast_node_t *parseWhereClause(struct parser_t *parser);
struct ast_node_t *parseInsert(struct parser_t *parser);
static struct ast_node_t *parseValueList(struct parser_t *parser);

/* Create a new Abstract Syntactical Tree node */
struct ast_node_t *astCreateNode(enum ast_node_type_t type, const char *value) {
    struct ast_node_t *node = malloc(sizeof(struct ast_node_t));
    if (!node)
        return NULL;

    node->type = type;
    node->value = value ? strdup(value) : NULL;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;

    return node;
}

/* Add a child to a node, this method check also the right
 * child_capacity and handle the child_count */
void astAddChild(struct ast_node_t *restrict parent,
                 struct ast_node_t *restrict child) {
    if (!parent || !child)
        return;

    /* In case the parent capacity is full we resize it
     * with capacity * 2 */
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity =
            parent->child_capacity ? parent->child_capacity * 2 : 4;

        parent->children =
            realloc(parent->children,
                    parent->child_capacity * sizeof(struct ast_node_t));
    }

    parent->children[parent->child_count++] = child;
}

/* Recursively release a node and its children */
void astFreeNode(struct ast_node_t *node) {
    if (!node)
        return;

    free(node->value);

    for (size_t i = 0; i < node->child_count; i++) {
        astFreeNode(node->children[i]);
    }

    free(node->children);
    free(node);
}

/* Parser Error Handling */
void parserError(struct parser_t *parser, const char *message) {
    parser->has_error = 1;
    snprintf(parser->error_message, sizeof(parser->error_message),
             "Parse error: %s at token '%s'", message,
             lexGetTokenText(parser->lexer));
}

int parserExpect(struct parser_t *parser, int expected_type) {
    if (lexGetTokenType(parser->lexer) != expected_type) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Expected %s",
                 lexGetTokenTypeName(expected_type));
        parserError(parser, msg);
        return 0;
    }
    return 1;
}

int parserConsume(struct parser_t *parser, int expected_type) {
    if (!parserExpect(parser, expected_type)) {
        return 0;
    }
    lexNextToken(parser->lexer);
    return 1;
}

struct ast_node_t *parseIndentifier(struct parser_t *parser) {
    if (!parserExpect(parser, RSQL_IDENTIFIER))
        return NULL;

    struct ast_node_t *node =
        astCreateNode(AST_IDENTIFIER, lexGetTokenText(parser->lexer));
    lexNextToken(parser->lexer);
    return node;
}

struct ast_node_t *parseColumnDef(struct parser_t *parser) {
    struct ast_node_t *col_def = astCreateNode(AST_COLUMN_DEF, NULL);

    struct ast_node_t *name = parseIndentifier(parser);
    if (!name)
        goto cleanup;
    astAddChild(col_def, name);

    if (lexIsToken(parser->lexer, RSQL_IDENTIFIER)) {
        struct ast_node_t *type = parseIndentifier(parser);
        astAddChild(col_def, type);
    }

    return col_def;

cleanup:
    astFreeNode(col_def);
    return NULL;
}

struct ast_node_t *parseColumnList(struct parser_t *parser) {
    struct ast_node_t *list = astCreateNode(AST_COLUMN_LIST, NULL);

    if (!parserConsume(parser, RSQL_LPAREN))
        goto cleanup;

    struct ast_node_t *col = parseColumnDef(parser);
    if (!col)
        goto cleanup;
    astAddChild(list, col);

    while (lexIsToken(parser->lexer, RSQL_COMMA)) {
        lexNextToken(parser->lexer);

        col = parseColumnDef(parser);
        if (!col)
            goto cleanup;
        astAddChild(list, col);
    }

    if (!parserConsume(parser, RSQL_RPAREN))
        goto cleanup;

    return list;

cleanup:
    astFreeNode(list);
    return NULL;
}

struct ast_node_t *parseExpression(struct parser_t *parser) {
    struct ast_node_t *left = NULL;

    if (lexIsToken(parser->lexer, RSQL_IDENTIFIER)) {
        left = parseIndentifier(parser);
    } else if (lexIsToken(parser->lexer, RSQL_STRING_LITERAL) ||
               lexIsToken(parser->lexer, RSQL_NUMERIC_LITERAL)) {
        left = astCreateNode(AST_LITERAL, lexGetTokenText(parser->lexer));
        lexNextToken(parser->lexer);
    } else {
        parserError(parser, "Expected identifier or literal");
        return NULL;
    }

    // Check for binary operator
    if (lexIsToken(parser->lexer, RSQL_EQ_OP) ||
        lexIsToken(parser->lexer, RSQL_NE_OP) ||
        lexIsToken(parser->lexer, RSQL_LT_OP) ||
        lexIsToken(parser->lexer, RSQL_GT_OP) ||
        lexIsToken(parser->lexer, RSQL_LE_OP) ||
        lexIsToken(parser->lexer, RSQL_GE_OP)) {

        const char *op = lexGetTokenText(parser->lexer);
        enum ast_node_type_t type = AST_OPERATOR;

        struct ast_node_t *op_node = astCreateNode(type, op);
        lexNextToken(parser->lexer);

        struct ast_node_t *right = parseExpression(parser);
        if (!right) {
            astFreeNode(op_node);
            astFreeNode(left);
            return NULL;
        }

        astAddChild(op_node, left);
        astAddChild(op_node, right);
        return op_node;
    }

    return left;
}

/* WHERE
 * =====
 * The statement 'WHERE' syntax is:
 *      WHERE id = 5
 *      WHERE name LIKE 'John%' */
struct ast_node_t *parseWhereClause(struct parser_t *parser) {

    /* chekc if there is WHERE clause (because It's optional) */
    if (!lexIsToken(parser->lexer, WHERE_KW))
        return NULL;

    struct ast_node_t *where_node = astCreateNode(AST_WHERE_CLAUSE, NULL);
    lexNextToken(parser->lexer); /* consume WHERE */

    struct ast_node_t *condition = parseExpression(parser);
    if (!condition)
        goto cleanup;
    astAddChild(where_node, condition);

    return where_node;

cleanup:
    free(where_node);
    return NULL;
}

/* Database creation 'CREATE DATABASE db_name;' */
struct ast_node_t *parseCreateDatabase(struct parser_t *parser) {
    struct ast_node_t *db_node = astCreateNode(AST_CREATE_DATABASE, NULL);

    /* consume 'DATABASE' kw */
    if (!parserConsume(parser, DATABASE_KW))
        goto cleanup;

    struct ast_node_t *db_name = parseIndentifier(parser);
    if (!db_name)
        goto cleanup;
    astAddChild(db_node, db_name);

    return db_node;

cleanup:
    astFreeNode(db_node);
    return NULL;
}

/* Table creation parser e.g. 'CREATE TABLE tb_name (id INT, name TEXT);' */
struct ast_node_t *parseCreateTable(struct parser_t *parser) {
    struct ast_node_t *create_node = astCreateNode(AST_CREATE_TABLE, NULL);

    /* Keyword 'CREATE' is already consumed from caller,
     * so consume the keyword 'TABLE' */
    if (!parserConsume(parser, TABLE_KW))
        goto cleanup;

    /* Parse table identifier 'CREATE TABLE <identifier> ...' */
    struct ast_node_t *table_name = parseIndentifier(parser);
    if (!table_name)
        goto cleanup;
    astAddChild(create_node, table_name);

    /* Parse column list 'CREATE TABLE name (id INT, name VARCHAR);' */
    struct ast_node_t *columns = parseColumnList(parser);
    if (!columns)
        goto cleanup;
    astAddChild(create_node, columns);

    return create_node;

cleanup:
    astFreeNode(create_node);
    return NULL;
}

/* Table deletion e.g. 'DROP TABLE animals;' */
struct ast_node_t *parseDropTable(struct parser_t *parser) {
    struct ast_node_t *drop_node = astCreateNode(AST_DROP_TABLE, NULL);

    /* Consume keyword 'TABLE' after 'DROP' */
    if (!parserConsume(parser, TABLE_KW))
        goto cleanup;

    /* Parse table name 'DROP TABLE <table_name>;' */
    struct ast_node_t *table_name = parseIndentifier(parser);
    if (!table_name)
        goto cleanup;
    astAddChild(drop_node, table_name);

    return drop_node;

cleanup:
    astFreeNode(drop_node);
    return NULL;
}

/* Select statement
 * ================
 * The 'SELECT' syntax based on MySQL standard:
 *
 *      SELECT column1, column2 FROM table_or_view WHERE <condition>;
 *
 *  SELECT: Keyword to get data from a table or a view
 *  COLUMNS: column list or '*' (All)
 *  FROM: Keyword to select the source of data
 *  SRC: table or view
 *  WHERE: Keyword that imposes a condition
 *  CONDITION: an expression */
struct ast_node_t *parseSelect(struct parser_t *parser) {
    struct ast_node_t *select_node = astCreateNode(AST_SELECT, NULL);

    /* Keyword 'SELECT' is already consumed by caller so
     * we need to start with column list or wildcard * */
    if (lexIsToken(parser->lexer, RSQL_MUL_OP)) {
        /* in case of wildcard 'SELECT *' */
        struct ast_node_t *all_cols = astCreateNode(AST_LITERAL, "*");
        astAddChild(select_node, all_cols);
        lexNextToken(parser->lexer);
    } else {
        /* in case 'SELECT column1, column2' parse the first column */
        struct ast_node_t *col = parseIndentifier(parser);
        if (!col)
            goto cleanup;
        astAddChild(select_node, col);

        /* parse the additional columns */
        while (lexIsToken(parser->lexer, RSQL_COMMA)) {
            lexNextToken(parser->lexer); /* Consume comma ',' */
            col = parseIndentifier(parser);
            if (!col)
                goto cleanup;
            astAddChild(select_node, col);
        }
    }

    /* parse the clause FROM (required) */
    if (!parserConsume(parser, FROM_KW))
        goto cleanup;

    /* parse table or view name  */
    struct ast_node_t *table_name = parseIndentifier(parser);
    if (!table_name)
        goto cleanup;
    astAddChild(select_node, table_name);

    /* parse the clause WHERE (optional) */
    struct ast_node_t *where_clause = parseWhereClause(parser);
    if (where_clause)
        astAddChild(select_node, where_clause);

    return select_node;
cleanup:
    astFreeNode(select_node);
    return NULL;
}

/* Parse a list of literals (e.g. ('Marco', 24)) */
static struct ast_node_t *parseValueList(struct parser_t *parser) {
    struct ast_node_t *value_list = astCreateNode(AST_VALUE_LIST, NULL);

    if (!parserConsume(parser, RSQL_LPAREN))
        goto cleanup;

    struct ast_node_t *value = parseExpression(parser);
    if (!value)
        goto cleanup;
    astAddChild(value_list, value);

    while (lexIsToken(parser->lexer, RSQL_COMMA)) {
        lexNextToken(parser->lexer);
        value = parseExpression(parser);
        if (!value)
            goto cleanup;
        astAddChild(value_list, value);
    }

    if (!parserConsume(parser, RSQL_RPAREN))
        goto cleanup;

    return value_list;

cleanup:
    astFreeNode(value_list);
    return NULL;
}

/* INSERT
 * =======
 * The INSERT weyword is used to insert valeus into SQL columsn
 * it has a syntax like:
 *      INSERT INTO tb_name (name, age) VALUES ('Marco', 24);
 *
 * we can also bulk insert more data:
 *      INSERT INTO clienti (denomination, address, phone) VALUES
        ("Barilla S.p.A.","Via Righi 10 Parma","3505712387"),
        ("Parmalat S.p.A.","Via Traverso 15 Parma","3409988776");
 */
struct ast_node_t *parseInsert(struct parser_t *parser) {
    struct ast_node_t *insert_node = astCreateNode(AST_INSERT, NULL);

    /* consume 'INTO' */
    if (!parserConsume(parser, INTO_KW))
        goto cleanup;

    /* parse table name */
    struct ast_node_t *table_name = parseIndentifier(parser);
    if (!table_name)
        goto cleanup;
    astAddChild(insert_node, table_name);

    /* parse column list */
    struct ast_node_t *columns = parseColumnList(parser);
    if (!columns)
        goto cleanup;
    astAddChild(insert_node, columns);

    /* consume VALUES keyword */
    if (!parserConsume(parser, VALUES_KW))
        goto cleanup;

    /* parse first value list */
    struct ast_node_t *value_list = parseValueList(parser);
    if (!value_list)
        goto cleanup;
    astAddChild(insert_node, value_list);

    /* optionally parse more value lists (bulk insert) */
    while (lexIsToken(parser->lexer, RSQL_COMMA)) {
        lexNextToken(parser->lexer);
        value_list = parseValueList(parser);
        if (!value_list)
            goto cleanup;
        astAddChild(insert_node, value_list);
    }

    return insert_node;

cleanup:
    astFreeNode(insert_node);
    return NULL;
}

/* Parse a full SQL statement */
struct ast_node_t *parseStatement(struct parser_t *parser) {
    if (parser->has_error)
        return NULL;

    switch (lexGetTokenType(parser->lexer)) {
    case CREATE_KW:
        lexNextToken(parser->lexer);
        int current_tok = lexGetTokenType(parser->lexer);

        return current_tok == DATABASE_KW ? parseCreateDatabase(parser)
               : current_tok == TABLE_KW  ? parseCreateTable(parser)
                                          : NULL;

    case DROP_KW:
        lexNextToken(parser->lexer);
        return parseDropTable(parser);

    case SELECT_KW:
        lexNextToken(parser->lexer);
        return parseSelect(parser);

    case INSERT_KW:
        lexNextToken(parser->lexer);
        return parseInsert(parser);

    default:
        parserError(parser, "Unexpected token");
        return parseSelect(parser);
    }
}

/* Create a new parser instance */
struct parser_t *parserCreate(struct lexer_t *lexer) {
    struct parser_t *parser = malloc(sizeof(struct parser_t));
    if (!parser)
        return NULL;

    parser->lexer = lexer;
    parser->has_error = 0;
    parser->error_message[0] = '\0';

    return parser;
}

/* Release parser memory */
void parserFree(struct parser_t *parser) {
    if (parser)
        free(parser);
}

/* Main function that gets the first token, parse the statement and
 * check if at the end a semicolon in present */
struct ast_node_t *parserParse(struct parser_t *parser) {
    lexNextToken(parser->lexer);

    struct ast_node_t *root = parseStatement(parser);

    if (root && !lexIsEOF(parser->lexer)) {
        /* Semicolon verification at the end of file */
        if (!parserConsume(parser, RSQL_SEMICOLON)) {
            astFreeNode(root);
            return NULL;
        }
    }

    return root;
}

const char *parserGetError(struct parser_t *parser) {
    return parser->has_error ? parser->error_message : NULL;
}

void astPrintNode(struct ast_node_t *node, int indent) {
    if (!node)
        return;

    for (int i = 0; i < indent; i++)
        printf("  ");

    switch (node->type) {
    case AST_CREATE_TABLE:
        printf("CREATE TABLE\n");
        break;
    case AST_CREATE_DATABASE:
        printf("CRATE DATABASE\n");
        break;
    case AST_DROP_TABLE:
        printf("DROP TABLE\n");
        break;
    case AST_SELECT:
        printf("SELECT\n");
        break;
    case AST_INSERT:
        printf("INSERT\n");
        break;
    case AST_VALUE_LIST:
        printf("VALUE LIST\n");
        break;
    case AST_IDENTIFIER:
        printf("IDENTIFIER: %s\n", node->value ? node->value : "NULL");
        break;
    case AST_COLUMN_LIST:
        printf("COLUMN LIST\n");
        break;
    case AST_COLUMN_DEF:
        printf("COLUMN DEF\n");
        break;
    case AST_WHERE_CLAUSE:
        printf("WHERE CLAUSE\n");
        break;
    case AST_LITERAL:
        printf("LITERAL: %s\n", node->value ? node->value : "NULL");
        break;
    default:
        printf("UNKNOWN NODE\n");
        break;
    }

    for (size_t i = 0; i < node->child_count; i++) {
        astPrintNode(node->children[i], indent + 1);
    }
}
