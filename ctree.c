/*
 * ctree -- generic tree implementation with nodes capable of having
 * arbitrary number of children and siblings stored in a circularly linked
 * list for fast insertion/deletion/moving
 *
 * chillu
 * Date: Sunday November, 15 2009
 * Time: 10:18:44 PM IST
 */

#include <stdlib.h>

#include "ctree.h"

/*
 * creates and returns pointer to an empty root node
 * root nodes cannot have siblings
 */
struct Node*
create_tree (void* data) {
	struct Node* root = (struct Node *) malloc(sizeof(struct Node));
	root->data = data;
	root->dataType = -1;
	root->parent = root->firstchild = (struct Node *) NULL;
	root->prevsibling = root->nextsibling = (struct Node *) NULL;
	return root;
}

/*
 * insert a given node under a target node
 * as a child of the target. If target node
 * already has children the node is added at
 * the end.
 */
int
insert_node_under (struct Node* node, struct Node* targetparent) {
    
    struct Node* lastchild = (struct Node *) NULL;
    
    node->parent = targetparent;
    
    if (!targetparent->firstchild) {
        targetparent->firstchild = node;
        node->nextsibling = node->prevsibling = node;
    } else {
        lastchild = targetparent->firstchild->prevsibling;
        
        lastchild->nextsibling = node;
        node->prevsibling = lastchild;
        
        node->nextsibling = targetparent->firstchild;
        targetparent->firstchild->prevsibling = node;
    }
    
    return 1;
}

/*
 * creates and returns poiner to a child under a given node
 */
struct Node*
create_node_under (struct Node* node, void* data) {
    
    struct Node* newchild = (struct Node *) malloc(sizeof(struct Node));
	newchild->data = data;
	newchild->dataType = -1;
	
	newchild->firstchild = (struct Node *) NULL;
	insert_node_under (newchild, node);
	
	return newchild;
}

/*
 * insert a given node next to a target node
 * as a sibling of the target.
 */
int
insert_node_next_to (struct Node* node, struct Node* targetsibling) {
    
	struct Node* next = targetsibling->nextsibling;
	node->parent = targetsibling->parent;
	
	/* take care of sibling links */
	targetsibling->nextsibling = node;
	next->prevsibling = node;
	node->nextsibling = next;
	node->prevsibling = targetsibling;
	
	return 1;
}

/*
 * creates and inserts a sibling node next to a given node
 * returning pointer to the newly created sibling
 */
struct Node*
create_node_next_to (struct Node* node, void* data) {
    
	struct Node* newsibling = (struct Node *) malloc(sizeof(struct Node));
	newsibling->data = data;
	newsibling->dataType = -1;
	
	newsibling->firstchild = (struct Node *) NULL;
	insert_node_next_to (newsibling, node);
	
	return newsibling;
}

static void
_traverse_node (struct Node* node, int depth,
                void (*print_data)(void*, int, int, unsigned int*)) {
    struct Node *start, *next;
    start = next = node->firstchild;
    static unsigned int bitmask = 0;
    int islastchild = node->nextsibling == ((node->parent) ? node->parent->firstchild : NULL);
    /* If a new traversal reset bitmask */
    if (!depth)
        bitmask = 0;
    
    /* call printing function */
    print_data (node->data, depth, islastchild, &bitmask);
    
    /* update bitmask if lastchild is past */
    if (islastchild) {
        bitmask ^= (1 << (depth));
    }
    
    /* if the node has a child */
    if (start) {
        _traverse_node (next, depth + 1, print_data);
        /* recurse without going round in circles */
        while ((next = next->nextsibling) != start) {
            _traverse_node (next, depth + 1, print_data);
        }
    }
}

/*
 * traverse recursively all the nodes under the given node
 * callback function is called with useful information like:
 *
 * data: The data stored in the node
 * depth: Distance from the root node
 * islastchild: Boolean is 0 if there are more siblings to this node
 * bitmask: Flags for each depth level, with 0 indicating more
 *          siblings are to come on that depth and 1 indicating
 *          that all siblings on that level are past. Thus, it
 *          starts out with all 0s.
 *
 *          NOTE: The bitmask is only 32 bits wide, so
 *          depth information is available only from 0-31
 */
void
traverse_node (struct Node* node,
               void (*print_data)(void*, int, int, unsigned int*)) {
    _traverse_node(node, 0, print_data);
}

/*
 * detach (but don't delete) the node, preserving
 * silbing and parent-child relationships
 *
 * NOTE: Assumes that node is not root
 * It doesn't make sense to "remove" root
 * from a tree as root _is_ the tree
 */
