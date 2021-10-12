define dso_local i32 @main() #0{

    ; a is the address of a[10]
    %a = alloca [10 x i32]

    ; a0 is the address of a[0]
    %a0 = getelementptr [10 x i32], [10 x i32]* %a, i32 0, i32 0

    ; assign constant 10 to a[0]
    store i32 10, i32* %a0

    ; a0value is the value of a[0]
    %a0value = load i32, i32* %a0

    ; calculate a[0]*2
    %mul = mul i32 %a0value, 2

    ; a1 is the address of a[1]
    %a1 = getelementptr [10 x i32], [10 x i32]* %a, i32 0, i32 1

    ; assign the result of multiplication to a[1]
    store i32 %mul, i32* %a1

    ; a1value is the value of a[1]
    %a1value = load i32, i32* %a1

    ; return the value of a[1]
    ret i32 %a1value
}