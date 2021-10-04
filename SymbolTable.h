#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "main.h"

class SymbolTable
{
private:
    struct T_Node
    {
    private:
        class LL_Param
        {
        private:
            class LL_Node
            {
                string type;
                LL_Node *next;
                LL_Node();
                LL_Node(string type, LL_Node *next = NULL);
            };
            LL_Node *head;
        };
        string id_name, type;
        int scope;
        T_Node *left, *right;
        LL_Param *func_param;
        T_Node();
        T_Node(string id_name, string type, int scope);
    };
    class Stree
    {
    private:
        T_Node *root;
        T_Node* right_rotate(T_Node *root);
        T_Node *left_rotate(T_Node * root);
        void BT_Insert(T_Node* &root, T_Node* node);
        void Delete(T_Node *&root, string name);
    public:
        Stree();
        void splay(T_Node* root);
        void Insert(T_Node* &root, T_Node* node);
        void Assign();// not decalare param yet
        void preOrder(T_Node* root);
    };

public:
    SymbolTable() {}
    void run(string filename);
};
#endif