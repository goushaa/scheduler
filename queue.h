// Define the Node struct for the queue
typedef struct Node {
    struct PCB data;
    struct Node* next;
} Node;

// Define the Queue struct
typedef struct Queue {
    Node* front;
    Node* rear;
} Queue;

// Initialize the queue
void init_queue(Queue* q) {
    q->front = NULL;
    q->rear = NULL;
}

// Check if the queue is empty
int is_empty(Queue* q) {
    return (q->front == NULL);
}

// Add an element to the queue
void enqueue(Queue* q, struct PCB data) {
    Node* new_node = (Node*) malloc(sizeof(Node));
    new_node->data = data;
    new_node->next = NULL;

    if (is_empty(q)) {
        q->front = new_node;
        q->rear = new_node;
    }
    else {
        q->rear->next = new_node;
        q->rear = new_node;
    }
}

// Remove an element from the queue
struct PCB dequeue(Queue* q) {
    if (is_empty(q)) {
        printf("Queue is empty.\n");
        struct PCB empty_pcb = {0};
        return empty_pcb;
    }

    struct PCB data = q->front->data;
    Node* temp = q->front;

    if (q->front == q->rear) {
        q->front = NULL;
        q->rear = NULL;
    }
    else {
        q->front = q->front->next;
    }

    free(temp);
    return data;
}

// Get the front element of the queue
struct PCB front(Queue* q) {
    if (is_empty(q)) {
        printf("Queue is empty.\n");
        struct PCB empty_pcb = {0};
        return empty_pcb;
    }

    return q->front->data;
}

// Get the size of the queue
int queue_size(Queue* q) {
    int count = 0;
    Node* current = q->front;

    while (current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

// Delete the queue
void delete_queue(Queue* q) {
    Node* current = q->front;

    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }

    q->front = NULL;
    q->rear = NULL;
}