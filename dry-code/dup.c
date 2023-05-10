#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

/*

Code:
1. Changed assert(!s) to assert(s) to assert that s is not NULL.
2. Added + 1 to malloc size to make space for null characer.
3. Changed loop condition to i<times from i<=times to stop after the last index.
4. Moved copying the string to the output to after moving the output pointer.
5. Moved output back to the start of the string after the loop.
6. Changed the assert to a conditional return so that it will stay in build mode.

Convention:
1. Changed LEN to len - local variables should be camelCase.
2. Indentet code nested in the for loop.

*/

char* stringDuplicator(char* s, int times){
    assert(s);
    assert(times > 0);
    int len = strlen(s);
    char* out = malloc(len*times + 1);
    if (!out) return NULL;
    for (int i=0; i<times; i++) {
        strcpy(out,s);
        out = out + len;
    }
    out -= len*times;
    return out;
}

int main() {
    char* string = "Hello world!";
    char* dupped = stringDuplicator(string, 3);
    printf("Returned: %s\n", dupped);
    return 0;
}