int
detach_node (struct Node* node) {
    /* detach the node out of the list of
     children of which the node is a part */
    struct Node* prev = node->prevsibling;
    struct Node* next = node->nextsibling;
    if (node != next) {
        /* has at least one other sibling */
        prev->nextsibling = next;
        next->prevsibling = prev;
    }
    
    /* if the node is not root then you have to
     manage parent-child relationships */
    if (node->parent) {
        /* the only child is about to be deleted */
        if (node->nextsibling == node) {
            node->parent->firstchild = (struct Node *) NULL;
        }
        /* the first child of the parent is about to be deleted */
        else if (node->parent->firstchild == node) {
            node->parent->firstchild = node->nextsibling;
        }
    }
    
    return 1;
}

static int
_delete_node (struct Node* node, int raw) {
    
    struct Node* start = node->firstchild;
    struct Node* curr  = (struct Node *) NULL;
    
    /* if deletion is raw then don't bother maintaining
     the prev/next sibling relationships as these nodes
     will anyway be deleted later. (i.e. part of a
     larger recursive deletion)
     raw is 0 only for the toplevel invocation of _delete_node */
    if (!raw && node->parent) {
        detach_node (node);
    }
    
    /* first delete all children of the node */
    if (start) {
        /* only one child */
        if (start == start->nextsibling) {
            _delete_node (start, 1);
        }
        /* more than one child */
        else {
            curr = start->nextsibling;
            while (start != curr) {
                curr = curr->nextsibling;
                _delete_node (curr->prevsibling, 1);
            }
            _delete_node (start, 1);
        }
    }
    
    /* delete the node itself */
    free (node);
    
    return 1;
}

/*
 * recursively delete all the children under a given node
 * maintaining the nextsibling relationships of the node
 * Returns 1 for success, 0 for failure
 *
 * TODO: Currently always returns 1 => Do error checking.
 */
int
delete_node (struct Node* node) {
    return _delete_node(node, 0);
}

/*
 * move a given node next to a target node
 * as a sibling of the target
 * Does not check if the target is actually
 * a child of the given node (in which case
 * the results are undefined)
 */
int
move_node_next_to (struct Node* node, struct Node* targetsibling) {
    return detach_node (node) && insert_node_next_to (node, targetsibling);
}

/*
 * move a given node next to a target node
 * as a child of the target
 * Does not check if the target is actually
 * a child of the given node (in which case
 * the results are undefined)
 */
int
move_node_under (struct Node* node, struct Node* targetparent) {
    return detach_node (node) && insert_node_under (node, targetparent);
}

/*
 * returns a pointer to a new tree that is a
 * recursive copy of the sub-tree under node
 *
 * NOTE: The new tree will still point to the same
 * set of pointers that the old tree pointed to
 */
struct Node* shallow_copy (struct Node* node)
{
    struct Node* root = (struct Node *) create_tree (node->data);
    struct Node *start, *next;
    start = next = node->firstchild;
    
    if (start) {
        insert_node_under (shallow_copy (start), root);
        while ((next = next->nextsibling) != start) {
            insert_node_under (shallow_copy (next), root);
        }
    }
    
    return root;
}

/*
 * returns a pointer to a new tree that is a
 * recursive copy of the sub-tree under node
 * with copyfunc allocating memory appropriately
 */
struct Node* deep_copy (struct Node* node, void* (*copyfunc)(void*)) {
    struct Node* root = (struct Node *) create_tree (node->data);
    root->data = copyfunc(root->data);
    
    struct Node *start, *next;
    start = next = node->firstchild;
    
    if (start) {
        insert_node_under (deep_copy (start, copyfunc), root);
        while ((next = next->nextsibling) != start) {
            insert_node_under (deep_copy (next, copyfunc), root);
        }
    }
    
    return root;
}

/*
 * traverse the tree and return the pointer to the first node for
 * which the compare function returns 0; otherwise, return NULL
 */
struct Node*
search (struct Node* node, void* a, int (*compare)(void* a, void* b)) {
    struct Node *start, *next, *temp;
    start = next = node->firstchild;
    
    if (!node->data)
        return NULL;
    
    if (compare(a, node->data) == 0)
        return node;
    
    if (start) {
        if ((temp = search (start, a, compare)))
            return temp;
        while ((next = next->nextsibling) != start)
            if ((temp = search (next, a, compare)))
                return temp;
    }
    
    return NULL;
}

/*
 * traverse the tree and return the pointer to the first node for
 * which the compare function returns 0; otherwise, return NULL
 */
struct Node*
searchFirstLevel (struct Node* node, void* a, int (*compare)(void* a, void * b)) {
	struct Node *start, *next;
	start = next = node->firstchild;
	
	if (start == NULL) {
		return NULL;
    }
	
    if (!next->data)
        return NULL;
    
    if (compare(a, next->data) == 0)
        return next;
	
	do{
		if (compare(a, next->data) == 0)
	        return next;
		next = next->nextsibling;
	}while (next != start);
    
    return NULL;
}
