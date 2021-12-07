#include "ActiveVars.hpp"
#include<map>
#include<utility>
#include<algorithm>
using namespace std;

void SetDifference(const set<Value*> &a, const set<Value*> &b, set<Value*> &target)
{
    auto it_a = a.begin();
    auto it_b = b.begin();
    while(it_a != a.end() && it_b != b.end())
    {
        if(*it_a < *it_b)
        {
            target.insert(*it_a);
            it_a++;
        }
        else if(*it_b < *it_a)
        {
            it_b++;
        }
        else
        {
            it_a++;
            it_b++;
        }
    }
    while(it_a != a.end())
    {
        target.insert(*it_a);
        it_a++;
    }
}

void ActiveVars::run()
{
    std::ofstream output_active_vars;
    output_active_vars.open("active_vars.json", std::ios::out);
    output_active_vars << "[";
    for (auto &func : this->m_->get_functions()) {
        if (func->get_basic_blocks().empty()) {
            continue;
        }
        else
        {
            func_ = func;  

            func_->set_instr_name();
            live_in.clear();
            live_out.clear();
            map<BasicBlock*, set<Value*> > live_use;
            map<BasicBlock*, set<Value*> > live_def;
            map<BasicBlock*, map<Value*, set<Value*> > > live_phi_use;
            // 在此分析 func_ 的每个bb块的活跃变量，并存储在 live_in live_out 结构内
            for(auto bb : func_->get_basic_blocks())
            {
                set<Value*> DEF;//DEF集合
                set<Value*> USE;//USE集合
                map<Value*, set<Value*> > PHI_USES;//Phi_user集合
                live_in.insert(pair<BasicBlock *, set<Value *> >{bb,set<Value*>()});
                live_out.insert(pair<BasicBlock *, set<Value *> >{bb,set<Value*>()});
                for(auto inst : bb->get_instructions())
                {
                    if(inst->is_ret() || inst->is_store() || inst->is_br())
                    {
                        for(auto op : inst->get_operands())
                        {
                            if(DEF.find(op) == DEF.end() && !dynamic_cast<Constant *>(op) && !dynamic_cast<BasicBlock*>(op))
                                USE.insert(op);
                        }
                    }//if(inst->is_void())
                    else if(inst->is_phi())
                    {
                        DEF.insert(inst);
                        auto op_list = inst->get_operands();
                        for(int i = 0; i < op_list.size() / 2; i++)
                        {
                            if(dynamic_cast<Constant *>(op_list[2 * i])) continue;
                            auto it = PHI_USES.find(op_list[2*i + 1]);
                            if( it == PHI_USES.end())//不能找到，为此bb新建phi集合
                            {
                                set<Value *> phi_value;
                                phi_value.insert(op_list[2*i]);
                                PHI_USES.insert(pair<Value*, set<Value*> >{op_list[2*i+1],phi_value});
                            }
                            else//可以找到，在此bb的phi集合中插入value。
                            {
                                it->second.insert(op_list[2*i]);
                            }
                        }
                    }//else if(inst->is_phi())
                    else if(inst->is_call())
                    {
                        auto op_list = inst->get_operands();
                        for(int i = 1; i < op_list.size(); i++)
                        {
                            if(DEF.find(op_list[i]) == DEF.end() && !dynamic_cast<Constant *>(op_list[i]))
                                USE.insert(op_list[i]);
                        }
                        if(!inst->get_type()->is_void_type())
                        {
                            if(USE.find(inst) == USE.end())
                                DEF.insert(inst);
                        }
                    }//else if(inst->is_call())
                    else
                    {
                        for(auto op : inst->get_operands())
                        {
                            if(DEF.find(op) == DEF.end() && !dynamic_cast<Constant *>(op))
                                USE.insert(op);
                        }
                        if(USE.find(inst) == USE.end())
                            DEF.insert(inst);
                    }//else
                }//for(auto inst : bb->get_instructions())

                live_use.insert(pair<BasicBlock*, set<Value*> >{bb,USE});
                live_def.insert(pair<BasicBlock*, set<Value*> >{bb,DEF});
                live_phi_use.insert(pair<BasicBlock*, map<Value*, set<Value*> > >{bb,PHI_USES});
            }

            bool is_in_modified = true;

            while(is_in_modified)
            {
                is_in_modified = false;
                for(auto bb : func_->get_basic_blocks())
                {
                    auto &out = live_out.find(bb)->second;
                    auto &in = live_in.find(bb)->second;
                    //OUT[B] = U IN[S] U PHI[S,B]
                    for(auto succ: bb->get_succ_basic_blocks())
                    {
                        const auto &in_succ = live_in.find(succ)->second;
                        for(auto value : in_succ)
                            out.insert(value);
                        const auto &succ_phi_list = live_phi_use.find(succ)->second;
                        auto it = succ_phi_list.find(bb);
                        if(it == succ_phi_list.end()) continue;
                        const auto &succ_phi = it->second;
                        for(auto value : succ_phi)
                            out.insert(value);
                    }
                    //IN[B] = USE[B] U (OUT[B] - DEF[B])

                    auto bb_def = live_def.find(bb)->second;
                    auto bb_use = live_use.find(bb)->second;
                    set<Value*> Right_set;
                    // SetDifference(out, bb_def, Right_set);
                    set_difference(out.begin(),out.end(),bb_def.begin(),bb_def.end(),inserter(Right_set, Right_set.begin()));
                    for(auto value : Right_set)
                    {
                        auto check = in.insert(value);
                        if(check.second) is_in_modified = true;
                    }
                    for(auto value : bb_use)
                    {
                        auto check = in.insert(value);
                        if(check.second) is_in_modified = true;
                    }
                }//for(bb)
            }//while
            for(auto bb : func_->get_basic_blocks())
            {
                auto &in = live_in.find(bb)->second;
                auto phi_list_map = live_phi_use.find(bb)->second;
                for(auto &phi_list_it : phi_list_map)
                {
                    auto &phi_list = phi_list_it.second;
                    for(auto value : phi_list)
                    {
                        if(dynamic_cast<Constant*>(value)) continue;
                        in.insert(value);
                    }
                }
            }
 
            auto i = live_in.size();

            output_active_vars << print();
            output_active_vars << ",";
        }
    }
    output_active_vars << "]";
    output_active_vars.close();
    return ;
}

std::string ActiveVars::print()
{
    std::string active_vars;
    active_vars +=  "{\n";
    active_vars +=  "\"function\": \"";
    active_vars +=  func_->get_name();
    active_vars +=  "\",\n";

    active_vars +=  "\"live_in\": {\n";
    for (auto &p : live_in) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars +=  "  \"";
            active_vars +=  p.first->get_name();
            active_vars +=  "\": [" ;
            for (auto &v : p.second) {
                active_vars +=  "\"%";
                active_vars +=  v->get_name();
                active_vars +=  "\",";
            }
            active_vars += "]" ;
            active_vars += ",\n";   
        }
    }
    active_vars += "\n";
    active_vars +=  "    },\n";
    
    active_vars +=  "\"live_out\": {\n";
    for (auto &p : live_out) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars +=  "  \"";
            active_vars +=  p.first->get_name();
            active_vars +=  "\": [" ;
            for (auto &v : p.second) {
                active_vars +=  "\"%";
                active_vars +=  v->get_name();
                active_vars +=  "\",";
            }
            active_vars += "]";
            active_vars += ",\n";
        }
    }
    active_vars += "\n";
    active_vars += "    }\n";

    active_vars += "}\n";
    active_vars += "\n";
    return active_vars;
}