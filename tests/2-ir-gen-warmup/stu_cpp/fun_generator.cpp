#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG                                             // 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl; // 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
    ConstantInt::get(num, module)

#define CONST_FP(num) \
    ConstantFP::get(num, module) // 得到常数值的表示,方便后面多次用到

int main()
{
    auto module = new Module("Cminus code"); // module name是什么无关紧要
    auto builder = new IRBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);

    auto calleeFun = Function::create(
        FunctionType::get(
            Int32Type,
            std::vector<Type *>(1, Int32Type)),
        "callee",
        module);

    auto bb = BasicBlock::create(module, "entry", calleeFun);
    builder->set_insert_point(bb);

    auto *aValue = *(calleeFun->arg_begin());
    auto mul_res = builder->create_imul(CONST_INT(2), aValue);
    builder->create_ret(mul_res);

    auto mainFun = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    bb = BasicBlock::create(module, "entry", mainFun);

    builder->set_insert_point(bb);
    builder->create_ret(
        builder->create_call(
            calleeFun,
            {CONST_INT(110)}));

    std::cout << module->print();
    delete module;
    return 0;
}