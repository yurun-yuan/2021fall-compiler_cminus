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

    auto Int32Type = Type::get_int32_type(module);
    auto *IntArray10Type = ArrayType::get(Int32Type, 10);

    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                    "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);

    auto aAlloca = builder->create_alloca(IntArray10Type);

    auto aIndex0=builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(0)});
    builder->create_store(CONST_INT(10), aIndex0);
    auto aValue0 = builder->create_load(aIndex0);
    auto mul_res = builder->create_imul(aValue0, CONST_INT(2));
    auto aIndex1 = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(1)});
    builder->create_store(mul_res, aIndex1);
    auto aValue1 = builder->create_load(aIndex1);
    builder->create_ret(aValue1);

    std::cout << module->print();
    delete module;
    return 0;
}