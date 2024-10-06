#ifndef REG
#define REG

#include "syntaxTree/tree.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <queue>
#include <set>

namespace reglib{
    typedef std::vector<int> State;

    struct GraphNode{
        typedef std::map<std::string, GraphNode*> Con;
        State st;
        bool val;
        Con con;
        GraphNode(State v, Con c, bool valid = false) : st(v), con(c), val(valid) {}
    };

    class DFA{
    private:
        typedef std::set<GraphNode*> Group;
        GraphNode* graph;
        GraphNode* curState; 
        std::shared_ptr<GraphNode> emptyState;
        std::vector<std::pair<std::string, State>> fp;
        std::vector<Group> groups;
        void next_state(char);
        GraphNode* create_node(State);
        void create_con(std::queue<GraphNode*>&, GraphNode*);
        GraphNode* search_node(const State&) const;
        void reset() { curState = graph; }
        static void print_state(State top) {for(const auto&i : top) std::cout << i << " "; if(top.empty()) std::cout << "empty"; std::cout << std::endl;}
        int get_group(GraphNode*);
        int find_appr_group(std::vector<Group>&, GraphNode*);
        bool comp_cons(GraphNode*, GraphNode*);
        void clear_graph();
        void print_groups();
    public:
        DFA(std::string regex){
            groups.push_back(Group());
            groups.push_back(Group());
            curState = new GraphNode(State(), GraphNode::Con(), false);
            emptyState = std::shared_ptr<GraphNode>(curState);
            auto tree = SyntaxTree(std::move(regex));
            fp = std::move(tree.get_fp());
            State startState = tree.get_root()->F;
            graph = nullptr;
            curState = graph = create_node(startState);
            std::queue<GraphNode*> q;
            q.push(graph);            
            while (!q.empty())
            {
                create_con(q, q.front());
                q.pop();
            }
            //print_groups();
            //minimize();
            reset();
        }
        bool is_valid() const;
        friend bool match(std::string str, DFA& dfa); 
        friend std::vector<std::string> findall(std::string str, DFA& dfa);
        std::pair<std::vector<std::map<std::string, int>>, std::vector<bool>> minimize();
        ~DFA();
    };

    struct MulState{
        int state[2];
        std::map<std::string, int> bonds;
        MulState(int s1, int s2) {
            state[0] = s1;
            state[1] = s2;
            bonds = std::map<std::string, int>();
        }
    };

    class MDFA{
        typedef std::vector<std::map<std::string, int>> Table;
        int state;
        Table table;
        std::vector<bool> valid;
        bool errorstate;
        void next_state(char a);
        void reset() {state = 0;}
        Table eliminate_states() const;
        typedef std::vector<MulState> Multiplication;
        static Multiplication multiply(const MDFA &op1, const MDFA &op2);
        static void minimize_multiplication(Multiplication& mul);
    public:
        MDFA(DFA& dfa){
            auto p = dfa.minimize();
            table = p.first;
            valid = p.second;
            errorstate = false;
            reset();
        }
        MDFA(std::string reg){
            auto dfa = DFA(reg);
            auto p = dfa.minimize();
            table = p.first;
            valid = p.second;
            errorstate = false;
            reset();
        }
        MDFA(Table t, std::vector<bool> v){
            state = 0;
            table = std::move(t);
            valid = std::move(v);
            errorstate = false;        
        }
        void print_table() const{
            for(int i = 0; i < table.size(); i++){
                std::cout << i << "(" << valid[i] << "): ";
                if(table[i].empty()) { std::cout << std::endl; continue; }
                auto j = table[i].begin();
                std::cout << "{" << j->first << ", " << j->second << "}";
                j++;
                for(j ; j != table[i].end(); j++){
                    std::cout << ", {" << j->first << ", " << j->second << "}";
                }
                std::cout << std::endl;
            }
        }
        bool is_valid() const { if(state == -1) return errorstate; return valid[state]; }
        friend bool match(std::string str, MDFA& dfa);
        friend std::vector<std::string> findall(std::string str, MDFA& dfa);
        std::string get_re() const;
        static MDFA inverse(MDFA op);
        static MDFA intersection(const MDFA &op1, const MDFA &op2);
    };

    DFA compile(std::string regex);
    MDFA compile_m(std::string regex);
    template <typename T>
    std::vector<std::string> findall(std::string str, std::string regex, bool minimize = true){
        if(minimize){
            MDFA dfa = compile(regex);
            return findall(str, dfa);
        }else{
            DFA dfa = compile(regex);
            return findall(str, dfa);
        }
    }
}

#endif