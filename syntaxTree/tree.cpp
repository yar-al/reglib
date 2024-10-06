#include "tree.h"

namespace reglib
{
    std::map<std::string, RegType> regTypeMap = { {"|", OR}, {"&", AND}, {"*", STAR}, {"+", PLUS}, {"/", SLASH}, 
        {"(!", BrO}, {")", BrC}, {"$", ENDS} };
    
    RegType getType(std::string s){
        auto a = regTypeMap.find(s);
        if(a != regTypeMap.end()){
            return a->second;
        }
        return A;
    }
    
    Node* SyntaxTree::add_range(std::vector<Node*>& nodes, std::string::iterator& it){
        char low=0, high=0;
        Node* p = new Node(A, "");
        ++it;
        while(*it != ']'){
            if(*it != '-'){
                p->content += *it;
                it++;
                continue;
            }
            low = *(it-1);
            high = *(it+1);
            for(char i = low+1; i < high; i++){
                p->content += i;
            }
            it++;
        }
        return p;
    }

    std::vector<Node*> SyntaxTree::parse(std::string::iterator start, std::string::iterator end){
        std::vector<Node*> nodes, subnodes;
        std::string servicestr;
        Node *p;
        int bracecounter = 0;
        for(auto it = start; it < end; it++){
            auto jt = it;
            auto kt = --jt;
            switch (*it)
            {
            case '*':
                p = new Node(STAR);
                break;
            case '|':
                p = new Node(OR);
                break;
            case '+':
                p = new Node(PLUS);
                break;
            case '%':
                it++;
                if(it == end)
                    throw SyntaxException("Operator without operand");
                p = new Node(A, {*it});
                break;
            case '(':
                if(it+1 != end){
                    if(*(it+1) == '!'){
                        p = new Node(BrO, std::to_string(++bracecounter));
                        it++;
                        break;
                    }
                }
                p = new Node(A, {*it});
                break;
            case ')':
                p = new Node(BrC, std::to_string(bracecounter--));
                break;
            case '$':
                p = new Node(ENDS);
                break;
            case '[':
                p = add_range(nodes, it);
                break;
            case '{':
                p = new Node(REP);
                while (*it != ','){
                    if(*it == ' ') continue;
                    if(it == end)
                        throw SyntaxException("Invalid repeat operator");
                    servicestr += *it;
                }
                if(servicestr.empty())
                    p->F.push_back(0);
                else 
                    p->F.push_back(std::stoi(servicestr));
                while(*it != '}'){
                    if(*it == ' ') continue;
                    if(it == end)
                        throw SyntaxException("Invalid repeat operator");
                    servicestr += *it;
                }
                if(servicestr.empty())
                    p->F.push_back(0);
                else 
                    p->F.push_back(std::stoi(servicestr));
                if(p->F[0] < 0 || p->F[1] < 0)
                        throw SyntaxException("Invalid repeat operator");
                break;
            default:
                p = new Node(A, {*it});
                break;
            }
            nodes.push_back(p);
        }
        if(bracecounter)
            throw SyntaxException("Invalid braces placement");
        return nodes;
    }
    
    Node* SyntaxTree::build_start(std::vector<Node*>::iterator start, std::vector<Node*>::iterator end){
        (*start)->content = std::to_string(0);
        (*(end-1))->content = std::to_string(0);
        while(true){
            int maxbracecount = 1, bracecount;
            auto substart = start;
            auto subend = end;
            for(auto it = start; it < end; it++){
                if(*it && (*it)->type == BrO){
                    bracecount = std::stoi((*it)->content);
                    if(bracecount > maxbracecount){
                        maxbracecount = bracecount;
                        substart = it;
                        auto jt = it;
                    }
                }
            }
            for(auto it = substart; it < end; it++){
                if(*it && (*it)->type == BrC){
                    subend = it+1;
                    break;
                }
            }
            if(substart == start || subend == end)
                break;
            build_start(substart, subend);
        }
        return build(start, end);
    }

    Node* SyntaxTree::deep_copy(Node* root){
        if(!root) return nullptr;
        Node *p = new Node(*root);
        p->left = deep_copy(root->left);
        p->right = deep_copy(root->right);
        return p;
    }

