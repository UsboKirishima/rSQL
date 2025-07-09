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
 *  Evaluator interface for traversing the Abstract Syntax Tree (AST) and
 *  generating SQL logic using the `db` module. The evaluator provides a
 *  context-aware interpretation layer between AST and the database system.
 */

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "db.h"
#include "parser.h"

typedef struct {
    struct parser_t *parser; /* contains lexer and AST  */
    struct ast_node_t *current_node;
    char *errors;
} evaluator_t;

struct ctx_t *evGetContext(void);
evaluator_t *evCreateEvaluator(char *input);
void evEvaluateNode(struct ast_node_t *node);
void evReleaseEvaluator(evaluator_t *evaluator);

#endif /* EVALUATOR_H */
