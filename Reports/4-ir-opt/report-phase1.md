# Lab4 实验报告-阶段一

小组成员 姓名 学号

袁玉润 PB19111692 (组长)

王历典 PB19111691

唐志鹏 PB19111690

## 实验要求

请按照自己的理解，写明本次实验需要干什么

1. 通过阅读`Mem2Reg`与`LoopSearch`两个优化 pass 的代码学习优化的基本流程以及 lightir 接口的使用
2. 利用 lightir 接口开发**常量传播与死代码删除、循环不变式外提、活跃变量分析**这三个优化 pass

## 思考题
### LoopSearch
1. `std::unordered_set<BasicBlock *>`

2. 在函数`find_loop_base`中查找，具体解释如下：

   ```cpp
   CFGNodePtr LoopSearch::find_loop_base(
       CFGNodePtrSet *set,
       CFGNodePtrSet &reserved)//输入一个强连通分量set和一个已经记录的base的集合reserved
   {
   
       CFGNodePtr base = nullptr;
       for (auto n : *set)//遍历set中所有的节点
       {
           for (auto prev : n->prevs)//遍历节点中的前驱
           {
               if (set->find(prev) == set->end())//如果有一个前驱不在set中，将其设为base
               {
                   base = n;
               }
           }
       }
       if (base != nullptr)
           return base;
       //这个是在删除记录过的base节点后的补救方案，如果再上一个循环中没有找到base，则在这里记录中的base节点的在此set中的后继，等价于前驱在set外的节点。
       for (auto res : reserved)
       {
           for (auto succ : res->succs)
           {
               if (set->find(succ) != set->end())
               {
                   base = succ;
               }
           }
       }
   
       return base;
   }
   ```

3. 在第一次遍历连通分量时，将每一个base结点在CFG中删除，这样就破坏了外层循环，进行第二次遍历前找到的强连通分量就是内部循环，以此往复，可以找到所有的内部循环。

4. 通过以下函数找到最内层循环：

   ```cpp
   BBset_t *get_inner_loop(BasicBlock* bb){
       if(bb2base.find(bb) == bb2base.end())
           return nullptr;
       return base2loop[bb2base[bb]];
   }
   ```

   其中用到了两个数据`bb2base`和`base2loop`，即先通过basicblock找到base节点，在找到base节点对应的loop。

   其中`bb2base`在每次循环中含有当前基本块时都会用新数据覆盖当前数据。

   ```cpp
   for (auto bb : *bb_set)
   {
       if (bb2base.find(bb) == bb2base.end())
           bb2base.insert({bb, base->bb});
       else
           bb2base[bb] = base->bb;
   }
   ```

   而`base2loop`则是每个循环新建记录：

   ```cpp
   base2loop.insert({base->bb, bb_set});
   ```

### Mem2reg
1. 请**简述**概念：支配性、严格支配性、直接支配性、支配边界。

   * 支配性

     若每一条从`entry`到节点`n`的路径都经过`d`，则称`d`支配`n`，记为`d dom n`. 

   * 严格支配性

     在*支配性*的定义中，对任意节点`n`有`n dom n`。*`d`严格支配`n`*指`d dom n`且`d != n`. 

   * 直接支配性

     若`d`满足：

     1. `d dom n`
     2. $\forall e$满足$e \text{ dom }n$，有$e \text{ dom }d$.

     则称`d`直接支配`n`. 

   * 支配边界

     `n`的支配边界是满足如下性质的节点`x`的集合：

     1. `x`的某个前继被`n`支配，且
     2. `n`并不严格支配`x`.

2. `phi`节点是SSA的关键特征，请**简述**`phi`节点的概念，以及引入`phi`节点的理由。

   $ \Phi$节点是具有$ \Phi$函数的节点。其引入是为解决：在SSA中一些寄存器的赋值与流图中到达此节点的路径有关。

   由于单赋值的特性，不同基本块中对相同变量的赋值会被翻译为对不同寄存器的赋值，这对汇合点(joint)中的变量引用造成困难，即无法判断应使用的寄存器值。$\Phi$函数允许根据进入此基本块的入边判断应使用的寄存器。

