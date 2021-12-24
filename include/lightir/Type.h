#ifndef SYSYC_TYPE_H
#define SYSYC_TYPE_H

#include <vector>
#include <iostream>
#include <string>
#include <cassert>

class Module;
class IntegerType;
class FunctionType;
class ArrayType;
class PointerType;
class FloatType;

class Type
{
public:
    enum TypeID
    {
        VoidTyID,     // Void
        LabelTyID,    // Labels, e.g., BasicBlock
        IntegerTyID,  // Integers, include 32 bits and 1 bit
        FunctionTyID, // Functions
        ArrayTyID,    // Arrays
        PointerTyID,  // Pointer
        FloatTyID,    // float
        StructTyID,   // struct
        ReferenceTyID // Reference
    };

    explicit Type(TypeID tid, Module *m);
    ~Type() = default;

    TypeID get_type_id() const { return tid_; }

    bool is_void_type() const { return get_type_id() == VoidTyID; }

    bool is_label_type() const { return get_type_id() == LabelTyID; }

    bool is_integer_type() const { return get_type_id() == IntegerTyID; }

    bool is_function_type() const { return get_type_id() == FunctionTyID; }

    bool is_struct_type() const { return get_type_id() == StructTyID; }

    bool is_array_type() const { return get_type_id() == ArrayTyID; }

    bool is_pointer_type() const { return get_type_id() == PointerTyID; }

    bool is_float_type() const { return get_type_id() == FloatTyID; }

    bool is_reference() const { return get_type_id() == ReferenceTyID; }

    static bool is_eq_type(Type *ty1, Type *ty2);

    static Type *get_void_type(Module *m);

    static Type *get_label_type(Module *m);

    static IntegerType *get_int1_type(Module *m);

    static IntegerType *get_int32_type(Module *m);

    static PointerType *get_int32_ptr_type(Module *m);

    static FloatType *get_float_type(Module *m);

    static PointerType *get_float_ptr_type(Module *m);

    static PointerType *get_pointer_type(Type *contained);

    static ArrayType *get_array_type(Type *contained, unsigned num_elements);

    Type *get_pointer_element_type();

    Type *get_array_element_type();

    int get_size();

    Module *get_module();

    virtual std::string print();

    virtual Type *get_print_type()
    {
        return this;
    }

private:
    TypeID tid_;
    Module *m_;
};

class IntegerType : public Type
{
public:
    explicit IntegerType(unsigned num_bits, Module *m);

    static IntegerType *get(unsigned num_bits, Module *m);

    unsigned get_num_bits();

private:
    unsigned num_bits_;
};

class PointerType : public Type
{
public:
    PointerType(Type *contained);
    Type *get_element_type() const { return contained_; }

    static PointerType *get(Type *contained);

    virtual Type *get_print_type() override
    {
        return PointerType::get(contained_->get_print_type());
    }

private:
    Type *contained_; // The element type of the ptr.
};

class FunctionType : public Type
{
public:
    FunctionType(Type *result, std::vector<Type *> params);

    static bool is_valid_return_type(Type *ty);
    static bool is_valid_argument_type(Type *ty);

    static FunctionType *get(Type *result,
                             std::vector<Type *> params, Module *m);

    unsigned get_num_of_args() const;

    /**
     * @brief Get the wrapped function type, i.e., from a function pointer
     * 
     * @param src 
     * @return FunctionType* 
     */
    static FunctionType *get_wrapped_function_type(Type *src)
    {
        FunctionType *res;
        if (src->is_function_type())
            res = dynamic_cast<FunctionType *>(src);
        else
        {
            assert(src->is_pointer_type());
            assert(src->get_pointer_element_type()->is_function_type());
            res = dynamic_cast<FunctionType *>(src->get_pointer_element_type());
        }
        assert(res);
        return res;
    }

    virtual Type *get_print_type() override
    {
        decltype(args_) args;
        auto result = result_;
        if (result_->is_struct_type())
        {
            args.push_back(PointerType::get(result_));
            result = Type::get_void_type(get_module());
        }
        for (auto arg_ : args_)
        {
            if (arg_->is_struct_type())
                args.push_back(PointerType::get(arg_));
            else
                args.push_back(arg_);
        }
        return get(result, args, get_module());
    }

    Type *get_param_type(unsigned i) const;
    std::vector<Type *>::iterator param_begin() { return args_.begin(); }
    std::vector<Type *>::iterator param_end() { return args_.end(); }
    const std::vector<Type *> &get_params() { return args_; }
    Type *get_return_type() const;

private:
    Type *result_;
    std::vector<Type *> args_;
};

class StructType : public Type
{
public:
    struct StructMember
    {
        Type *type;
        std::string member_id;
    };

    StructType(std::string struct_id, std::vector<StructMember> members, Module *m);

    static StructType *get(std::string struct_id, Module *m);
    int get_element_index(std::string member_id)
    {
        for (size_t i = 0; i < members.size(); i++)
        {
            if (members[i].member_id == member_id)
                return i;
        }
        return -1;
    }
    auto &get_members() const { return members; }
    std::string get_id() const
    {
        return struct_id;
    }
    std::string print() override
    {
        return "%struct." + struct_id;
    }

    std::string print_definition()
    {
        std::string res;
        res += "%struct." + struct_id;
        res += " = type {";
        if (!members.empty())
        {
            res += members.front().type->print();
            for (auto iter = members.begin() + 1; iter != members.end(); iter++)
            {
                res += ", ";
                res += iter->type->print();
            }
        }
        res += "}\n";
        return res;
    }

private:
    std::string struct_id;
    std::vector<StructMember> members;
};

class ArrayType : public Type
{
public:
    ArrayType(Type *contained, unsigned num_elements);

    static bool is_valid_element_type(Type *ty);

    static ArrayType *get(Type *contained, unsigned num_elements);

    Type *get_element_type() const { return contained_; }
    unsigned get_num_of_elements() const { return num_elements_; }

    virtual Type *get_print_type() override
    {
        return ArrayType::get(contained_->get_print_type(), num_elements_);
    }

private:
    Type *contained_;       // The element type of the array.
    unsigned num_elements_; // Number of elements in the array.
};

class ReferenceType : public Type
{
public:
    PointerType *type;
    ReferenceType(PointerType *type) : type(type), Type(Type::ReferenceTyID, type->get_module()) {}
    static ReferenceType *get_reference_type(Type *contained)
    {
        // TODO Memory management
        return new ReferenceType(PointerType::get(contained));
    }
};

class FloatType : public Type
{
public:
    FloatType(Module *m);
    static FloatType *get(Module *m);

private:
};

#endif // SYSYC_TYPE_H