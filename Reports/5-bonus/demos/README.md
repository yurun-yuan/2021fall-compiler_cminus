# Demo: Complex Number Calculator

This program takes 2 complex number and an operator as input, then prints the result. 

## Compile

```bash
cminusfc complex_num_cal.cminus -o complex_num_cal
```

## Run

```bash
$ ./complex_num_cal
```

Enter an expression consisting of numbers, `i`, arithmetic operators, parentheses and white spaces ended with a character `$`. 
Hit enter and check if the result is correct. 

e.g. 

```bash
$ ./complex_num_cal
5 + (1 + 2i) * (3 + 4i)$
0.000000 10.000000i
```

The instance shown above means

$$
5+(1+2i)(3+4i)=10i
$$