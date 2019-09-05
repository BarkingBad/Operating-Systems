#define MSGMAX 4096

#define STOP 1
#define LIST 2
#define FRIENDS 3
#define _2ALL 4
#define _2FRIENDS 5
#define _2ONE 6
#define INIT 7
#define ECHO 8
#define ADD 9
#define DEL 10

struct Message {
    long mtype;
    char msg[4000];
    int id;
    int priority;
} Message;

int msg_size() {
    return (sizeof(struct Message) - sizeof(long));
}
