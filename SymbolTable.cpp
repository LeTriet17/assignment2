#include "SymbolTable.h"
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
                if ((++li_token)->compare("true") == 0 && block != 0)
                    throw InvalidDeclaration(line);
            }
            else if (regex_match(line, regex("INSERT [a-z]\\w* \\(((number|string)(,number|,string)*)?\\)->(number|string) (true|false)")))
            {
                regex delim("\\s");
                sregex_token_iterator li_token(line.begin(), line.end(), delim, -1);
                string name = *(++li_token);
                string func = *(++li_token);
                if ((++li_token)->compare("false") == 0 || block != 0)
                    throw InvalidDeclaration(line);
                string type(line[line.find_first_not_of("->")], line[line.size()]);
                node = new T_Node(name, block, type);
                node->func_param = new SymbolTable::T_Node::LL_Param;
                regex func_regex("(number|string)");
                for (sregex_iterator it = sregex_iterator(func.begin(), func.end() - 8, func_regex); it != sregex_iterator(); it++)
                {
                    node->func_param->add(it->str());
                }
            }
            else
                throw InvalidInstruction(line);
            Insert(node, line);
            if (seq->head == NULL || (seq->head->node)->scope == block)
                seq->add_next(node);
            else
                seq->add_first(node);
        }
        else if (regex_match(line, regex("ASSIGN [a-z]\\w* ([0-9]+|'[a-zA-Z0-9\\s]+')")))
        {
            smatch match;
            if (regex_search(line, match, regex("[a-z]\\w*")))
            {
                T_Node *node = new T_Node(match.str(0), block);
                root = splay(root, node);
                if ((root->type.compare("string") == 0 && !regex_search(line, regex("'[a-zA-Z0-9\\s]+'"))) || (root->type.compare("number") == 0 && !regex_search(line, regex("[0-9]+"))))
                    throw TypeMismatch(line);
                if (!root->id_name.compare(match.str(0)))
                    throw Undeclared(line);
            }
        }
        else if (regex_match(line, regex("ASSIGN [a-z]\\w* [a-z]\\w*")))
        {
            regex name_regex(("[a-z]\\w*"));
            string name[2];
            int i = 0;
            for (sregex_iterator it = sregex_iterator(line.begin(), line.end(), name_regex); it != sregex_iterator(); it++, i++)
                name[i] = it->str();
            if (!seq->findNode(name[1]) || !seq->findNode(name[0]))
                throw Undeclared(line);
            if (seq->findNode(name[1])->type.compare(seq->findNode(name[0])->type))
                throw TypeMismatch(line);
        }
        else if (regex_match(line, regex("ASSIGN [a-z]\\w* [a-z]\\w*\\((([0-9]+|'[a-zA-Z0-9\\s]+')(,[0-9]+|,'[a-zA-Z0-9\\s]+')*)?\\)")))
        {
            string name_func(&line[line.find_first_not_of("ASSIGN ")], &line[line.find_first_of("(")]);
            string iden_name(&name_func[0], &name_func[name_func.find_first_of(" ")]);
            string func_name(&name_func[iden_name.length()] + 1, &name_func[name_func.length()]);
            T_Node *func_node = seq->findNode(func_name);
            if (func_node)
            {
                root = splay(root, func_node);
                if (!root->func_param)
                throw TypeMismatch(line);
            }
            else throw Undeclared(line);
            string param(&line[line.find_first_of("(") + 1], &line[line.find_last_of(")")]);
            regex func_regex("([0-9]+|'[a-zA-Z0-9\\s]+')");
            SymbolTable::T_Node::LL_Param::LL_Node *curr = root->func_param->get_head();
            for (sregex_iterator it = sregex_iterator(param.begin(), param.end(), func_regex); it != sregex_iterator(); it++)
            {
                if ((curr->type.compare("string") == 0 && !regex_match(it->str(), regex("'[a-zA-Z0-9\\s]+'"))) || (curr->type.compare("number") == 0 && !regex_match(it->str(), regex("[0-9]+"))))
                    throw TypeMismatch(line);
                curr = curr->next;
            }
            T_Node *iden_node = seq->findNode(iden_name);
            if (iden_node)
            {
                root = splay(root, iden_node);
                if (!func_node->type.compare(iden_node->type)||root->func_param)
                    throw TypeMismatch(line);
            }
            else
                throw Undeclared(line);
        }
        else if (regex_match(line, regex("BEGIN")))
            block++;
        else if (regex_match(line, regex("END")))
        {
            if (block == 0)
                throw UnknownBlock();
            while ((seq->head->node)->scope == block)
            {
                T_Node *rm_node = seq->head->node;
                seq->remove_head();
                remove(rm_node);
            }
            block--;
        }
        else if (regex_match(line, regex("LOOKUP [a-z]\\w*")))
        {
            string name(&line[line.find_first_of(" ") + 1], &line[line.size()]);
            T_Node *node = seq->findNode(name);
            if (node)
            {
                root = splay(root, node);
                cout << root->id_name << "//" << root->scope << endl;
            }
            else
                throw Undeclared(line);
        }
        else if (regex_match(line, regex("PRINT")))
        {
            preOrder(root);
            cout << endl;
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
            Right_Tree_Min = Right_Tree_Min->left = t;
            t = t->left;
        }
        else if (*t < node)
        {
            if (t->right && *t->right < node)
                t = left_rotate(t);
            if (!t->right)
                break;
            Left_Tree_Max = Left_Tree_Max->right = t;
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
void SymbolTable::remove(T_Node *&node)
{
    T_Node *new_Tree;
    if (!root)
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