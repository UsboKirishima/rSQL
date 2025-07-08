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
#include <stdio.h>
#include <string.h>

#include "lex.h"
#include "parser.h"

/* Runs main CLI getting the line buffer */
static void rSQL_runConsole(void) {
    char buffer[100];

    printf("%s", ">> ");
    if (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
    }
}

int main(int argc, char **argv) {
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

    // Array di query di test
    const char *test_queries[] = {
        "CREATE TABLE users (id INT, name VARCHAR);",
        "DROP TABLE users;",
        "SELECT * FROM users;",
        "SELECT id, name FROM users WHERE id;",
        "INSERT INTO users (name, age) VALUES ('Enrico', 13);",
        "INSERT INTO guys (name, age) VALUES ('Marco', 23), ('Davide', '25');",
        NULL // Terminatore array
    };

    // Testa ogni query
    for (int i = 0; test_queries[i]; i++) {
        printf("=== Parsing: %s ===\n", test_queries[i]);

        // Inizializza lexer e parser
        struct lexer_t lexer;
        lexInitialize(&lexer, test_queries[i]);

        struct parser_t *parser = parserCreate(&lexer);
        struct ast_node_t *ast = parserParse(parser);

        if (ast) {
            printf("✓ Parse successful!\n");
            astPrintNode(ast, 0);
            astFreeNode(ast);
        } else {
            printf("✗ Parse failed: %s\n", parserGetError(parser));
        }

        parserFree(parser);
        printf("\n");
    }

    return 0;

    while (1) {
        rSQL_runConsole();
    }

    return 0;
}
