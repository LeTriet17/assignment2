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
                LL_Node() : type("Unknow"), next(NULL) {}
                LL_Node(string type, LL_Node *next = NULL) : type(type), next(next) {}
            };
            LL_Node *head, *tail;

        public:
            LL_Param() : head(NULL), tail(NULL) {}
            void add(string type);
            LL_Node *get_head()
            {
                return head;
            }
        };

        friend class SymbolTable;

    public:
        string id_name;
        int scope;
        string type;
        bool nstatic;
        T_Node *left, *right, *parent;
        LL_Param *func_param;
        T_Node() : id_name("Unknown"), scope(-1), type("Unknown"), nstatic(false), left(NULL), right(NULL), parent(NULL), func_param(NULL) {}
        T_Node(string id_name, int scope, string type = "Unknow",bool nstatic = false) : id_name(id_name), scope(scope), type(type), nstatic(nstatic), left(NULL), right(NULL), parent(NULL), func_param(NULL) {}
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
        Q_Node *head, *tail, *t_static;

    public:
        Sequence() : head(NULL), tail(NULL), t_static(NULL) {}
        void add_first(T_Node *T_node)
        {
            Q_Node *node = new Q_Node(T_node);
            node->next = head;
            tail = head = node;
        }
        void add_next(T_Node *T_node)
        {
            Q_Node *node = new Q_Node(T_node);
            node->next = tail->next;
            tail = tail->next = node;
        }
        void add_static(T_Node *T_node)
        {
            if (head)
            {
                Q_Node *node = new Q_Node(T_node);
                t_static = t_static->next = node;
            }
            else
                head = t_static = new Q_Node(T_node);
        }
        void remove_head()
        {
            if (head->next == NULL)
            {
                delete head;
                head = tail = NULL;
            }
            else
            {
                Q_Node *tmp = head;
                head = head->next;
                delete tmp;
            }
        }
        T_Node *findNode(string &name)
        {
            Q_Node *curr = head;
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
    void rightRotate(T_Node *);
    void leftRotate(T_Node *);
    void splay(T_Node *);
    int Insert(T_Node *);
    void Assign(); // not decalare param yet
    void preOrder(T_Node *root)
    {
        if (root)
        {
            preOrder(root->left);
            cout << root->id_name << "//" << root->scope << " ";
            preOrder(root->right);
        }
    }
    void block_detect();
    bool contains(T_Node *&);
    void remove(T_Node *);
    int num_com(T_Node *node);
    int num_splay(T_Node *node);

public:
    SymbolTable() : root(NULL), block(0)
    {
        seq = new Sequence;
    }
    void run(string filename);
};
#endif