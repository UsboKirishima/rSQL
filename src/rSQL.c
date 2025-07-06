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

#include "db.h"

/* Runs main CLI getting the line buffer */
static void rSQL_runConsole(void) {
    char buffer[100];
    
    printf("%s", ">> ");
    if (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
    }
}

int main(int argc, char **argv) {
    

    while (1) {
        rSQL_runConsole();
    }

    return 0;
}
