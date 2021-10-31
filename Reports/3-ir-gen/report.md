# Lab3 实验报告

小组成员 姓名 学号


## 实验难点

实验中遇到哪些挑战

1. 使用栈对表达式求值

2. implicit 类型转换

3. 函数声明中进入新作用域的时机

4. 数组类型转化为指针的时机

   Solution：如果数组在表达式中，则转化为指针类型。
   
5. TODO

   优化：数值未被使用的表达式可不求值。

6. 类型转换时需考虑`int1`类型

7. `scope` 中的类型问题
   `scope` 中可能出现`GlobalVariable`, `AllocaInst`, `Function`类型。在取出元素后难以对数据类型判断。
   Solution: `GlobalVariable`, `AllocaInst`在`create`时均把变量类型的指针类型作为其`type`域的值。使用`var->get_type()->get_pointer_element_type()`即可得到定义的变量的类型。
   
8. 未考虑`if`、`while`条件表达式结果类型转换

9. 未考虑终止语句的多样性
   一个基本块可以以`ret`或`br`结束。而对一个statement而言，可以如下定义它是*terminate statement*:
   $$
   \begin{align}
   \textit{terminateStmt}&\rightarrow \textbf{return}\ \mathit{expr}\\
   &\rightarrow\textbf{compound statement}\ \mathit{\{\cdots;\ terminateStmt;\ \cdots\}}\\
   &\rightarrow\mathbf{if}\ \mathit{terminateStmt}\ \mathbf{else}\ \mathit{terminateStmt}
   \end{align}
   $$
   当在`compound-statement`中遇到`terminateStmt`是，后续语句可直接跳过。
   
   此外，这也是为了处理有时无需插入`br`的控制语句。例如
   
   ```c
   if(expr)
       terminateStmt
   ```
   
   则`if-statement`结尾除无需再添加`br`指令。
   
   再如
   
   ```c
   if(expr)
       terminateStmt
   else
       terminateStmt
   ```
   
   此`if`语句本身也是`terminateStmt`。则`if-statement`, `else-statement`结尾处均无需加`br`，且整个`if-else`语句后也不应加入BB插入点。
   
10. `void-function` without `return` statement

    返回值为`void`的函数可以不含`return`语句。

## 实验设计

请写明为了顺利完成本次实验，加入了哪些亮点设计，并对这些设计进行解释。
可能的阐述方向有:

1. 如何设计全局变量
2. 遇到的难点以及解决方案
3. 如何降低生成 IR 中的冗余
4. ...


### 实验总结

此次实验有什么收获

### 实验反馈 （可选 不会评分）

对本次实验的建议

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息
