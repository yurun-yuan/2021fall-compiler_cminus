# Group Discussions

组内交流记录

## 自建基础设施文档

一些可以复用的函数、全局变量、宏定义，可以在这里介绍，从而方便其他组员使用，避免重复劳动。

1. 将`enum CminusType`转换为`Type* `类型
   ```cpp
   /**
    * @brief Convert `enum CminusType` to `Type *`
    * @arg builder: just pass the builder in CminusfBuilder to it
    * @arg CminusType: a `enum CminusType` value
    * @return  a `Type *` value. 
    * 
    */
    Type *CminusTypeConvertor(IRBuilder *builder, enum CminusType c);
   ```
2. 强制类型转换
   ```cpp
   /**
    * @brief Compulsively convert value `origin` to `target` type.
    * 
    * e.g. If `a` is of type `int` and has a non-zero value, 
    * `compulsiveTypeConvert(a, GET_BOOL)` will change `a` to 
    * a bool type with value true. 
    * 
    */
    void compulsiveTypeConvert(Value *&origin, Type *target);
   ```
3. 提升类型转化
   ```cpp

   /**
    * @brief Convert left/right to the same type
    * 
    * e.g. If left is of type bool, right is of type int, 
    * then left is converted to type int with value 1. 
    * 
    * The rank of types are as following: 
    * 1. float
    * 2. int
    * 3. bool
    * 
    * The conversion can only be performed from a lower-rank type
    * to a high-rank type. 
    * 
    * @param left 
    * @param right 
    * @return Type* The type of left and right after conversion.
    */
    Type *augmentTypeConvert(Value *&left, Value *&right);

   /**
    * @brief Same as `Type *augmentTypeConvert(Value *&left, Value *&right)`
    * with the exception that, after conversion, the rank of
    * the type of left&right cannot be lower than min.
    * 
    * e.g. if left and right are both of type bool, 
    * after running `compulsiveTypeConvert(left, right, GET_INT)`,
    * both of them are converted to int. 
    * @return Type* 
    */

    Type *augmentTypeConvert(Value *&left, Value *&right, Type *min);

   ```

## 难点提醒

1. 表达式求值：使用全局的栈
   一些细节：栈中保存表达式的值而非指针，计算前无需load。

2. implicit 类型转换（详见助教文档）

3. 函数声明中进入新作用域的时机
   个人意见：为了统一性，在`compoundStmt`的`visit`函数中进入新作用域(i.e. `scope.enter()`). 
   这样做会遇到一些麻烦：函数参数作用域应在`compoundStmt`的新作用域中，但我们在`compoundStmt`的`visit`中无法直接访问参数列表，而是在`functionDeclaration.visit()`（即调用`compoundStmt.visit()`的父函数）中访问。我的做法是使用全局变量将参数列表(的指针)作为继承属性传递。

4. 数组类型转化为指针的时机
   `Ty arr[]`与`Ty *ptr`为不同类型，但在传递参数时、参与运算时(如`arr + n`，虽然我们不用支持这个)前者会隐式转换为后者。
   Solution：如果数组在表达式中单独成为运算元，则转化为指针类型。
   
5. TODO

   优化：数值未被使用的表达式可不求值。

6. 类型转换时需考虑`int1`类型

7. `scope` 中的类型问题
   `scope` 中可能出现`GlobalVariable`, `AllocaInst`, `Function`类型。在取出元素后难以对数据类型判断。
   Solution: 根据查看lightir代码，`GlobalVariable`, `AllocaInst`在`create`时均把变量类型的指针类型作为其`type`域的值。使用`var->get_type()->get_pointer_element_type()`即可得到定义的变量的类型。
   
8. 未考虑`if`、`while`条件表达式结果类型转换
   需将表达式结果转换为`int1`类型。

9. 未考虑终止语句的多样性
   一个基本块可以以`ret`或`br`结束。而对一个statement而言，可以如下定义它是*terminal statement*:
   $$
   \begin{align}
   \textit{terminalStmt}&\rightarrow \textbf{return}\ \mathit{expr}\\
   &\rightarrow\textbf{compound statement}\ \mathit{\{\cdots;\ terminalStmt;\ \cdots\}}\\
   &\rightarrow\mathbf{if}\ \mathit{terminalStmt}\ \mathbf{else}\ \mathit{terminalStmt}
   \end{align}
   $$
   当在`compound-statement`中遇到`terminalStmt`是，后续语句可直接跳过。
   
   此外，这也是为了处理有时无需插入`br`的控制语句。例如
   
   ```c
   if(expr)
       terminalStmt
   ```
   
   则`if-statement`结尾除无需再添加`br`指令。
   
   再如
   
   ```c
   if(expr)
       terminalStmt
   else
       terminalStmt
   ```
   
   此`if`语句本身也是`terminalStmt`。则`if-statement`, `else-statement`结尾处均无需加`br`，且整个`if-else`语句后也不应加入BB插入点。
   
10. `void-function` without `return` statement

    返回值为`void`的函数可以不含`return`语句。
    这一点在使用`terminalStmt`这一概念后十分容易解决。只需要在`functionDeclaration.visit()`中`accept(compond_stmt)`后判断`compound_stmt`是否为`terminalStmt`。若不是，说明它不含`return`语句，此时补充`ret`. 
