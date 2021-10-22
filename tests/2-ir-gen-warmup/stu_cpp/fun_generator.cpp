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