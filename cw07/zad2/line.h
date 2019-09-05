struct Line {
    int line;
    pid_t pid;
    struct timeval tv;
};

struct Metadata {
    int K;
    int M;
    int current_m;
};