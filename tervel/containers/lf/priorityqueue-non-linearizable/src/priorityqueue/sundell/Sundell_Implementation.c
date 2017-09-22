#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "lock-free_priority_queue_test.h"
//#include <windows.h>
//#include <WinBase.h>
//#include <atomic>


//For all functions in the peudocode:
//Time stamping removed
//Backoff code removed, commented out
//Memory management code removed, commented out
//The are some temporary code lines added to prevent errors for removed code these are marked throughout the code.


#define maxLevel 10 //temporary definition

//Top Function declarations
struct Node * HelpDelete(struct Node * node, int level);
struct Node * CreateNode(int level, int key, int * value);


typedef struct Node * Link; //pointer to node
typedef long * VLink; //pointer to Value

struct Node {
    int key, level, validLevel;
    VLink value;
    Link next[maxLevel]; //temporarily assigned maxLevel
    struct Node * prev;
};

//Global variables:
struct Node * head, * tail;

//Local variables (for all functions/procedures)
struct Node * newNode, * savedNodes[maxLevel];
struct Node * node1, * node2, * prev, * last;
int i, level;

//The following are currently
//Undefined functions
//Need to add instructions?

struct Node * MALLOC_NODE(){return 0;}; //I replaced every function call of this with a simple malloc call.
struct Node * READ_NODE(Link address /*Previously pointer to link*/){
printf("%d", address->level);//temporary print to avoid error
return 0;};
struct Node * COPY_NODE(struct Node * node){
printf("%d", node->level);//temporary print to avoid error
return 0;};

struct Node * CreateNode(int level, int key, int * value){
 struct Node * node = malloc(sizeof *node);
 node->prev=NULL;
 node->validLevel = 0;
 node->level = level;
 node->key=key;
 node->value = (VLink)((long)value| 0x0);
 return node;
};

//Was having trouble with double pointers in these next two functions, seems to be fixed now:
struct Node * ReadNext(struct Node ** node1, int level){
    if((long)(*node1)->value & ~1){
    *node1=HelpDelete(*node1,level);
    }
    node2=READ_NODE((*node1)->next[level]);
    while(node2==NULL){
        *node1=HelpDelete(*node1,level);
        node2=READ_NODE((Link)(*node1)->next[level]);
    }
    return node2;
};

struct Node * ScanKey(struct Node ** node1, int level, int key){
    node2 = ReadNext(node1, level);
    while(node2->key<key){
        //RELEASE_NODE(*node1);
        *node1=node2;
        node2=ReadNext(node1,level);
    }
    return node2;
};


_Bool Insert(int key, int * value){

    //Choose level randomly according to the skip-list distribution
    srand(time(NULL));
    int level = rand();

    struct Node * newNode = CreateNode(level,key,value);
    COPY_NODE(newNode);
    node1 = COPY_NODE(head);
    int i;
    for(i=maxLevel-1; i>=1; i--){
        node2 = ScanKey(&node1,i,key);
        //Release_Node(node2);
        if(i<level){
            savedNodes[i]= COPY_NODE(node1);
        }
    }
    while(true){
        node2 = ScanKey(&node1,0,key);

         VLink val2 = node2->value;

        if(!(((long)val2 & ~1)) && node2->key==key){

            if(__sync_bool_compare_and_swap (&node2->value, val2, value)){
                //RELEASE_NODE(node1);
                //RELEASE_NODE(node2);
                int i;
                for(i=1; i<(level-1); i++){
                    //RELEASE_NODE(savedNodes[i]);
                }
                //RELEASE_NODE(newNode);
                //RELEASE_NODE(newNode);
                return true;
            }
            else{
                //RELEASE_NODE(node2);
                continue;
            }
        }
        newNode->next[0] = (Link)((long)node2 | 0x0);
        //RELEASE_NODE(node2);
        if(__sync_bool_compare_and_swap(&node1->next[0], node2, newNode)){
            //RELEASE_NODE(node1);
            break;
        }
        //Back-off
    }
    for(i=1;i<level-1;i++){
            newNode->validLevel=i;
            node1 = savedNodes[i];
            while(true){
                node2 = ScanKey(&node1,i,key);
                newNode->next[i]= (Link)((long)node2 | 0x0);
                //RELEASE_NODE(node2);
                if(((long long)newNode->value & ~1) ||
                     __sync_bool_compare_and_swap(&node1->next[i],node2,newNode)){
                    //RELEASE_NODE(node1);
                    break;
                   }
                //Back-off
            }
        }
        newNode->validLevel=level;
        if((long long)newNode->value & ~1){
            newNode=HelpDelete(newNode,0);
        }
        //RELEASE_NODE(newNode);
        return true;
}

