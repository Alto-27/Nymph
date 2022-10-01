#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

int main(int argc, char * argv[]) {
    FILE * fptr;
    //char buffer[1024];
    uint16_t opcode_test = 0x4AF0;
    printf("%04X\n", opcode_test);

    fptr = fopen("nymph.log", "w");

    if (fptr == NULL) {
        printf("Error!");
        exit(1);
    }

    fprintf(fptr, "Use fprintf for logging in Nymph. Probably the best way to make the cpu log file to compare with nestest.log\n");
    fclose(fptr);
    return 0;
}