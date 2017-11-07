#include <stdio.h>
#include "requestQueue.h"

int main() {

    push("hello");
    request tmp;
    tmp = peek();
    printf("Data: %s", tmp.req);
    return 0;
}