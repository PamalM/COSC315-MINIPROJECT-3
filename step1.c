#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char inputfile[] = "step1input.txt";

int main(){
    FILE* file = fopen(inputfile,"r");

    char line[256];
    char delim[] = " ";

    int n = 0;
    int m = 0;
    unsigned int v[256];

    int i = 0;
    while (fgets(line, sizeof(line), file)) {

        char *var = strtok(line, delim);
        char *val = strtok(NULL, delim);
       // printf("%s\n",var);
       // printf("%s\n",val);

        switch (i)
        {
        case 0:
            n = atoi(val);
            break;
        case 1:
            m = atoi(val);
            break;
        default:
            v[i] = atoi(val);
            break;
        }

        i++;
    }

    for(int vcount = 2; vcount < i; vcount++){
        unsigned int value = v[vcount];

        unsigned int div = value >> n;
        unsigned int mod = value & ((1 << n) - 1);

        // int quotient = 0;
        // while(numerator >= denominator)
        // {
        //     numerator = numerator - denominator;
        //     quotient++;
        // }

        printf("Virutal Address of v%d is in page number %d and offset %d\n", (vcount - 1), div, mod);

    }



    fclose(file);
}