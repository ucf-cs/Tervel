#ifndef MDLIST_H
#define MDLIST_H

#include <cstdint>
#include <string>
#include "dictionary/mdlist/allocator.h"

class MDList 
{
public:
    static const uint32_t DIMENSION = 16;

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
        uint8_t m_coord[DIMENSION];
        uint32_t m_key;             //key
        Desc* m_pending;            //pending operation to adopt children 
    };

public:
    MDList (uint32_t testSize, uint32_t numThread, uint32_t keyRange);
    ~MDList ();
    
    void Insert(uint32_t key);
    bool Delete(uint32_t key);
    bool Find(uint32_t key);

private:
    //Procedures used by Insert()
    template<int D>
    void KeyToCoord(uint32_t key, uint8_t coord[]);

    void LocatePred(uint8_t coord[], Node*& pred, Node*& curr, uint32_t& dim, uint32_t& pred_dim);
    Desc* FillNewNode(Node* new_node, Node*& pred, Node*& curr, uint32_t& dim, uint32_t& pred_dim);
    void FinishInserting(Node* n, Desc* desc);

    void Traverse(Node* n, Node* parent, int dim, std::string& prefix);

public:
    Allocator m_pool;
private:
    Node* m_head;
    uint32_t m_basis;
};

#endif /* end of include guard: MDLIST_H */
