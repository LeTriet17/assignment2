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
            friend class SymbolTable;

        private:
            class LL_Node
            {
            public:
                string type;
                LL_Node *next;
                LL_Node();
                LL_Node(string type, LL_Node *next = NULL);
            };
            LL_Node *head, *tail;

        public:
            LL_Param();
            void add(string type);
            LL_Node *get_head()
            {
                return head;
            }
        };

        friend class SymbolTable;

    public:
        string id_name, type;
        int scope;
        T_Node *left, *right;
        LL_Param *func_param;
        T_Node();
        T_Node(string id_name, int scope, string type = "Unknow");
        bool operator==(T_Node *&node)
        {
            return (this->scope == node->scope && this->id_name.compare(node->id_name) == 0);
        }
        bool operator>(T_Node *&node)
        {
            return ((this->scope > node->scope) || (this->scope == node->scope && this->id_name.compare(node->id_name) > 0));
        }
        bool operator<(T_Node *&node)
        {
            return ((this->scope < node->scope) || (this->scope == node->scope && this->id_name.compare(node->id_name) < 0));
        }
    };
    class Sequence
    {
    private:
        friend class SymbolTable;

        struct Q_Node
        {
            T_Node *node;
            Q_Node *next;
            Q_Node(T_Node *T_node) : node(T_node), next(NULL) {}
        };
        Q_Node *head;
        Q_Node *tail;

    public:
        Sequence() : head(NULL) {}
        void add_first(T_Node *T_node)
        {
            Q_Node *node = new Q_Node(T_node);
            node->next = head;
            tail = head = node;
        }
        void add_next(T_Node *T_node)
        {
            if (head)
            {
                Q_Node *node = new Q_Node(T_node);
                node->next = tail->next;
                tail = tail->next = node;
            }
            else
                head = tail = new Q_Node(T_node);
        }
        void remove_head()
        {
            Q_Node *tmp = head;
            head = head->next;
            delete tmp;
        }
        T_Node *findNode(string &name)
        {
            Q_Node* curr = head;
            while (curr)
                if ((curr->node)->id_name.compare(name) == 0)
                    return curr->node;
                else
                    curr = curr->next;
            return NULL;
        }
    };
    T_Node *root;
    Sequence *seq;
    int block;
    T_Node *right_rotate(T_Node *root);
    T_Node *left_rotate(T_Node *root);
    T_Node *splay(T_Node *&root, T_Node *&node);
    void Insert(T_Node *node, string &line);
    void Assign(); // not decalare param yet
    void preOrder(T_Node *root)
    {
        if (root)
        {
            cout << root->id_name << "//" << root->scope << " ";
            preOrder(root->left);
            preOrder(root->right);
        }
    }
    void block_detect();
    bool contains(T_Node *&node);
    void remove(T_Node *&node);
public:
    SymbolTable();
    void run(string filename);
};
#endif