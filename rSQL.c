#include <stdio.h>
#include <string.h>

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
