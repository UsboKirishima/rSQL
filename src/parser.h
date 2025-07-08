#ifndef _PARSER_H
#define _PARSER_H

#include <stdlib.h>

/* AST Node Types */
enum ast_node_type_t {
    AST_STATEMENT,
    AST_CREATE_DATABASE,
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
    AST_TABLE_REF,
    AST_VALUE_LIST,
    AST_OPERATOR,
    AST_VALUES
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

void astFreeNode(struct ast_node_t *node);

struct ast_node_t *parseStatement(struct parser_t *parser);
struct ast_node_t *parseIndentifier(struct parser_t *parser);
struct ast_node_t *parseColumnDef(struct parser_t *parser);
struct ast_node_t *parseColumnDef(struct parser_t *parser);
struct ast_node_t *parseExpression(struct parser_t *parser);
struct ast_node_t *parseIndentifier(struct parser_t *parser);
struct ast_node_t *parseCreateTable(struct parser_t *parser);
struct ast_node_t *parseDropTable(struct parser_t *parser);
struct ast_node_t *parseSelect(struct parser_t *parser);
struct ast_node_t *parseWhereClause(struct parser_t *parser);
struct ast_node_t *parseInsert(struct parser_t *parser);

struct parser_t *parserCreate(struct lexer_t *lexer);
void parserFree(struct parser_t *parser);
struct ast_node_t *parserParse(struct parser_t *parser);
const char *parserGetError(struct parser_t *parser);
void astPrintNode(struct ast_node_t *node, int indent);

#endif /* _PARSER_H */
