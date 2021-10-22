#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl;
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
    ConstantInt::get(num, module)

#define CONST_FP(num) \
    ConstantFP::get(num, module)
int main()
{
    auto module = new Module("Cminus code");
    auto builder = new IRBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                    "main", module);

    auto bb = BasicBlock::create(module, "entry", mainFun);

    builder->set_insert_point(bb);

    auto *aAlloca = builder->create_alloca(Int32Type);
    auto *iAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(10), aAlloca);
    builder->create_store(CONST_INT(0), iAlloca);
    auto loopEntry = BasicBlock::create(module, "loopEntry", mainFun);
    builder->create_br(loopEntry);

    auto trueBB = BasicBlock::create(module, "trueBB", mainFun);
    auto falseBB = BasicBlock::create(module, "falseBB", mainFun);
    {
        builder->set_insert_point(loopEntry);
        auto iValue = builder->create_load(iAlloca);
        auto compValue = builder->create_icmp_lt(iValue, CONST_INT(10));
        builder->create_cond_br(compValue, trueBB, falseBB);
    }
    {
        builder->set_insert_point(trueBB);
        auto iValue0 = builder->create_load(iAlloca);
        auto iAddRes = builder->create_iadd(iValue0, CONST_INT(1));
        builder->create_store(iAddRes, iAlloca);
        auto iValue1 = builder->create_load(iAlloca);
        auto aValue = builder->create_load(aAlloca);
        auto aAddRes = builder->create_iadd(aValue, iValue1);
        builder->create_store(aAddRes, aAlloca);
        builder->create_br(loopEntry);
    }
    {
        builder->set_insert_point(falseBB);

        auto aValue = builder->create_load(aAlloca);
        builder->create_ret(aValue);
    }

    std::cout
        << module->print();
    delete module;
    return 0;
}