#include "cachelab.h"
#include "stdlib.h"
#include <stdio.h>
#include "getopt.h"
#include <string.h>

static int S;
static int E;
static int B;
static int hit_count = 0;
static int miss_count = 0;
static int eviction_count = 0;
static int totalSet;

typedef struct _Node{
    unsigned tag;
    struct _Node* next;
    struct _Node* prev;
}Node;

typedef struct _LRU{
    Node* head; // Connect the accessed data to the head
    Node* tail; // Data linked to the tail will be deleted if the cache is full.
    int* size; // record the total number of Linkedlists
}LRU;

static LRU* lru;

void initLRU(int i){
    lru[i].head = malloc(sizeof(Node));
    lru[i].tail = malloc(sizeof(Node));

    lru[i].head->next = lru[i].tail;
    lru[i].tail->prev = lru[i].head;
    (lru[i].size) = (int* )malloc(sizeof(int));
    *(lru[i].size) = 0;
}

void deleteElement(unsigned set, Node* node, LRU* curLru){
    node->next->prev = node->prev;
    node->prev->next = node->next;
    *(curLru->size) = *(curLru->size) - 1;
}

void evict(unsigned set, LRU* curLru){
    deleteElement(set, curLru->tail->prev, curLru);
}

void addFirst(unsigned set, Node* node, LRU* curLru){
    node->next = curLru->head->next;
    node->prev = curLru->head;

    curLru->head->next->prev = node;
    curLru->head->next       = node;

    *(curLru->size) = *(curLru->size) + 1;
}

void parseOption(int argc, char** argv, char** fileName){
    int option;
    while( (option = getopt(argc, argv, "s:E:b:t:")) != -1){
        switch (option) {
            case 's':
                S = atoi(optarg);
            case 'E':
                E = atoi(optarg);
            case 'b':
                B = atoi(optarg);
            case 't':
                strcpy(*fileName, optarg);
        }
    }


    totalSet = 1 << S; // 2^S
}

void update(unsigned address){
    unsigned mask = 0xFFFFFFFF;
    unsigned maskSet = mask >> (32 - S);
    unsigned targetSet = ((maskSet) & (address >> B));
    unsigned targetTag = address >> (S + B);

    LRU curLru = lru[targetSet];
    
    // check if we have one, if it does, we find it.
    Node* cur = curLru.head->next;
    int found = 0; // 1 is the first one, 0 is the last one
    while(cur != curLru.tail){
        if(cur->tag == targetTag){
            found = 1;
	    break;
        }

        cur = cur->next;
    }
    
    if(found){
        hit_count++;
        deleteElement(targetSet, cur, &curLru);
        addFirst(targetSet, cur, &curLru);
	    printf("hit!, the set number %d \n", targetSet);
    }else{

        Node* newNode = malloc(sizeof(Node));
        newNode->tag = targetTag;
        if(*(curLru.size) == E){ // full, evict
            deleteElement(targetSet, curLru.tail->prev, &curLru);
            addFirst(targetSet, newNode, &curLru);

            eviction_count++;
            miss_count++;
	    printf("evic + miss set -> %d\n", targetSet);
        }else{
            miss_count++;
            addFirst(targetSet, newNode, &curLru);
            printf("only miss %d\n", targetSet);
        }    
    }
}

void cacheSimu(char* fileName){
    lru = malloc(totalSet * sizeof(*lru));
    for(int i = 0; i < totalSet; i++)
        initLRU(i);

    FILE* file = fopen(fileName, "r");
    char op;
    unsigned address;
    int size;

    // for example : M 20,1
    while(fscanf(file, " %c %x,%d", &op, &address, &size) > 0){
        printf("%c, %x %d\n", op, address, size);
	switch (op) {
            case 'L':
                update(address);
                break;
            case 'M':
                update(address);
            case 'S':
                update(address);
                break;
        }

    }
}

int main(int argc, char** argv)
{
    char* fileName = malloc(100 * sizeof(char));

    parseOption(argc, argv, &fileName);

    cacheSimu(fileName);

    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
