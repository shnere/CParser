#ifndef __CTREE_H__
#define __CTREE_H__

/* Buffer size */
#define BSIZE 1024

/* struct Node structure */
struct Node {
	void* data; // int
    void* dataReal; // el valor de los datos: 2
	void* dataType;
	void* name; // el nombre de la variable
	struct Node* parent;
	struct Node* prevsibling;
	struct Node* nextsibling;
	struct Node* firstchild;
};

/* create tree */
struct Node* create_tree (void* data);

/* insert node */
int insert_node_under (struct Node* node, struct Node* targetparent);
int insert_node_next_to (struct Node* node, struct Node* targetsibling);

/* create node */
struct Node* create_node_under (struct Node* node, void* data, void* dataType, void* name, void* dataReal);
struct Node* create_node_next_to (struct Node* node, void* data);

/* traverse tree */
void traverse_node (struct Node* node,
                    void (*print_data)(void*, int, int, unsigned int*));
struct Node* pre_order(struct Node* node, VoidNode ** ast);

/* detach node */
int detach_node (struct Node* node);

/* delete node */
int delete_node (struct Node* node);

/* move node */
int move_node_next_to (struct Node* node, struct Node* targetsibling);
int move_node_under (struct Node* node, struct Node* targetparent);

/* shallow copy */
struct Node* shallow_copy (struct Node* node);

/* deep copy */
struct Node* deep_copy (struct Node* node, void* (*copyfunc)(void*));

/* search tree */
struct Node* search (struct Node* node, void* a, int (*compare)(void* a, void* b));
struct Node* searchFirstLevel (struct Node* node, void* a, int (*compare)(void * a, void * b));;

#endif /* __CTREE_H__ */
