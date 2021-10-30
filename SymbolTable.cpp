//  _
//                                _ooOoo_
//                               o8888888o
//                               88" . "88
//                               (| -_- |)
//                               O\  =  /O
//                            ____/`---'\____
//                          .'  \\|     |//  `.
//                         /  \\|||  :  |||//  \
//                        /  _||||| -:- |||||_  \
//                        |   | \\\  -  /'| |   |
//                        | \_|  `\`---'//  |_/ |
//                        \  .-\__ `-. -'__/-.  /
//                      ___`. .'  /--.--\  `. .'___
//                   ."" '<  `.___\_<|>_/___.' _> \"".
//                  | | :  `- \`. ;`. _/; .'/ /  .' ; |
//                  \  \ `-.   \_\_`. _.'_/_/  -' _.' /
//        ===========`-.`___`-.__\ \___  /__.-'_.'_.-'================
//                                `=--=-'                    
#include "SymbolTable.h"
void SymbolTable::run(string filename)
{
    ifstream infile(filename);
    string line;
    while (getline(infile, line))
    {
        if (regex_match(line, regex("INSERT [a-z]\\w* (number|string) (true|false)")) || regex_match(line, regex("INSERT [a-z]\\w* \\(((number|string)(,number|,string)*)?\\)->(number|string) (true|false)")))
        {
            T_Node *node = NULL;
            regex delim("\\s");
            sregex_token_iterator li_token(line.begin(), line.end(), delim, -1);
            string name = *(++li_token);
            T_Node *check = seq->findNode(name);
            if (check && check->scope == block)
                throw Redeclared(line);
            if (regex_match(line, regex("INSERT [a-z]\\w* (number|string) (true|false)")))
            {
                node = new T_Node(name, block, *(++li_token));
                if ((++li_token)->compare("true") == 0)
                {
                    if (check && check->scope == 0)
                        throw Redeclared(line);
                    node->scope = 0;
                    node->nstatic = true;
                }
            }
            else if (regex_match(line, regex("INSERT [a-z]\\w* \\(((number|string)(,number|,string)*)?\\)->(number|string) (true|false)")))
            {
                if (check && check->scope == 0)
                    throw Redeclared(line);
                string func = *(++li_token);
                if ((++li_token)->compare("false") == 0 && block)
                    throw InvalidDeclaration(line);
                string type(&func[func.find_first_of(">") + 1], &func[func.size()]);
                node = new T_Node(name, 0, type, true);
                node->func_param = new SymbolTable::T_Node::LL_Param;
                regex func_regex("(number|string)");
                for (sregex_iterator it = sregex_iterator(func.begin(), func.end() - 8, func_regex); it != sregex_iterator(); it++)
                    node->func_param->add(it->str());
            }
            int nsplay = num_splay(node);
            int com = Insert(node);
            if (seq->head == NULL || node->nstatic || block == 0)
                seq->add_static(node);
            else if ((seq->head->node)->scope != block)
                seq->add_first(node);
            else
                seq->add_next(node);
            cout << com << " " << nsplay << endl;
        }
        else if (regex_match(line, regex("ASSIGN [a-z]\\w* ([0-9]+|'[a-zA-Z0-9\\s]*')")))
        {
            smatch match;
            int com = 0;
            int nsplay = 0;
            if (regex_search(line, match, regex("[a-z]\\w*")))
            {
                string name = match.str(0);
                T_Node *node = seq->findNode(name);
                if (!node)
                    throw Undeclared(line);
                com = num_com(node);
                nsplay = num_splay(node);
                splay(node);
                if ((root->type.compare("string") == 0 && !regex_search(line, regex("'[a-zA-Z0-9\\s]*'"))) || (root->type.compare("number") == 0 && !regex_search(line, regex("$[0-9]+"))))
                    throw TypeMismatch(line);
            }
            cout << com << " " << nsplay << endl;
        }
        else if (regex_match(line, regex("ASSIGN [a-z]\\w* [a-z]\\w*")))
        {
            regex name_regex(("[a-z]\\w*"));
            string name[2];
            int i = 0;
            for (sregex_iterator it = sregex_iterator(line.begin(), line.end(), name_regex); it != sregex_iterator(); it++, i++)
                name[i] = it->str();
            T_Node *node1 = seq->findNode(name[1]);
            T_Node *node0 = seq->findNode(name[0]);
            if (!node0 || !node1)
                throw Undeclared(line);
            if (node1->func_param || node0->func_param || node1->type.compare(node0->type))
                throw TypeMismatch(line);
            int com = num_com(node1);
            int nsplay = num_splay(node1);
            splay(node1);
            com += num_com(node0);
            nsplay += num_splay(node0);
            splay(node0);
            cout << com << " " << nsplay << endl;
        }
        else if (regex_match(line, regex("ASSIGN [a-z]\\w* [a-z]\\w*\\((([0-9]+|'[a-zA-Z0-9\\s]*'|[a-z]\\w*)(,[0-9]+|,'[a-zA-Z0-9\\s]+'|,[a-z]\\w*)*)?\\)")))
        {
            string name_func(&line[line.find_first_not_of("ASSIGN ")], &line[line.find_first_of("(")]);
            string iden_name(&name_func[0], &name_func[name_func.find_first_of(" ")]);
            string func_name(&name_func[iden_name.length()] + 1, &name_func[name_func.length()]);
            T_Node *func_node = seq->findNode(func_name);
            int com = 0;
            int nsplay = 0;
            if (func_node)
            {
                com += num_com(func_node);
                nsplay += num_splay(func_node);
                splay(func_node);
                if (!root->func_param)
                    throw TypeMismatch(line);
            }
            else
                throw Undeclared(line);
            string param(&line[line.find_first_of("(") + 1], &line[line.find_last_of(")")]);
            regex func_regex("([0-9]+|'[a-zA-Z0-9\\s]*'|[a-z]\\w*)");
            SymbolTable::T_Node::LL_Param::LL_Node *curr = root->func_param->get_head();
            for (sregex_iterator it = sregex_iterator(param.begin(), param.end(), func_regex); it != sregex_iterator(); it++, curr = curr->next)
            {
                if (!curr)
                    throw TypeMismatch(line);
                if (regex_match(it->str(), regex("[a-z]\\w*")))
                {
                    string name = it->str();
                    T_Node *node = seq->findNode(name);
                    if (node)
                    {
                        com += num_com(node);
                        nsplay += num_splay(node);
                        splay(node);
                        if (node->type.compare(curr->type))
                            throw TypeMismatch(line);
                    }
                    else
                        throw Undeclared(line);
                }
                else if ((curr->type.compare("string") == 0 && !regex_match(it->str(), regex("'[a-zA-Z0-9\\s]*'"))) || (curr->type.compare("number") == 0 && !regex_match(it->str(), regex("[0-9]+"))))
                    throw TypeMismatch(line);
            }
            if (curr)
                throw TypeMismatch(line);
            T_Node *iden_node = seq->findNode(iden_name);
            if (iden_node)
            {
                com += num_com(iden_node);
                nsplay += num_splay(iden_node);
                splay(iden_node);
                if (iden_node->func_param || func_node->type.compare(iden_node->type))
                    throw TypeMismatch(line);
            }
            else
                throw Undeclared(line);
            cout << com << " " << nsplay << endl;
        }
        else if (regex_match(line, regex("BEGIN")))
            block++;
        else if (regex_match(line, regex("END")))
        {
            if (block == 0)
                throw UnknownBlock();
            while (seq->head && (seq->head->node)->scope == block)
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
                splay(node);
                cout << root->scope << endl;
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
void SymbolTable::rightRotate(T_Node *y)
{
    T_Node *z = y->parent;
    T_Node *x = y->left;
    y->left = NULL;
    if (z)
    {
        if (z->left == y)
            z->left = x;
        if (z->right == y)
            z->right = x;
        x->parent = z;
    }
    else
        x->parent = NULL;
    if (x->right)
    {
        y->left = x->right;
        y->left->parent = y;
    }
    x->right = y;
    y->parent = x;
}
void SymbolTable::leftRotate(T_Node *y)
{
    T_Node *z = y->parent;
    T_Node *x = y->right;
    y->right = NULL;
    if (z)
    {
        if (z->left == y)
            z->left = x;
        if (z->right == y)
            z->right = x;
        x->parent = z;
    }
    else
        x->parent = NULL;
    if (x->left)
    {
        y->right = x->left;
        y->right->parent = y;
    }
    x->left = y;
    y->parent = x;
}
void SymbolTable::splay(T_Node *node)
{
    if (root == NULL || root == node)
        return;
    while (node->parent)
    {
        if (node == node->parent->left)
        {
            if (node->parent->parent)
            {
                if (node->parent == node->parent->parent->left)
                {
                    rightRotate(node->parent->parent);
                    rightRotate(node->parent);
                }
                else if (node->parent == node->parent->parent->right)
                {
                    rightRotate(node->parent);
                    leftRotate(node->parent);
                }
            }
            else
                rightRotate(node->parent);
        }
        else if (node == node->parent->right)
        {
            if (node->parent->parent)
            {
                if (node->parent == node->parent->parent->right)
                {
                    leftRotate(node->parent->parent);
                    leftRotate(node->parent);
                }
                else if (node->parent == node->parent->parent->left)
                {
                    leftRotate(node->parent);
                    rightRotate(node->parent);
                }
            }
            else
                leftRotate(node->parent);
        }
    }
    root = node;
}
int SymbolTable::Insert(T_Node *newnode)
{
    int com = 0;
    if (root)
    {
        T_Node *x = root;
        T_Node *y = NULL;
        while (x)
        {
            y = x;
            if (*x > newnode)
                x = x->left;
            else if (*x < newnode)
                x = x->right;
            com++;
        }
        if (y == NULL)
            y = newnode;
        else if (*newnode < y)
        {
            y->left = newnode;
            newnode->parent = y;
        }
        else if (*newnode > y)
        {
            y->right = newnode;
            newnode->parent = y;
        }
        splay(newnode);
    }
    else
        root = newnode;
    return com;
}
void SymbolTable::remove(T_Node *node)
{
    if (!root->right && !root->left)
    {
        delete root;
        root = NULL;
        return;
    }
    splay(node);
    T_Node *pLeft = root->left;
    T_Node *pRight = root->right;
    if (pLeft)
    {
        pLeft->parent = root->left = NULL;
        delete root;
        if (pRight)
        {
            pRight->parent = root->right = NULL;
            T_Node *maxleft = pLeft;
            while (maxleft->right)
            {
                maxleft = maxleft->right;
            }
            splay(maxleft);
            root->right = pRight;
            pRight->parent = root;
        }
        else
            root = pLeft;
    }
    else
    {
        delete root;
        pRight->parent = NULL;
        root = pRight;
    }
}
int SymbolTable::num_com(T_Node *node)
{
    int com = 0;
    if (root)
    {
        T_Node *x = root;
        while (x)
        {
            if (*node < x)
                x = x->left;
            else if (*node > x)
                x = x->right;
            else
                return ++com;
            com++;
        }
        return -1;
    }
    return com;
}
int SymbolTable::num_splay(T_Node *node)
{
    return (root && root != node) ? 1 : 0;
}