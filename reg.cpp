#include "reg.h"
#include <set>
namespace reglib
{
    GraphNode* DFA::search_node(const State& st) const{
        if(!graph) return nullptr;
        std::queue<GraphNode*> q;
        std::set<GraphNode*> visited;
        q.push(graph);
        while(!q.empty()){
            if(q.front()->st == st){
                return q.front();
            }
            visited.insert(q.front());
            for(const auto& i : q.front()->con){
                if(visited.find(i.second) == visited.end())
                    q.push(i.second);
            }
            q.pop();
        }
        return nullptr;
    }

    void DFA::create_con(std::queue<GraphNode*>& q, GraphNode*n){
        std::map<std::string, State> map;
        for(const auto &p : n->st){
            if(p == fp.size())
                continue;
            auto findRes = map.find(fp[p].first);
            if(findRes == map.end())
                findRes = map.insert({fp[p].first, fp[p].second}).first;
            findRes->second = Util::merge(findRes->second, fp[p].second);
        }
        for(const auto &i : map){
            GraphNode* p = search_node(i.second);
            if(!p) {p = create_node(i.second); q.push(p);}
            n->con.insert({i.first, p});
        }
    }

    GraphNode* DFA::create_node(State st){
        bool valid = false;
        for(const auto &j : st){
            valid = (j == fp.size());
            if(valid)
                break;
        }
        auto p = new GraphNode(st, GraphNode::Con(), valid);
        if(p->val) groups[1].insert(p);
        else groups[0].insert(p);
        return p;
    }

    void DFA::next_state(char a){
        bool flag;
        for(const auto& i : curState->con){
            flag = false;
            for(const auto& j : i.first){
                if(j == a){
                    flag = true;
                    break;
                }
            }
            if(flag){
                curState = i.second;
                break;
            }
            else
                curState = emptyState.get();
        }
    }

    bool DFA::is_valid() const{
        return curState->val;
    }

    DFA compile(std::string regex)
    {
        return DFA(std::move(regex));
    }

    MDFA compile_m(std::string regex)
    {
        auto dfa = DFA(regex);
        return MDFA(dfa);
    }

    std::pair<std::vector<std::map<std::string, int>>, std::vector<bool>> DFA::minimize()
    {
        std::vector<Group> newGroups;
        int appr;
        while(true){
            for(const auto &group : groups){
                for(const auto &node : group){
                    appr = find_appr_group(newGroups, node);
                    if(appr == -1){
                        appr = newGroups.size();
                        newGroups.push_back(Group());
                    }
                    newGroups[appr].insert(node);
                }
            }
            if(Util::vector_s_c<Group>(groups, newGroups)) break;
            groups = std::move(newGroups);
            //print_groups();
        }
        std::vector<std::map<std::string, int>> newDFA;
        GraphNode* p;
        std::map<std::string, int> grCon;
        std::vector<bool> val;
        bool valid;
        for(const auto &i : groups){
            p = *(i.begin());
            for(const auto& j : p->con){
                grCon.insert({j.first, get_group(j.second)});
            }
            newDFA.push_back(grCon);
            valid = false;
            for(const auto &j : i){
                if(j->val){
                    valid = true;
                    break;
                }
            }
            val.push_back(valid);
            grCon = std::map<std::string, int>();
        }
        return std::make_pair(newDFA, val);
    }

    int DFA::get_group(GraphNode *n)
    {
        for(int i = 0; i < groups.size(); i++){
            for(const auto &j : groups[i]){
                if(j == n)
                    return i;
            }
        }
        return -1;
    }

    int DFA::find_appr_group(std::vector<Group>& groups, GraphNode *n)
    {
        bool appr;
        for(int i = 0; i < groups.size(); i++){
            appr = true;
            for(const auto &node : groups[i]){
                appr = comp_cons(n, node);
                if(!appr) break;
            }
            if(appr) return i;
        }
        return -1;
    }

    bool DFA::comp_cons(GraphNode *a, GraphNode *b)
    {
        int found = 0;
        for(const auto &i : a->con){
            for(const auto &j : b->con){
                if(i.first == j.first){
                    if(get_group(i.second) == get_group(j.second)){ found++; }
                    else {return false;}
                }
            }
        }
        if((found == b->con.size()) && (found == a->con.size())) return true;
        return false;
    }

    void DFA::clear_graph()
    {
        std::queue<GraphNode*> q;
        std::set<GraphNode*> visited;
        q.push(graph);
        while(!q.empty()){
            visited.insert(q.front());
            for(const auto& i : q.front()->con){
                if(visited.find(i.second) == visited.end())
                    q.push(i.second);
            }
            q.pop();
        }
        for(auto&i : visited){
            delete i;
        }
        curState = graph = nullptr;
    }

    DFA::~DFA(){
        clear_graph();
    }

