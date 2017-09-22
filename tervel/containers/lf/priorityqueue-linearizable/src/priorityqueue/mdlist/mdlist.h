#ifndef MDLIST_H
#define MDLIST_H

#include <cstdint>
#include <string>
#include "priorityqueue/mdlist/allocator.h"

class MDList 
{
public:
    static const uint32_t DIMENSION = 8;

    struct Node;

    //Any insertion as a child of node in the rage [pred_dim, dim] needs to help finish the task
    struct Desc
    {
        Node* curr;
        uint8_t pred_dim;              //dimension of pred node
        uint8_t dim;                   //dimension of this node
    };

    struct Node
    {
        Node* m_child[DIMENSION];
        uint32_t m_key;
        uint8_t m_coord[DIMENSION];
        uint32_t m_seq;
        Node* m_purged;
        Desc* m_pending;            //pending operation to update the children of 
    };

    struct Stack
    {
        Node* m_head;
        Node* m_del[DIMENSION];
    };

public:
    MDList (uint32_t testSize, uint32_t numThread);
    ~MDList ();
    
    void Insert(uint32_t key);
    uint32_t DeleteMin();

private:
    //Procedures used by Insert()
    template<int D>
    void KeyToCoord(uint32_t key, uint8_t coord[]);

    void LocatePred(Stack* path, Node* new_node, Node*& pred, Node*& curr, uint32_t& dim, uint32_t& pred_dim);
    Desc* FillNewNode(Node* new_node, Node*& pred, Node*& curr, uint32_t& dim, uint32_t& pred_dim);
    void AddSibling(Node* new_node, Node*& curr);
    void FinishInserting(Node* n, Desc* desc);

    //Procedures used by DeleteMin();
    void Purge(Node* head, Node* purged);

    void Traverse(Node* n, Node* parent, int dim, std::string& prefix);

public:
    Allocator m_pool;
private:
    Node* m_head;
    char m_pad[CACHE_LINE_SIZE - sizeof(Node*)];
    Stack* m_stack;
    char m_pad1[CACHE_LINE_SIZE - sizeof(Node*)];
    Stack* m_purge;
};

#endif /* end of include guard: MDLIST_H */
