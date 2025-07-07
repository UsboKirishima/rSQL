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
#include "db.h"
#include "lex.h"
#include "logs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* AST Node Types */
enum ast_node_type_t {
    AST_STATEMENT,
    AST_CREATE_TABLE,
    AST_DROP_TABLE,
    AST_SELECT,
    AST_INSERT,
    AST_UPDATE,
    AST_DELETE,
    AST_IDENTIFIER,
    AST_COLUMN_LIST,
    AST_COLUMN_DEF,
    AST_WHERE_CLAUSE,
    AST_EXPRESSION,
    AST_BINARY_OP,
    AST_LITERAL,
    AST_TABLE_REF
};

/* AST Node Structure is a node used by parser to rapresent the
 * flow of code. */
struct ast_node_t {
    /* node type is another identifier to rapresent a keyword
     * or another syntactical operator */
    enum ast_node_type_t type;

    /* different from the type, the value is the real token
     * rapresented in string */
    char *value;

    /* Children are all the nodes after the triggered value
     * for example if we get "CREATE TABLE tb_name;":
     * CREATE TABLE                 (root node)
     *      IDENTIFIER: tb_name     (child) */
    struct ast_node_t **children;
    size_t child_count;

    /* Child capacity is the maximium recursive limit,
     * for example if we still have "CREATE TABLE tb_name;"
     * the child_capacity will be 1 */
    size_t child_capacity;
};

struct parser_t {
    struct lexer_t *lexer;
    int has_error;
    char error_message[256];
};

struct ctx_t *__ctx = NULL;

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

/* prototypes */
struct ast_node_t *parseIndentifier(struct parser_t *parser);
struct ast_node_t *parseColumnDef(struct parser_t *parser);

struct ast_node_t *parseIndentifier(struct parser_t *parser) {
    if (parserExpect(parser, RSQL_IDENTIFIER))
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

cleanup:
    astFreeNode(col_def);
    return NULL;
}

/* This function is used to check if the global context variable
 * exists and in case to create it and parse errors and logs. */
struct ctx_t *rSQL_getCtx(void) {
    /* In case of ctx already exists returns it */
    if (__ctx)
        return __ctx;

    __ctx = dbCreateCtx();

    if (!__ctx) {
        LOG_ERROR("Failed to create database context.");
        exit(EXIT_FAILURE);
    }

    return __ctx;
};
