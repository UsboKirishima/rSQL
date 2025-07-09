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
 *
 * ---------------------------------------------------------------------------
 *  This is module is used to visit all the Abstract Syntax Tree (AST) and
 *  for each instruction making a valid SQL logic using `db.c`.
 *  It's a very simple design implementation, `db.c` module provides
 *  many useful library functions that permits to us to manage the basic
 *  SQL data structure. So we could have just one context, not present in
 *  SQL but used to rapresent an instance of `db.c` module. Context includes
 *  different databases (NOTE maybe in future, context will contains multiple
 *  connections, and a connection includes multiple databases). One database
 *  could include zero or multiple tables that include columns and rows.
 *
 *  Originally-authored-by: Davide Usberti <usbertibox@gmail.com>
 */
#include "eval.h"
#include "db.h"
#include "lex.h"
#include "logs.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>

struct ctx_t *context = NULL;

/* Basic function to handle the global variable context,
 * if the context exists returns it, else creates it*/
struct ctx_t *evGetContext() {
    if (!context)
        context = dbCreateCtx();
    return context;
}

/* Evaluator initialization */
evaluator_t *evCreateEvaluator(char *input) {

    /* Lexer initialization */
    struct lexer_t lexer;
    lexInitialize(&lexer, input);

    /* Parser initialization */
    struct parser_t *parser = parserCreate(&lexer);
    struct ast_node_t *ast = parserParse(parser);

    /* Evaluator initialization */
    evaluator_t *eval = malloc(sizeof(evaluator_t));
    memset(eval, 0, sizeof(evaluator_t));

    eval->parser = parser;
    eval->current_node = ast;

    if (!ast) {
        const char *err = parserGetError(parser);
        if (err) {
            eval->errors = strdup(err);
            LOG_ERROR("Failed to parse: %s", err);
        }
    }

    return eval;
}

void evEvaluateNode(struct ast_node_t *node) {

    if (!node)
        LOG_ERROR("Falied to parse node (NULL)");

    struct ctx_t *ctx = evGetContext();

    switch (node->type) {
    case AST_CREATE_DATABASE:
        if (node->child_count != 1) {
            LOG_ERROR(
                "CRATE DATABASE only accepts 1 identifier (database_name).");
            return;
        }
        struct ast_node_t *db_name_node = node->children[0];
        char *db_name =
            db_name_node->type == AST_IDENTIFIER ? db_name_node->value : NULL;

        if (!db_name) {
            LOG_ERROR("Invalid db_name provided");
        }

        struct database_t *new_database = dbCreateNew(ctx, db_name);
        LOG_INFO("New Database %s created successfully", new_database->name);
        break;
    default:
        LOG_ERROR("Invalid AST type");
        return;
    }
}

/* Frees an evaluator and all associated resources */
void evReleaseEvaluator(evaluator_t *evaluator) {
    if (!evaluator)
        return;

    if (evaluator->parser)
        parserFree(evaluator->parser);

    /* Lexer is not allocated, so we don't need to release it */

    if (evaluator->errors)
        free(evaluator->errors);

    free(evaluator);
}
