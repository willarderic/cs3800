typedef struct requests {
    unsigned char req[500];
} request;

int max = 1024;
int front = 0;
int back = 0;
request requestQueue[1024];

void push(request req) {
    requestQueue[front] = req;
    front++;
}

void pop() {
    front = (front + 1) % max;
}

request peek() {
    return requestQueue[front];
}

