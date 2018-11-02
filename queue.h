struct data
{
    int pip[2];
};
struct queue
{
    struct data *data;
    int sp, offset;
    int q_max; 
    struct data (*read)(struct queue *, int);
    void (*wirte)(struct queue *, struct data, int);
    void (*addsp)(struct queue *);
};

struct queue *queue_new();
void queue_del(struct queue **);
