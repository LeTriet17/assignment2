#include "SymbolTable.h"
SymbolTable::T_Node::LL_Param::LL_Node::LL_Node() : type("Unknow"), next(NULL) {}
SymbolTable::T_Node::LL_Param::LL_Node::LL_Node(string type, LL_Node *next) : type(type), next(next) {}
SymbolTable::T_Node::LL_Param::LL_Param() : head(NULL), tail(NULL) {}
SymbolTable::T_Node::T_Node() : id_name("Unknown"), type("Unknown"), scope(-1), left(NULL), right(NULL), func_param(NULL) {}
SymbolTable::T_Node::T_Node(string id_name, string type, int scope) : id_name(id_name), type(type), scope(scope), left(NULL), right(NULL), func_param(NULL) {}
SymbolTable::SymbolTable() : root(NULL), block(0) {}
void SymbolTable::run(string filename)
{
    ifstream infile(filename);
    string line;
    while (getline(infile, line))
    {
        if (regex_search(line, regex("^INSERT")))
        {
            if (regex_match(line, regex("INSERT [a-z]\\w* (number|string) (true|false)")))
            {
                regex delim("\\s");
                sregex_token_iterator li_token(line.begin(), line.end(), delim, -1);
                T_Node *node = new T_Node(*(++li_token), *(++li_token), block);
                string stas = *(++li_token);
            }
            else if (regex_match(line, regex("INSERT [a-z]\\w* \\(((number|string)(,number|,string)*)?\\)->(number|string) true")))
            {
                regex delim("\\s");
                sregex_token_iterator li_token(line.begin(), line.end(), delim, -1);
                string name = *(++li_token);
                string func = *(++li_token);
                string type = "";
                smatch s;
                if (regex_search(func, s, regex("(number|string)$")))
                    type = s[0];
                T_Node *node = new T_Node(name, type, block);
                string list_param(&func[1], &func[func.find_last_of(")")]);
                delim = (",");
                sregex_token_iterator param(list_param.begin(), list_param.end(), delim, -1);
                std::sregex_token_iterator end;
                while (param != end)
                {
                    node->func_param->add(*(param++));
                }
            }
            else
                throw InvalidInstruction(line);
        }
        else if (regex_search(line, regex("^ASSIGN")))
        {
        }
        else if (regex_search(line, regex("^BEGIN")))
        {
        }
        else if (regex_search(line, regex("^END")))
        {
        }
        else if (regex_search(line, regex("^PRINT")))
        {
        }
        else
            throw InvalidInstruction(line);
    }
    infile.close();
}
void SymbolTable::T_Node::LL_Param::add(string type)
{
    if (head)
    {
        tail->next = new LL_Node(type);
        tail = tail->next;
    }
    else
        head = tail = new LL_Node(type);
}
SymbolTable::T_Node *SymbolTable::right_rotate(T_Node *root)
{
    T_Node *tmp = root->left;
    root->left = tmp->right;
    tmp->right = root;
    ;
    return tmp;
}
SymbolTable::T_Node *SymbolTable::left_rotate(T_Node *root)
{
    T_Node *tmp = root->right;
    root->right = tmp->left;
    tmp->left = root;
    return tmp;
}
SymbolTable::T_Node *SymbolTable::splay(T_Node *root, string &name)
{
    if (root == NULL || root->id_name.compare(name) == 0)
        return root;
    if (root->id_name.compare(name) == 1)
    {
        if (root->left)
        {
            if (root->left->id_name.compare(name) == 1)
            {
                root->left->left = splay(root->left->left, name);
                root = right_rotate(root);
            }
            else if (root->left->id_name.compare(name) == -1)
            {
                root->left->right = splay(root->left->right, name);
                if (root->left->right)
                    root->left = left_rotate(root->left);
            }
        }
        else
            return root;
        return (root->left) ? right_rotate(root) : root;
    }
    else
    {
        if (root->right)
        {
            if (root->right->id_name.compare(name) == -1)
            {
                root->right->right = splay(root->right->right, name);
                root = left_rotate(root);
            }
            else if (root->right->id_name.compare(name) == 1)
            {
                root->right->left = splay(root->right->left, name);
                if (root->right->left)
                    root->right = right_rotate(root->right);
            }
        }
        else return root;
        return (root->right) ? left_rotate(root) : root;
    }
}