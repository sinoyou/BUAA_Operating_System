#include <queue.h>
#include <my_test.h>
#define NULL 0
typedef struct Node {
    int value;
    LIST_ENTRY(Node) entry;
} Node;
void test_queue(){
	printf("testing queue...\n");
	Node node1, node2, node3, node4;
	node1.value = 1;
	node2.value = 2;
	node3.value = 3;
	node4.value = 4;
	LIST_HEAD(ListHead, Node) head;
	LIST_INIT(&head);
	printf("list inited\n");
	LIST_INSERT_TAIL(&head, &node1, entry);
	LIST_INSERT_TAIL(&head, &node2, entry);
	LIST_INSERT_AFTER(&node2, &node3, entry);
	LIST_INSERT_TAIL(&head, &node4, entry);
	Node * tmp;
	LIST_FOREACH(tmp, &head, entry) {
		printf("%d\n", tmp->value);
	}
}
