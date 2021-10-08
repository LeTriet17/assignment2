#include "SymbolTable.h"
SymbolTable::T_Node::LL_Param::LL_Node::LL_Node() : type("Unknow"), next(NULL) {}
SymbolTable::T_Node::LL_Param::LL_Node::LL_Node(string type, LL_Node *next) : type(type), next(next) {}
SymbolTable::T_Node::LL_Param::LL_Param() : head(NULL), tail(NULL) {}
SymbolTable::T_Node::T_Node() : id_name("Unknown"), type("Unknown"), scope(-1), left(NULL), right(NULL), func_param(NULL) {}
SymbolTable::T_Node::T_Node(string id_name, int scope, string type) : id_name(id_name), scope(scope), type(type), left(NULL), right(NULL), func_param(NULL) {}
SymbolTable::SymbolTable() : root(NULL), block(0)
{
    seq = new Sequence;
}
void SymbolTable::run(string filename)
{
    ifstream infile(filename);
    string line;
    while (getline(infile, line))
    {
        if (regex_search(line, regex("^INSERT")))
        {
            T_Node *node = NULL;
            if (regex_match(line, regex("INSERT [a-z]\\w* (number|string) (true|false)")))
            {
                regex delim("\\s");
                sregex_token_iterator li_token(line.begin(), line.end(), delim, -1);
                node = new T_Node(*(++li_token), block, *(++li_token));
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
                smatch s;
                if (regex_search(func, s, regex("(number|string)$")))
                    node = new T_Node(name, block, s[0].str());
                string list_param(&func[1], &func[func.find_last_of(")")]);
                delim = (",");
                sregex_token_iterator param(list_param.begin(), list_param.end(), delim, -1);
                std::sregex_token_iterator end;
                node->func_param = new SymbolTable::T_Node::LL_Param;
                while (param != end)
                {
                    string param_type = (*(param++));
                    node->func_param->add(param_type);
                }
            }
            else
                throw InvalidInstruction(line);
            Insert(node, line);
            if (seq->head == NULL || (*seq->head->node)->scope == block)
                seq->add_next(node);
            else if ((*seq->head->node)->scope != block)
                seq->add_first(node);
        }
        else if (regex_search(line, regex("^ASSIGN")))
        {
            if (regex_match(line, regex("ASSIGN [a-z]\\w* ([0-9]+|'[a-zA-Z0-9\\s]+')")))
            {
                smatch match;
                regex name("[a-z]\\w*");
                regex string_type("'[a-zA-Z0-9\\s]+'");
                regex num_type("[0-9]+");
                if (regex_search(line, match, name))
                {
                    string name = match[0];
                    T_Node *node = new T_Node(name, block);
                    root = splay(root, node);
                    if ((root->type.compare("string") && !regex_search(line, string_type)) || (root->type.compare("number") && !regex_search(line, num_type)))
                        throw TypeMismatch(line);
                    if (!root->id_name.compare(name))
                        throw Undeclared(line);
                }
            }
            else if (regex_match(line, regex("ASSIGN [a-z]\\w* [a-z]\\w*")))
            {
                // regex name("[a-z]\\w*");
                // sregex_iterator name_iden(line.begin(), line.end(), name);
                // smatch name = *name_iden;
            }
            else if (regex_match(line, regex("ASSIGN [a-z]\\w* [a-z]\\w*\\((([0-9]+|'[a-zA-Z0-9\\s]+')(,[0-9]+|,'[a-zA-Z0-9\\s]+')*)?\\)")))
            {
                regex name("[a-z]\\w*");
                string name_func(&line[line.find_first_not_of("ASSIGN ")], &line[line.find_first_of("(")]);
                string iden_name(&name_func[0], &name_func[name_func.find_first_of(" ")]);
                string func_name(&name_func[iden_name.length()] + 1, &name_func[name_func.length()]);
                T_Node *func_node = new T_Node(func_name, block);
                if (!contains(func_node))
                    throw Undeclared(line);
                func_node->type = root->type;
                if (!root->func_param)
                    throw TypeMismatch(line);
                string param(&line[line.find_first_of("(") + 1], &line[line.find_last_of(")")]);
                regex delim(",");
                sregex_token_iterator param_token(param.begin(), param.end(), delim, -1);
                std::sregex_token_iterator end;
                SymbolTable::T_Node::LL_Param::LL_Node *curr = root->func_param->get_head();
                while (param_token != end)
                {
                    string type = *(param_token);
                    if ((curr->type.compare("string") == 0 && !regex_match(type, regex("'[a-zA-Z0-9\\s]+'"))) || (curr->type.compare("number") == 0 && !regex_match(type, regex("[0-9]+"))))
                        throw TypeMismatch(line);
                    curr = curr->next;
                    param_token++;
                }
                T_Node *iden_node = new T_Node(func_name, block);
                if (!contains(iden_node))
                    throw Undeclared(line);
                iden_node->type = root->type;
                if (!func_node->type.compare(iden_node->type))
                    throw TypeMismatch(line);
            }
            else
                throw InvalidInstruction(line);
        }
        else if (regex_search(line, regex("^BEGIN")))
        {
            block++;
        }
        else if (regex_search(line, regex("^END")))
        {
            if (block == 0)
                throw UnknownBlock();
            while ((*seq->head->node)->scope == block)
            {
                T_Node *rm_node = *seq->head->node;
                seq->remove_head();
                remove(rm_node);
            }
            block--;
        }
        else if (regex_search(line, regex("^PRINT")))
        {
        }
        else
            throw InvalidInstruction(line);
    }
    if (block > 0)
        throw UnclosedBlock(block);
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
SymbolTable::T_Node *SymbolTable::splay(T_Node *&t, T_Node *&node)
{
    T_Node *Left_Tree_Max, *Right_Tree_Min;
    T_Node header;
    header.left = header.right = NULL;
    Left_Tree_Max = Right_Tree_Min = &header;
    for (;;)
        if (*t > node)
        {
            if (t->left && *t->left > node)
                t = right_rotate(t);
            if (!t->left)
                break;
            Right_Tree_Min->left = t;
            Right_Tree_Min = t;
            t = t->left;
        }
        else if (*t < node)
        {
            if (t->right && *t->right < node)
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
void SymbolTable::Insert(T_Node *node, string &line)
{
    if (root)
    {
        root = splay(root, node);
        if (*root == node)
            throw Redeclared(line);
        if (*root > node)
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
SymbolTable::T_Node *SymbolTable::search(T_Node *&node)
{
    return splay(root, node);
}
bool SymbolTable::contains(T_Node *&node)
{
    if (isEmpty())
        return false;
    root = splay(root, node);
    return *root == node;
}
bool SymbolTable::isEmpty()
{
    return root == NULL;
}
void SymbolTable::remove(T_Node *&node)
{
    T_Node *new_Tree;
    if (!contains(node))
        return;
    if (root->left)
    {
        new_Tree = root->left;
        splay(node, new_Tree);
        new_Tree->right = root->right;
    }
    else
        new_Tree = root->right;
    delete root;
    root = new_Tree;
}
