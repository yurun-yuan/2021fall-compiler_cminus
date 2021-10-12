define dso_local i32 @main() #0{
    %a_ptr = alloca i32
    %i_ptr = alloca i32
    store i32 10, i32* %a_ptr
    store i32 0, i32* %i_ptr
    br label %loop_entry

loop_entry:
    %i_val1 = load i32, i32* %i_ptr
    %comp_res = icmp slt i32 %i_val1, 10
    br i1 %comp_res, label %loop_cont, label %loop_fin

loop_cont:
    %iadd = add i32 %i_val1, 1
    store i32 %iadd, i32* %i_ptr
    %a_val1 = load i32, i32* %a_ptr
    %i_val2 = load i32, i32* %i_ptr
    %aadd = add i32 %a_val1, %i_val2
    store i32 %aadd, i32* %a_ptr
    br label %loop_entry
    
loop_fin:
    %a_val3 = load i32, i32* %a_ptr
    ret i32 %a_val3
}