3. 下面给出的`cminus`代码显然不是SSA的，后面是使用lab3的功能将其生成的LLVM IR（**未加任何Pass**），说明对一个变量的多次赋值变成了什么形式？

   `cminus`代码：

   ```c
   int main(void){
       int a;
       a = 0;
       a = 1 + 2;
       a = a * 4;
       return 0;
   }
   ```

   生成的LLVM IR：

   ```c
   ; ModuleID = 'cminus'
   source_filename = "non_ssa.cminus"
   
   declare i32 @input()
   
   declare void @output(i32)
   
   declare void @outputFloat(float)
   
   declare void @neg_idx_except()
   
   define i32 @main() {
   label_entry:
     %op0 = alloca i32
     store i32 0, i32* %op0
     %op1 = add i32 1, 2
     store i32 %op1, i32* %op0
     %op2 = load i32, i32* %op0
     %op3 = mul i32 %op2, 4
     store i32 %op3, i32* %op0
     ret i32 0
   }
   ```

   对单个变量的多次赋值变为对不同寄存器的赋值及将寄存器值store。

4. 对下面给出的`cminus`程序，使用lab3的功能，分别关闭/开启`Mem2Reg`生成LLVM IR。对比生成的两段LLVM IR，开启`Mem2Reg`后，每条`load`, `store`指令发生了变化吗？变化或者没变化的原因是什么？请分类解释。

   `cminus`代码：

   ```c
   int globVar;
   int func(int x){
       if(x > 0){
           x = 0;
       }
       return x;
   }
   int main(void){
       int arr[10];
       int b;
       globVar = 1;
       arr[5] = 999;
       b = 2333;
       func(b);
       func(globVar);
       return 0;
   }
   ```

   关闭`Mem2Reg`生成的LLVM IR：

   ```c
   ; ModuleID = 'cminus'
   source_filename = "mem2reg_example.cminus"
   
   @globVar = global i32 zeroinitializer
   declare i32 @input()
   
   declare void @output(i32)
   
   declare void @outputFloat(float)
   
   declare void @neg_idx_except()
   
   define i32 @func(i32 %arg0) {
   label_entry:
     %op1 = alloca i32
     store i32 %arg0, i32* %op1
     %op2 = load i32, i32* %op1
     %op3 = icmp sgt i32 %op2, 0
     %op4 = zext i1 %op3 to i32
     %op5 = icmp ne i32 %op4, 0
     br i1 %op5, label %label6, label %label7
   label6:                                                ; preds = %label_entry
     store i32 0, i32* %op1
     br label %label7
   label7:                                                ; preds = %label_entry, %label6
     %op8 = load i32, i32* %op1
     ret i32 %op8
   }
   define i32 @main() {
   label_entry:
     %op0 = alloca [10 x i32]
     %op1 = alloca i32
     store i32 1, i32* @globVar
     %op2 = icmp slt i32 5, 0
     br i1 %op2, label %label3, label %label4
   label3:                                                ; preds = %label_entry
     call void @neg_idx_except()
     ret i32 0
   label4:                                                ; preds = %label_entry
     %op5 = getelementptr [10 x i32], [10 x i32]* %op0, i32 0, i32 5
     store i32 999, i32* %op5
     store i32 2333, i32* %op1
     %op6 = load i32, i32* %op1
     %op7 = call i32 @func(i32 %op6)
     %op8 = load i32, i32* @globVar
     %op9 = call i32 @func(i32 %op8)
     ret i32 0
   }
   ```

   开启了`Mem2Reg`生成的LLVM IR：

   ```c
   ; ModuleID = 'cminus'
   source_filename = "mem2reg_example.cminus"
   
   @globVar = global i32 zeroinitializer
   declare i32 @input()
   
   declare void @output(i32)
   
   declare void @outputFloat(float)
   
   declare void @neg_idx_except()
   
   define i32 @func(i32 %arg0) {
   label_entry:
     %op3 = icmp sgt i32 %arg0, 0
     %op4 = zext i1 %op3 to i32
     %op5 = icmp ne i32 %op4, 0
     br i1 %op5, label %label6, label %label7
   label6:                                                ; preds = %label_entry
     br label %label7
   label7:                                                ; preds = %label_entry, %label6
     %op9 = phi i32 [ %arg0, %label_entry ], [ 0, %label6 ]
     ret i32 %op9
   }
   define i32 @main() {
   label_entry:
     %op0 = alloca [10 x i32]
     store i32 1, i32* @globVar
     %op2 = icmp slt i32 5, 0
     br i1 %op2, label %label3, label %label4
   label3:                                                ; preds = %label_entry
     call void @neg_idx_except()
     ret i32 0
   label4:                                                ; preds = %label_entry
     %op5 = getelementptr [10 x i32], [10 x i32]* %op0, i32 0, i32 5
     store i32 999, i32* %op5
     %op7 = call i32 @func(i32 2333)
     %op8 = load i32, i32* @globVar
     %op9 = call i32 @func(i32 %op8)
     ret i32 0
   }
   ```

   除对`globVar`的`load`以外，`load`、`store`被全部删除。

   * `load`被删除的原因

     在获取变量值时使用`var_val_stack[l_val].back()`得到相应寄存器，从而无需`load`. 

   * `store`被删除的原因

     对变量赋值时改为了对寄存器赋值，且记录了存有变量最新值的寄存器，因此无需`store`. 

   * `globVar`的`load`被保留的原因

     `globVar`是全局变量，需跨函数共享，仅用寄存器记录其值是不够的。

   删除`store`和`load`指令的代码对应`Mem2Reg.cpp`中第103行和第116行：

   ```c++
   if ( instr->is_load() )
   {
       // step 4: replace load with the top of stack[l_val]
       auto l_val = static_cast<LoadInst *>(instr)->get_lval();
       if (!IS_GLOBAL_VARIABLE(l_val) && !IS_GEP_INSTR(l_val))
       {
           if ( var_val_stack.find(l_val)!=var_val_stack.end())
           {
               instr->replace_all_use_with(var_val_stack[l_val].back());
               wait_delete.push_back(instr);                // Line 103
           }
       }
   }
   if (instr->is_store())
   {
       // step 5: push r_val of store instr as lval's lastest value define
       auto l_val = static_cast<StoreInst *>(instr)->get_lval();
       auto r_val = static_cast<StoreInst *>(instr)->get_rval();
       if (!IS_GLOBAL_VARIABLE(l_val) && !IS_GEP_INSTR(l_val))
       {
           var_val_stack[l_val].push_back(r_val);
           wait_delete.push_back(instr);                   // Line 116
       }
   } 
   ```

