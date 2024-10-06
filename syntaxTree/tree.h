#ifndef REGTREE
#define REGTREE

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "util.h"

namespace reglib
{
    class SyntaxException : public std::exception{
    private:
        std::string message;
    public:
        SyntaxException(const char *m) : message(m) {}
        const char * what () const noexcept override {
            return message.c_str();
        }
    };

    enum RegType {A, OR, AND, STAR, PLUS, SLASH, BrO, BrC, ENDS, REP};

    struct Node{
        RegType type;
        Node* left;
        Node* right;
        std::string content;
        bool N;
        //int pos;
        std::vector<int> F;
        std::vector<int> L;
        explicit Node(RegType type, std::string content = "") : type(type), content(content), left(nullptr), right(nullptr), N(true) {}
    };

    class SyntaxTree{
    private:
        Node* root;
        std::vector<std::pair<std::string, std::vector<int>>> fp;
        
        void check_syntax(std::string reg) {}
        static std::vector<Node*> parse(std::string::iterator, std::string::iterator);
        static Node* build(std::vector<Node*>::iterator, std::vector<Node*>::iterator);
        static Node* build_start(std::vector<Node*>::iterator, std::vector<Node*>::iterator);
        static Node* deep_copy(Node*);
        void fill_sets(Node *n);
        static Node* add_range(std::vector<Node*>& nodes, std::string::iterator& it);
        //static Node* add_repeat(std::vector<Node*>& nodes, std::vector<Node*>& subnodes);
        void build_fp(Node*);
        static void print2DUtil(Node* root, int space)
        {
            if (root == NULL)
                return;
        
            space += 5;
        
            print2DUtil(root->right, space);
        
            std::cout << std::endl;
            for (int i = 5; i < space; i++)
                std::cout << " ";
            //std::cout << root->type << ":" << root->content << std::endl;
            std::string F, L;
            for(const auto &i : root->F)
                (F += std::to_string(i)) += " ";
            for(const auto &i : root->L)
                (L += std::to_string(i)) += " ";
            std::cout << root->type << ":" << root->content << ":" << F << ":" << L << std::endl;
        
            print2DUtil(root->left, space);
        }
        
        static void printout(Node* root, bool print2D = true){
            std::cout << "Tree:" << std::endl;
            if(print2D){
                print2DUtil(root, 0);
                return;
            }
            if(!root) return;
            printout(root->left);
            std::string F, L;
            for(const auto &i : root->F)
                (F += std::to_string(i)) += " ";
            for(const auto &i : root->L)
                (L += std::to_string(i)) += " ";
            std::cout << root->type << ":" << root->content << ":" << F << ":" << L;
            printout(root->right);
        }
        void del_node(Node* n){
            if(!n) return;
            del_node(n->left);
            del_node(n->right);
            delete n;
        }
        void print_fp() const{
            std::cout << "FP:" << std::endl;
            for(size_t i = 0; i < fp.size(); i++){
                std::cout << i << " " << fp[i].first << ": {"; 
                for(int j = 0; j < fp[i].second.size()-1; j++){
                    std::cout << fp[i].second[j] << ",";
                }
                if(fp[i].second.size()) std::cout << fp[i].second[fp[i].second.size()-1];
                std::cout << "}" << std::endl;
            }
        }
    public:
        SyntaxTree(std::string reg) { 
            root = nullptr;
            check_syntax(reg); 
            reg = "(!" + reg + ")";
            auto nodes = parse(reg.begin(), reg.end());
            root = build_start(nodes.begin(), nodes.end()); 
            Node *p = new Node(AND);
            p->left = root;
            p->right = new Node(ENDS);
            root = p;
            fill_sets(root);
            //printout(root);
            build_fp(root);
            //print_fp();
            std::cout << std::endl;
        }
        
        const Node* get_root() { return root; }
        auto& get_fp() {return fp;}
        
        ~SyntaxTree(){ del_node(root); }
    };   
}

#endif