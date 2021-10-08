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
            int num_slay = 0;
            int num_comp = 0;
            T_Node *node = NULL;
            if (regex_match(line, regex("INSERT [a-z]\\w* (number|string) (true|false)")))
            {
                regex delim("\\s");
                sregex_token_iterator li_token(line.begin(), line.end(), delim, -1);
                node = new T_Node(*(++li_token), *(++li_token), block);
                string stas = *(++li_token);
                if (stas.compare("true") == 0 && block != 0)
                    throw InvalidDeclaration(line);
            }
            else if (regex_match(line, regex("INSERT [a-z]\\w* \\(((number|string)(,number|,string)*)?\\)->(number|string) (true|false)")))
            {
                regex delim("\\s");
                sregex_token_iterator li_token(line.begin(), line.end(), delim, -1);
                string name = *(++li_token);
                string func = *(++li_token);
                string stas = *(++li_token);
                if (stas.compare("false") == 0 || block != 0)
                    throw InvalidDeclaration(line);
                string type = "";
                smatch s;
                if (regex_search(func, s, regex("(number|string)$")))
                    type = s[0];
                node = new T_Node(name, type, block);
                string list_param(&func[1], &func[func.find_last_of(")")]);
                delim = (",");
                sregex_token_iterator param(list_param.begin(), list_param.end(), delim, -1);
                std::sregex_token_iterator end;
                node->func_param= new SymbolTable::T_Node::LL_Param;
                while (param != end)
                {
                    string param_type = (*(param++));
                    node->func_param->add(param_type);
                }
            }
            else
                throw InvalidInstruction(line);
            Insert(root, node, line);
        }
        else if (regex_search(line, regex("^ASSIGN")))
        {
        }
        else if (regex_search(line, regex("^BEGIN")))
        {
            block++;
        }
        else if (regex_search(line, regex("^END")))
        {
            // if (block > 0)
            //     Delete(root, block);
            // else
            //     throw UnknownBlock();
        }
        else if (regex_search(line, regex("^PRINT")))
        {
        }
        else
            throw InvalidInstruction(line);
    }
    block_detect();
    preOrder(root);
    infile.close();
}
void SymbolTable::T_Node::LL_Param::add(string type)
{
    if (!head)
        head = tail = new LL_Node(type);
    else
        tail = tail->next = new LL_Node(type);
}
SymbolTable::T_Node *SymbolTable::right_rotate(T_Node *root)
{
    T_Node *tmp = root->left;
    root->left = tmp->right;
    tmp->right = root;
    return tmp;
}
SymbolTable::T_Node *SymbolTable::left_rotate(T_Node *root)
{
    T_Node *tmp = root->right;
    root->right = tmp->left;
    tmp->left = root;
    return tmp;
}
SymbolTable::T_Node *SymbolTable::splay(T_Node *&t, string &name)
{
    T_Node *Left_Tree_Max, *Right_Tree_Min;

    T_Node header;
    header.left = header.right = NULL;
    Left_Tree_Max = Right_Tree_Min = &header;
    for (;;)
        if (t->id_name.compare(name) == 1)
        {
            if (t->left && t->left->id_name.compare(name) == 1)
                t = right_rotate(t);
            if (!t->left)
                break;
            Right_Tree_Min->left = t;
            Right_Tree_Min = t;
            t = t->left;
        }
        else if (t->id_name.compare(name) == -1)
        {
            if (t->right && t->right->id_name.compare(name) == -1)
                t = left_rotate(t);
            if (!t->right)
                break;
            Left_Tree_Max->right = t;
            Left_Tree_Max = t;
            t = t->right;
        }
        else
            break;
    Left_Tree_Max->right = t->left;
    Right_Tree_Min->left = t->right;
    t->left = header.right;
    t->right = header.left;
    return t;
}
void SymbolTable::Insert(T_Node *&t, T_Node *node, string &line)
{
    if (root)
    {
        if (root->id_name.compare(node->id_name) == 0 && root->scope == node->scope)
            throw Redeclared(line);
        root = splay(root, node->id_name);
        if (root->id_name.compare(node->id_name) == 1)
        {
            node->right = root;
            node->left = root->left;
            root->left = NULL;
        }
        else
        {
            node->left = root;
            node->right = root->right;
            root->right = NULL;
        }
    }
    root = node;
}
void SymbolTable::block_detect()
{
    if (block > 0)
        throw UnclosedBlock(block);
    if (block < 0)
        throw UnknownBlock();
}