5. 指出放置phi节点的代码，并解释是如何使用支配树的信息的。需要给出代码中的成员变量或成员函数名称。

   完整生成$ \Phi$指令分为两步：

   1. `Mem2Reg::generate_phi`中插入$ \Phi$指令

      `Mem2Reg.cpp: line 66`: 

      ```cpp
      auto phi = PhiInst::create_phi(var->get_type()->get_pointer_element_type(), bb_dominance_frontier_bb);
      ```

   2. `Mem2Reg::re_name`中在$ \Phi$指令中加入(寄存器-前继)节点的配对。

      `Mem2Reg.cpp: line 132`:

      ```cpp
      static_cast<PhiInst *>(instr)->add_phi_pair_operand(var_val_stack[l_val].back(), bb);
      ```

   在寻找$ \Phi$节点的过程中(在`Mem2Reg::generate_phi`函数里)，使用`Dominators::get_dominance_frontier`查询支配边界，并在支配边界的基本块内插入$ \Phi$指令。

   在`Mem2Reg::re_name`中，利用`Dominators::get_dom_tree_succ_blocks`查找支配树中后继，从而实现了对支配树的DFS。

### 代码阅读总结

此次实验有什么收获

1. 通过代码实现过程的学习掌握了基于 lightir 开发优化 pass 的方法，对第二阶段的工作有了大致的理解
2. 较为完整地了解了`LoopSearch`和`Dominators`各个接口的具体实现和使用方法
3. 了解了`phi`指令的功能，以及`Men2Reg`中添加`phi`指令和删除部分`load`、`store`的过程

### 实验反馈 （可选 不会评分）

对本次实验的建议

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息