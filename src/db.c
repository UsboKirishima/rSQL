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

void dbReleaseColumns(struct table_t *table);
void dbReleaseRows(struct table_t *table);
void dbReleaseTables(struct database_t *db);

struct ctx_t *dbCreateCtx(void) {
    struct ctx_t *context = malloc(sizeof(struct ctx_t));
    if (!context) return NULL;
    context->database_count = 0;
    memset(context->databases, 0, sizeof(context->databases));
    return context;
}

struct database_t *dbCreateNew(struct ctx_t *ctx, const char db_name[64]) {
    if (ctx->database_count >= MAX_DB_NUM) return NULL;

    struct database_t *new_db = malloc(sizeof(struct database_t));
    if (!new_db) return NULL;

    memset(new_db, 0, sizeof(struct database_t));
    strncpy(new_db->name, db_name, 63);
    new_db->name[63] = '\0';

    ctx->databases[ctx->database_count] = new_db;
    ctx->database_count++;
    return new_db;
}

void dbReleaseColumns(struct table_t *table) {
    if (!table) return;

    for (size_t i = 0; i < table->column_count; i++) {
        if (table->columns[i]) {
            free(table->columns[i]);
            table->columns[i] = NULL;
        }
    }

    table->column_count = 0;
}

void dbReleaseRows(struct table_t *table) {
    if (!table) return;

    for (size_t i = 0; i < table->row_count; i++) {
        struct row_t *row = table->rows[i];
        if (!row) continue;

        for (size_t c = 0; c < table->column_count; c++) {
            if (row->cells[c]) {
                free(row->cells[c]);
                row->cells[c] = NULL;
            }
        }

        free(row);
        table->rows[i] = NULL;
    }

    table->row_count = 0;
}

void dbReleaseTables(struct database_t *db) {
    if (!db) return;

    for (size_t i = 0; i < db->table_count; i++) {
        struct table_t *table = db->tables[i];
        if (!table) continue;

        dbReleaseColumns(table);
        dbReleaseRows(table);
        free(table);
        db->tables[i] = NULL;
    }

    db->table_count = 0;
}

int dbDelete(struct ctx_t *ctx, struct database_t *db) {
    if (!ctx || !db) return 0;

    size_t idx = MAX_DB_NUM;
    for (size_t i = 0; i < ctx->database_count; i++) {
        if (ctx->databases[i] == db) {
            idx = i;
            break;
        }
    }

    if (idx == MAX_DB_NUM) return 0;

    dbReleaseTables(db);
    free(db);

    for (size_t i = idx; i + 1 < ctx->database_count; i++) {
        ctx->databases[i] = ctx->databases[i + 1];
    }

    ctx->databases[ctx->database_count - 1] = NULL;
    ctx->database_count--;
    return 1;
}

struct table_t *dbTableNew(struct database_t *db, const char table_name[64]) {
    if (db->table_count >= MAX_TABLE_NUM) return NULL;

    struct table_t *new_table = malloc(sizeof(struct table_t));
    if (!new_table) return NULL;

    memset(new_table, 0, sizeof(struct table_t));
    strncpy(new_table->name, table_name, 63);
    new_table->name[63] = '\0';

    db->tables[db->table_count] = new_table;
    db->table_count++;
    return new_table;
}

int dbTableDelete(struct database_t *db, struct table_t *table) {
    if (!db || !table) return 0;

    size_t idx = MAX_TABLE_NUM;
    for (size_t i = 0; i < db->table_count; i++) {
        if (db->tables[i] == table) {
            idx = i;
            break;
        }
    }

    if (idx == MAX_TABLE_NUM) return 0;

    dbReleaseColumns(table);
    dbReleaseRows(table);
    free(table);

    for (size_t i = idx; i + 1 < db->table_count; i++) {
        db->tables[i] = db->tables[i + 1];
    }

    db->tables[db->table_count - 1] = NULL;
    db->table_count--;
    return 1;
}

struct column_t *dbColumnCreate(struct table_t *table, const char col_name[64], 
    int col_type, const int constraints[MAX_CONSTRAINTS_NUM]) {
    if (table->column_count >= MAX_COLUMNS_NUM) return NULL;

    struct column_t *new_col = malloc(sizeof(struct column_t));
    if (!new_col) return NULL;

    memset(new_col, 0, sizeof(struct column_t));
    strncpy(new_col->name, col_name, 63);
    new_col->name[63] = '\0';
    new_col->type = col_type;

    if (constraints) {
        memcpy(new_col->constraints, constraints, sizeof(int) * MAX_CONSTRAINTS_NUM);
    } else {
        memset(new_col->constraints, 0, sizeof(int) * MAX_CONSTRAINTS_NUM);
    }

    table->columns[table->column_count] = new_col;
    table->column_count++;
    return new_col;
}

int dbColumnDelete(struct table_t *table, struct column_t *col) {
    if (!table || !col) return 0;

    size_t idx = MAX_COLUMNS_NUM;
    for (size_t i = 0; i < table->column_count; i++) {
        if (table->columns[i] == col) {
            idx = i;
            break;
        }
    }

    if (idx == MAX_COLUMNS_NUM) return 0;

    free(col);

    for (size_t i = idx; i + 1 < table->column_count; i++) {
        table->columns[i] = table->columns[i + 1];
    }

    table->columns[table->column_count - 1] = NULL;
    table->column_count--;
    return 1;
}

struct row_t *dbRowNew(struct table_t *table) {
    if (!table || table->row_count >= MAX_ROWS_NUM) return NULL;

    struct row_t *new_row = malloc(sizeof(struct row_t));
    if (!new_row) return NULL;

    memset(new_row, 0, sizeof(struct row_t));

    for (size_t i = 0; i < table->column_count; i++) {
        new_row->cells[i] = malloc(sizeof(union cell_value_t));

        if (!new_row->cells[i]) {
            for (size_t j = 0; j < i; j++) {
                free(new_row->cells[j]);
            }

            free(new_row);
            return NULL;
        }

        memset(new_row->cells[i], 0, sizeof(union cell_value_t));
    }

    table->rows[table->row_count] = new_row;
    table->row_count++;
    return new_row;
}

int dbRowDelete(struct table_t *table, struct row_t *row) {
    if (!table || !row) return 0;

    size_t idx = MAX_ROWS_NUM;
    for (size_t i = 0; i < table->row_count; i++) {
        if (table->rows[i] == row) {
            idx = i;
            break;
        }
    }

    if (idx == MAX_ROWS_NUM) return 0;
    for (size_t c = 0; c < table->column_count; c++) {
        if (row->cells[c]) {
            free(row->cells[c]);
            row->cells[c] = NULL;
        }
    }

    free(row);

    for (size_t i = idx; i + 1 < table->row_count; i++) {
        table->rows[i] = table->rows[i + 1];
    }

    table->rows[table->row_count - 1] = NULL;
    table->row_count--;
    return 1;
}
