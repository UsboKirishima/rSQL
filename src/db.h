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
#ifndef _DB_H
#define _DB_H

#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TABLE_NUM 64
#define MAX_COLUMNS_NUM 64
#define MAX_CONSTRAINTS_NUM 4
#define MAX_ROWS_NUM 2048
#define MAX_DB_NUM 32

struct column_t {
    char name[64];
    int type;
    int constraints[MAX_CONSTRAINTS_NUM];
};

union cell_value_t {
    int i;
    char s[256];
};

struct row_t {
    union cell_value_t *cells[MAX_COLUMNS_NUM];
};

struct table_t {
    char name[64];
    struct column_t *columns[MAX_COLUMNS_NUM];
    size_t column_count;
    struct row_t *rows[MAX_ROWS_NUM];
    size_t row_count;
};

struct database_t {
    char name[64];
    struct table_t *tables[MAX_TABLE_NUM];
    size_t table_count;
};

struct ctx_t {
    struct database_t *databases[MAX_DB_NUM];
    size_t database_count;
};

struct ctx_t *dbCreateCtx(void);
struct database_t *dbCreateNew(struct ctx_t *ctx, const char db_name[64]);
void dbReleaseColumns(struct table_t *table);
void dbReleaseRows(struct table_t *table);
void dbReleaseTables(struct database_t *db);
int dbDelete(struct ctx_t *ctx, struct database_t *db);
struct table_t *dbTableNew(struct database_t *db, const char table_name[64]);
int dbTableDelete(struct database_t *db, struct table_t *table);
struct column_t *dbColumnCreate(struct table_t *table, const char col_name[64],
                                int col_type,
                                const int constraints[MAX_CONSTRAINTS_NUM]);
int dbColumnDelete(struct table_t *table, struct column_t *col);
struct row_t *dbRowNew(struct table_t *table);
int dbRowDelete(struct table_t *table, struct row_t *row);

#endif /* _DB_H */
