# Lab4 实验报告

小组成员 姓名 学号

袁玉润 PB19111692 (组长)

王历典 PB19111691

唐志鹏 PB19111690

## 实验要求

请按照自己的理解，写明本次实验需要干什么

## 实验难点

实验中遇到哪些挑战

## 实验设计

* 常量传播

    1. 实现思路：

       有以下3个地方的优化

       1. 将计算为常数的寄存器替换为字面常量，并将原寄存器赋值语句删除。

          这里的难点有

          1. 找到所有的运算

             包括整数/浮点的四则运算、比较运算、类型转换运算、位拓展运算。

          2. 注意在遍历bb的instructions顺序表时勿对其做修改，避免可能的Iterator失效等问题。使用延迟删除/增添

          3. 得益于SSA的特性，只需对流图做1次遍历，只要确保任意节点的支配节点在其之前被访问。本实现中使用DFS。

       2. 将条件为常值的条件br改为无条件br

          值得注意的有

          1. 修改后注意对流图进行修改（即调整前后继列表）。

       3. 对块内全局变量实现部分mem2reg

          仿照`mem2reg.cpp`中局部思路。由于不涉及块间调整，实现简单许多。

    2. 相应代码：

       1. 将计算为常数的寄存器替换为字面常量，并将原寄存器赋值语句删除。

          ```cpp
          for (auto instr : bb->get_instructions()){
              ...
              // `const_compute` is used to determine 
              // if the result is a constant at compile
              // time and, if it is, to compute the value. 
              auto res = const_compute(instr);
              if (res)
              {
                  instr->replace_all_use_with(res);
                  wait_delete.push_back(instr);
              }
              ...
          }
          ```

       2. 将条件为常值的条件br改为无条件br

          ```cpp
          for (auto instr : bb->get_instructions()){
              ...
              if (instr->is_br()) // instr is a branch instruction
              {
                  auto br_instr = dynamic_cast<BranchInst *>(instr);
                  if (br_instr->is_cond_br()) // instr is a conditional branch instruction
                  {
                      auto cond = dynamic_cast<ConstantInt *>(br_instr->get_operand(0));
                      if (cond) // The condition is a constant
                      {
                          auto cond_val = cond->get_value();
                          BasicBlock *goto_bb, *other_bb;
                          
                          // `goto_bb` is the block we branch unconditionally to. 
                          // `other_bb` will never be branched to. 
                          goto_bb = dynamic_cast<BasicBlock *>(br_instr->get_operand(2 - cond_val));
                          other_bb = dynamic_cast<BasicBlock *>(br_instr->get_operand(cond_val + 1));
                          
                          to_add_truebb = goto_bb;
                          wait_delete.push_back(instr);
                          
                          // Modify flow graph.
                          bb->remove_succ_basic_block(other_bb);
                          other_bb->remove_pre_basic_block(bb);
                      }
                  }
              }
              ...
          }
          
          // Add new br here
          ```

       3. 对块内全局变量实现部分mem2reg

          ```cpp
          for (auto instr : bb->get_instructions()){
              ...
              if (instr->is_store())
                  // `store` instructions are left unchanged, but 
                  // it is used to bookkeep the latest value of 
                  // a global variable. 
                  latest_val[instr->get_operand(1)] = instr->get_operand(0);
              else if (instr->is_load())
              {
                  if (latest_val.find(instr->get_operand(0)) == latest_val.end())
                      // If no register have latest value, a `load` is needed. 
                      latest_val[instr->get_operand(0)] = instr;
                  else
                  {
                      // Use the register to replace `load`
                      instr->replace_all_use_with(latest_val[instr->get_operand(0)]);
                      wait_delete.push_back(instr);
                  }
              }
              ...
          }
          ```

    3. 优化前后的IR对比（举一个例子）并辅以简单说明：

       以`testcase3`中`max`函数的`entry_basic_block`为例：

       ```c
       int max(void)
       {
       
       ========== Entry basic block ===========
           opa = 0*1*2*3*4*5*6*7;
           opb = 1*2*3*4*5*6*7*8;
           opc = 2*3*4*5*6*7*8*9;
           opd = 3*4*5*6*7*8*9*10;
       
           if( opa < opb){
       ========== Entry basic block ===========
       
               if( opb < opc){
                   if( opc < opd){
                       return opd;
                   }
               }
           }
           return 0;
       }
       ```

       优化前：

       ```llvm
       label_entry:
         %op0 = mul i32 0, 1
         %op1 = mul i32 %op0, 2
         %op2 = mul i32 %op1, 3
         %op3 = mul i32 %op2, 4
         %op4 = mul i32 %op3, 5
         %op5 = mul i32 %op4, 6
         %op6 = mul i32 %op5, 7
         store i32 %op6, i32* @opa
         %op7 = mul i32 1, 2
         %op8 = mul i32 %op7, 3
         %op9 = mul i32 %op8, 4
         %op10 = mul i32 %op9, 5
         %op11 = mul i32 %op10, 6
         %op12 = mul i32 %op11, 7
         %op13 = mul i32 %op12, 8
         store i32 %op13, i32* @opb
         %op14 = mul i32 2, 3
         %op15 = mul i32 %op14, 4
         %op16 = mul i32 %op15, 5
         %op17 = mul i32 %op16, 6
         %op18 = mul i32 %op17, 7
         %op19 = mul i32 %op18, 8
         %op20 = mul i32 %op19, 9
         store i32 %op20, i32* @opc
         %op21 = mul i32 3, 4
         %op22 = mul i32 %op21, 5
         %op23 = mul i32 %op22, 6
         %op24 = mul i32 %op23, 7
         %op25 = mul i32 %op24, 8
         %op26 = mul i32 %op25, 9
         %op27 = mul i32 %op26, 10
         store i32 %op27, i32* @opd
         %op28 = load i32, i32* @opa
         %op29 = load i32, i32* @opb
         %op30 = icmp slt i32 %op28, %op29
         %op31 = zext i1 %op30 to i32
         %op32 = icmp ne i32 %op31, 0
         br i1 %op32, label %label33, label %label39
       ```

       优化后：

       ```llvm
       label_entry:
         store i32 0, i32* @opa
         store i32 40320, i32* @opb
         store i32 362880, i32* @opc
         store i32 1814400, i32* @opd
         br label %label33
       ```

       可以看到

       1. 所有可计算的常量均以字面常量形式硬编码入IR
       2. 末尾的条件br被改为无条件br
       3. 对值为常量的全局变量引用不会引发`load`. 

    4. 运行结果

       ![image-20211204163315031](report-phase2.assets/image-20211204163315031.png)


* 循环不变式外提
    实现思路：
    相应代码：
    优化前后的IR对比（举一个例子）并辅以简单说明：
    
* 活跃变量分析
    实现思路：
    相应的代码：

### 实验总结

此次实验有什么收获

### 实验反馈 （可选 不会评分）

对本次实验的建议

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息
