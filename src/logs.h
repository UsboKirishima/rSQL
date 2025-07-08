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
#ifndef _LOGS_H
#define _LOGS_H

#include <stdio.h>
#include <time.h>

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[1;31m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_CYAN "\033[1;36m"

/* Timestamp */
#define LOG_TIMESTAMP()                                                        \
    do {                                                                       \
        time_t t = time(NULL);                                                 \
        struct tm *tm_info = localtime(&t);                                    \
        char buf[20];                                                          \
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);              \
        printf("[%s] ", buf);                                                  \
    } while (0)

#define LOG_INFO(msg, ...)                                                     \
    do {                                                                       \
        printf(COLOR_GREEN "[INFO] " COLOR_RESET msg "\n", ##__VA_ARGS__);     \
    } while (0)

#define LOG_WARN(msg, ...)                                                     \
    do {                                                                       \
        LOG_TIMESTAMP();                                                       \
        printf(COLOR_YELLOW "[WARN] " COLOR_RESET msg "\n", ##__VA_ARGS__);    \
    } while (0)

#define LOG_ERROR(msg, ...)                                                    \
    do {                                                                       \
        LOG_TIMESTAMP();                                                       \
        fprintf(stderr, COLOR_RED "[ERROR] " COLOR_RESET msg "\n",             \
                ##__VA_ARGS__);                                                \
    } while (0)

#define LOG_DEBUG(msg, ...)                                                    \
    do {                                                                       \
        LOG_TIMESTAMP();                                                       \
        printf(COLOR_BLUE "[DEBUG] " COLOR_RESET msg "\n", ##__VA_ARGS__);     \
    } while (0)

#endif /* _LOGS_H */