    bool match(std::string str, DFA& dfa)
    {
        bool res = false;
        for(const auto& i : str){
            dfa.next_state(i);
        }
        res = dfa.is_valid();
        dfa.reset();
        return res;
    }

    std::vector<std::string> findall(std::string str, DFA& dfa)
    {
        std::vector<std::string> res;
        std::string ser;
        bool val;
        for(auto it = str.begin(); it < str.end(); it++){
            dfa.reset();
            ser = "";
            for(auto jt = it; jt < str.end(); jt++){
                val = false;
                dfa.next_state(*jt);
                while(dfa.is_valid()){
                    ser += *jt;
                    val = true;
                    dfa.next_state(*(++jt));
                }
                if(val){
                    res.push_back(ser);
                    it = jt;
                    break;
                }
                ser += *jt;
            }
        }
        dfa.reset();
        return res;
    }
    
    void DFA::print_groups()
    {
        for(int i = 0; i < groups.size(); i++){
            std::cout << "Group " << i << ":" << std::endl;
            for(const auto &j : groups[i]){
                for(const auto &k : j->st){
                    std::cout << k << " ";
                }
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }
    void MDFA::next_state(char a)
    {
        if(state == -1) return;
        for(const auto& i : table[state]){
            for(const auto& j : i.first){
                if(j == a){
                    state = i.second; 
                    return;
                }
            }
        }
        state = -1;
    }

    bool match(std::string str, MDFA &dfa)
    {
        bool res = false;
        for(const auto& i : str){
            dfa.next_state(i);
        }
        res = dfa.is_valid();
        dfa.reset();
        return res;
    }

    std::vector<std::string> findall(std::string str, MDFA &dfa)
    {
        std::vector<std::string> res;
        std::string ser;
        bool val;
        for(auto it = str.begin(); it < str.end(); it++){
            dfa.reset();
            ser = "";
            for(auto jt = it; jt < str.end(); jt++){
                val = false;
                dfa.next_state(*jt);
                while(dfa.is_valid()){
                    ser += *jt;
                    val = true;
                    dfa.next_state(*(++jt));
                }
                if(val){
                    res.push_back(ser);
                    it = jt;
                    break;
                }
                ser += *jt;
            }
        }
        dfa.reset();
        return res;
    }
    
    MDFA::Table MDFA::eliminate_states() const
    {
        Table nt = table;
        std::string loop;
        std::vector<std::pair<std::string, int>> tails;
        std::string re;
        std::map<std::string, int> nr;

        //for(int i = 1; i < table.size(); i++){
        for(int i = table.size()-1; i >= 0; i--){
            if(valid[i]) continue; 
            tails = std::vector<std::pair<std::string, int>>();
            loop = std::string();
            for(const auto& it : nt[i]){
                if(it.second == i){
                    if(loop.empty()) loop = "(!";
                    else loop.push_back('|');
                    loop += (it.first);
                    continue;
                }
                tails.push_back(it);
            }
            if(!loop.empty()) loop += ")*";
            for(int j = 0; j < nt.size(); j++){
                if(j == i) continue;
                nr = nt[j];
                for(auto it = nt[j].begin(); it != nt[j].end(); it++){
                    if(it->second == i){
                        nr.erase(it->first);
                        for(const auto& jt : tails){
                            re = it->first + loop + jt.first;
                            nr.insert(std::make_pair(re, jt.second));
                        }
                    }
                }
                nt[j] = nr;
            }
        }
        std::cout << "\nStates after elimination:\n";
        for(int i = 0; i < nt.size(); i++){
                std::cout << i << "(" << valid[i] << "): ";
                if(nt[i].empty()) { std::cout << std::endl; continue; }
                auto j = nt[i].begin();
                std::cout << "{" << j->first << ", " << j->second << "}";
                j++;
                for(j ; j != nt[i].end(); j++){
                    std::cout << ", {" << j->first << ", " << j->second << "}";
                }
                std::cout << std::endl;
            }
        return nt;
    }

    std::string MDFA::get_re() const
    {
        Table nt = eliminate_states();
        std::string reLocal;
        std::string reGlobal;
        std::string fromStart;
        std::string toStart;
        std::string startStateLoop;
        std::string currentStateLoop;
        std::string fromOtherValid;
        int locals = 0;
        for(auto & it : nt[0]){
            if(it.second == 0){
                if(startStateLoop.empty()) startStateLoop += "(!";
                else startStateLoop += "|";
                startStateLoop += it.first;
            }
        }
        if(!startStateLoop.empty()) startStateLoop += ")";
        std::cout << "startStateLoop: " << startStateLoop << std::endl;
        for(int i = 1; i < nt.size(); i++){
            if(!valid[i]) continue;
            fromStart = std::string();
            toStart = std::string();
            currentStateLoop = std::string();
            fromOtherValid = std::string();
            for(const auto& it :  nt[0]){
                if(it.second != i) continue;
                if(fromStart.empty()) fromStart = "(!";
                else fromStart += "|";
                fromStart += it.first;
            }
            for(const auto& it : nt[i]){
                if(it.second == 0){
                    if(toStart.empty()) toStart = "(!";
                    else toStart += "|";
                    toStart += it.first;
                }
                if(it.second == i){
                    if(currentStateLoop.empty()) currentStateLoop = "(!";
                    else currentStateLoop += "|";
                    currentStateLoop += it.first;
                }
            }
            for(int j = 1; j < nt.size(); j++){
                if(!valid[j] || i == j) continue;
                for(auto &it : nt[j]){
                    if(it.second == i){
                        if(fromOtherValid.empty()) fromOtherValid = "(!";
                        else fromOtherValid += "|";
                        fromOtherValid += it.first + ")";
                    }
                }
            }
            if(!fromStart.empty()) fromStart += ")";
            if(!toStart.empty()) toStart += ")";
            if(!currentStateLoop.empty()) currentStateLoop += ")*";
            
            std::cout << "\n" << i << " " << locals << ":\n";
            std::cout << "fromStart: " << fromStart << std::endl;
            std::cout << "toStart: " << toStart << std::endl;
            std::cout << "currentStateLoop: " << currentStateLoop << std::endl;
            std::cout << "reGlobal: " << reGlobal << std::endl;
            std::cout << "fromOtherValid: " << fromOtherValid << std::endl;
            reLocal = startStateLoop;
            if(!reLocal.empty()) { if(locals > 1) reLocal += "|"; locals++; }
            reLocal += (fromStart.empty() || toStart.empty() ? std::string() : (fromStart + currentStateLoop + toStart)); 
            //if(!fromOtherValid.empty()) reLocal = reLocal + (reLocal.empty() ? std::string() : "|") + fromOtherValid;
            if(!reLocal.empty()) reLocal += ")";
            reLocal += fromStart + currentStateLoop;     
            if(!reGlobal.empty()) reGlobal += "|";
            reGlobal += "(!" + reLocal + ")";       
        }
        if(reGlobal.empty()) reGlobal += startStateLoop;
        return reGlobal;
    }

    MDFA MDFA::inverse(MDFA op)
    {
        for(int i = 0; i < op.valid.size(); i++){
            op.valid[i] = !op.valid[i];
        }
        op.errorstate = !op.errorstate;
        return op;
    }

    MDFA::Multiplication MDFA::multiply(const MDFA &op1, const MDFA &op2)
    {
        Multiplication res;
        for(int i = 0; i < op1.table.size(); i++){
            for(int j = 0; j < op2.table.size(); j++){
                res.push_back(MulState(i,j));
            }
        }
        bool bond_exists;
        int to;
        
        op1.print_table();
        std::cout << "\n";
        op2.print_table();
        std::cout << "\n";

        for(int i = 0; i < res.size(); i++){
            for(const auto&it : op1.table[res[i].state[0]]){
                for(int j = 0; j < res.size(); j++){
                    if(res[j].state[0] == it.second){
                        to = j;        
                        bond_exists = false;
                        for(const auto&jt : op2.table[res[i].state[1]]){
                            if(res[to].state[1] == jt.second && it.first == jt.first){
                                bond_exists = true;
                                break;
                            }
                        }
                        if(bond_exists){
                            res[i].bonds.insert(std::make_pair(it.first, to));
                        }
                    }
                }
            }
        }
        minimize_multiplication(res);
        return res;
    }

    void MDFA::minimize_multiplication(Multiplication &mul)
    {
        int i = 1;
        bool no_bonds;
        while(i < mul.size()){
            no_bonds = true;
            for(int j = 0; j < mul.size(); j++){
                if(j == i) continue;
                for(const auto& it : mul[j].bonds){
                    if(it.second == i){ 
                        no_bonds = false;
                        break;
                    }
                }
            }
            if(!no_bonds) {i++; continue;}
            for(int j = 0; j < mul.size(); j++){
                for(auto& it : mul[j].bonds){
                    if(it.second > i){ 
                        it.second -= 1;
                    }
                }
            }
            mul.erase(mul.begin() + i);
            i = 1;
        }
    }

    MDFA MDFA::intersection(const MDFA &op1, const MDFA &op2)
    {
        auto mul = multiply(op1, op2);
        Table nt;
        std::vector<bool> nv;
        for(int i = 0; i < mul.size(); i++){
            nt.push_back(std::map<std::string, int>());
            nv.push_back(false);
        }
        for(int i = 0; i < mul.size(); i++){
            nt[i] = mul[i].bonds;
            nv[i] = op1.valid[mul[i].state[0]] && op2.valid[mul[i].state[1]];
        }
        return MDFA(nt, nv);
    }
}