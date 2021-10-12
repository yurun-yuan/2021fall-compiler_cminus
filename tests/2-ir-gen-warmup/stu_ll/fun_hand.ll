define dso_local i32 @callee(i32 %a) #0 {
    %mul_res = mul i32 %a, 2
    ret i32 %mul_res
}

define dso_local i32 @main() #0{
    %ret_val = call i32 @callee(i32 110)
    ret i32 %ret_val
}