    Node* SyntaxTree::build(std::vector<Node*>::iterator start, std::vector<Node*>::iterator end){
        Node *p, *p1, *root = nullptr;
        if(start + 1 == end - 1)
            return nullptr;
        /*for(auto it = start; it < end; it++){
            if(*it)
                std::cout << (*it)->type << " " << (*it)->content << std::endl;
            else
                std::cout << "N" << " " << std::endl;
        }*/
        std::cout << std::endl;
        auto jt = start + 1;
        if(*jt) if((*jt)->type == STAR || (*jt)->type == PLUS || (*jt)->type == OR || (*jt)->type == REP)
            throw SyntaxException("Operator without operand");
        while(!*jt){
            jt = ++start + 1;
        }
        for(auto it = start+2; it < end-1; it++){
            if(!*it)
                continue;
            switch ((*it)->type)
            {
            case STAR:
                p = *jt;
                *jt = *it;
                (*jt)->left = p; 
                *it = nullptr;
                break;
            case PLUS:
                p = deep_copy(*jt);
                (*it)->type = STAR;
                (*it)->left = p;
                break;
            case REP:
                break;
            default:
                break;
            }
            jt = it;
        }
        jt = start + 1;
        for(auto it = start+2; it < end-1; it++){
            if(!*it)
                continue;
            if((*it)->type == OR){
                if((*it)->left && (*it)->right){
                    p = new Node(AND);
                    p->left = *jt;
                    p->right = *it;
                    *jt = p;
                    *it = nullptr;
                    continue;
                }
                jt = it+1;
                it++;
                continue;
            }
            if((*it)->type != A && (*it)->type != STAR && (*it)->type != AND)
                continue;
            p = new Node(AND);
            p->left = *jt;
            p->right = *it;
            *jt = p;
            *it = nullptr;
        }
        jt = start+1;
        for(auto it = start+2; it < end-1; it++){
            if(!*it)
                continue;
            if((*it)->type != OR)
                continue;
            if((*it)->left && (*it)->right)
                continue;
            (*it)->left = *jt;
            (*it)->right = *(it+1);
            *jt = *it;
            *it = nullptr;
            *(it+1) = nullptr;
        }
        root = *(start+1);
        end--;
        delete *end;
        *end = nullptr;
        delete *start;
        *start = nullptr;
        //printout(root);
        return root;
    }

    void SyntaxTree::fill_sets(Node *n){
        if(!n) return;
        fill_sets(n->left);
        fill_sets(n->right);
        std::vector<int>::iterator end;
        switch (n->type)
        {
        case A:
            n->N = false;
            n->F.push_back(fp.size());
            n->L.push_back(fp.size());
            fp.push_back(std::make_pair(n->content, std::vector<int>()));
            break;
        case ENDS:
            n->N = false;
            n->F.push_back(fp.size());
            n->L.push_back(fp.size());
            break;
        case AND:
            n->N = n->left->N && n->right->N;
            if(n->left->N){
                n->F = std::move(Util::merge(n->left->F, n->right->F));
            }
            else
                n->F = n->left->F;
            if(n->right->N){
                n->L = std::move(Util::merge(n->left->L, n->right->L));
            }
            else
                n->L = n->right->L;
            break;
        case OR:
            n->N = n->left->N || n->right->N;
            n->F = std::move(Util::merge(n->left->F, n->right->F));
            n->L = std::move(Util::merge(n->left->L, n->right->L));
            break;
        case STAR:
            n->N = true;
            n->F = n->left->F;
            n->L = n->left->L;
            break;
        default:
            break;
        }
    }
    
    void SyntaxTree::build_fp(Node*root){
        if(!root)
            return;
        if(root->type == A)
            return;
        build_fp(root->left);
        build_fp(root->right);
        switch (root->type)
        {
        case AND:
            for(const auto &i : root->left->L)
                if(i < fp.size()) for(const auto &j : root->right->F)
                    fp[i].second.push_back(j);
            break;
        case STAR:
            for(const auto &i : root->L)
                if(i < fp.size()) for(const auto &j : root->F)
                    fp[i].second.push_back(j);
        default:
            break;
        }
    }
}