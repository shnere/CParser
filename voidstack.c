#include <stdlib.h>
#include <stdio.h>

typedef struct voidnode {
  struct voidnode * next;
  void * data;
} VoidNode;

void removeFirst(VoidNode ** root)
{
  if(*root)
  {
    VoidNode * tmp = (*root)->next;
    free(*root);
    *root = tmp;    
  }
}

void removeLast(VoidNode ** root) 
{
  if(*root)
  {
    if(!(*root)->next)
    {
      free(*root);
      *root = NULL;
    }
    else
    {
      VoidNode * prev = *root;
      VoidNode * next = (*root)->next;
      while(next->next)
      {
        prev = prev->next;
        next = next->next;
      }
      free(next);
      next = NULL;
      prev->next = NULL;
      
    }
  }
}

void addLast(VoidNode ** root, void * data)
{
  VoidNode *add = (VoidNode *) malloc(sizeof(VoidNode));
  if(add)
  {
    add->data = data;
    add->next = NULL;
    VoidNode * tmp = *root;
    if(!tmp)
    {
      *root = add;
    }
    while(tmp->next)
    {
      tmp = tmp->next;
    }
    tmp->next = add;
  }
}


void addFirst(VoidNode ** root, void * data)
{
  VoidNode *add = (VoidNode *) malloc(sizeof(VoidNode));
  if(add)
  {
    add->data = data;
    add->next = NULL;
    if(*root)
    {
      add->next = *root;
    }
    *root = add;
  }
}

void printIntList(VoidNode ** root)
{
  VoidNode * temp = *root;
  while(temp)
  {
    printf("%d\n", (int) temp->data);
    temp = temp->next;
  }
}

void printCharList(VoidNode ** root)
{
  VoidNode * temp = *root;
  printf("[");
  while(temp)
  {
    printf("%s", (char *) temp->data);
    temp = temp->next;
    if(temp){
      printf(", ");
    } else {
      printf("]");
    }
  } 
}

/*

int main()
{

  VoidNode * root = NULL;
  addFirst(&root, (void *) 5);
  addFirst(&root, (void *) 52);
  removeFirst(&root);
  addLast(&root, (void *) 14);
  addLast(&root, (void *) 8);
  printIntList(&root);
  return 0;

}
*/