void RemoveNode(struct Node * node, struct Node ** prev, int level){
    while(true){
        if((node->next[level]== NULL) & (~1)){
                break;
        }
        last=ScanKey(prev,level,node->key);
        //RELEASE_NODE(last);
        if((last!=node) || ((node->next[level]== NULL) & (~true))){
            break;
        }
        if(__sync_bool_compare_and_swap(&(*prev)->next[level],node,node->next[level])){
            node->next[level]= (Link) 1; //Is this correct?
            break;
        }
        if((node->next[level]== NULL) & (~1)){
            break;
        }
        //Back-off
    }
}


 long * DeleteMin(){ //Temporarily changed this to a long function due to the return value
    prev=COPY_NODE(head);
    while(true){
        node1=ReadNext(&prev,0);
        if(node1==tail){
            //RELEASE_NODE(prev);
            //RELEASE_NODE(node1);
            return NULL;
        }
    }

    retry: printf(" ");
        long * value = node1->value;
        if(node1!=prev->next[0]){
            //RELEASE_NODE(node1);
        }
        if(!((long long)value & ~1)){
            if(__sync_bool_compare_and_swap(&node1->value, value, (long)value | 0x1)){
                node1->prev=prev;
                goto done;
            }
            else{
                goto retry;
            }
        }
        else if(((long long)value & ~1)){
            node1=HelpDelete(node1,0);
        }
        //RELEASE_NODE(prev);
        prev=node1;

    done: printf(" ");
    int i;
    for(i=0;i<(node1->level)-1;i++){
        //repeat
        while(true){
            node2 = node1->next[i];
            if(((long long)value & ~1) || __sync_bool_compare_and_swap(&node1->next[i],node2,(long)node2 | 0x1)){
                break;
            }
        }
    }//Not sure if this for loop is correct
    prev=COPY_NODE(head);
    for(i=(node1->level)-1; i>=0; i--){
        RemoveNode(node1,&prev,i);
    }
    //RELEASE_NODE(prev);
    //RELEASE_NODE(node1);
    //RELEASE_NODE(node1);
    return value; //incompatible return?
};

struct Node * HelpDelete(struct Node * node, int level){
    int i;
    for(i=level; i<(node->level)-1; i++){
        while(true){
            node2 = node1->next[i];
             if((long)node2 & ~1 ||
                  __sync_bool_compare_and_swap(&node1->next[i],node2,(long)node2 | 0x1)){
                break;

            }
        }
    }
    prev = node->prev;
    if(!prev || level>=prev->validLevel){
        prev=COPY_NODE(head);
        for(i=maxLevel-1;i>level;i--){
            node2=ScanKey(&prev,i,node->key);
            //RELEASE_NODE(node2);
        }
    }
    else{
        COPY_NODE(prev);
    }
    RemoveNode(node,&prev,level);
    //RELEASE_NODE(node);
    return prev;
};

//Test Functions
int main(){

    //Perform Random Inserts and Deletes
    int i=0;
    while(i<25){

    //Random values
    srand(time(NULL));
    int num = rand()%2;
    //int * num2= (int *)num;

    if(num==0){
     //   _Bool test = Insert(num,num2);
    }
    else{
     DeleteMin();
    }

    i++;
    }
return 0;
}//End of test
