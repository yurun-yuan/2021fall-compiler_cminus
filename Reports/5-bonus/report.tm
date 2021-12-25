<TeXmacs|2.1>

<style|<tuple|generic|chinese>>

<\body>
  <doc-data|<doc-title|Compilers Priciple Lab5: <em|Language Features>
  Report>|<doc-author|<author-data|<author-name|\<#8881\>\<#7389\>\<#6DA6\>
  PB19111692>>>>

  <abstract-data|<\abstract>
    \<#57FA\>\<#4E8E\>Lab1-Lab4\<#FF0C\>\<#6211\>\<#5728\>\<#672C\>\<#6B21\>\<#5B9E\>\<#9A8C\>\<#4E2D\>\<#589E\>\<#52A0\>\<#4E86\><verbatim|cminusf>\<#7684\>\<#8BED\>\<#8A00\>\<#7279\>\<#6027\>\<#FF0C\>\<#4E3B\>\<#8981\>\<#5305\>\<#62EC\>\<#FF1A\>

    <\enumerate>
      <item>\<#7C7B\>\<#578B\>\<#62D3\>\<#5C55\>

      \<#589E\>\<#52A0\>\<#4E86\>\<#4E30\>\<#5BCC\>\<#7684\>\<#6307\>\<#9488\>\<#3001\>\<#6570\>\<#7EC4\>\<#7C7B\>\<#578B\>\<#FF0C\>\<#652F\>\<#6301\>\<#7ED3\>\<#6784\>\<#4F53\>\<#3002\>

      <item>\<#8FD0\>\<#7B97\>\<#7B26\>\<#62D3\>\<#5C55\>

      \<#9664\>\<#57FA\>\<#672C\>\<#7684\>\<#7B97\>\<#6570\>\<#8FD0\>\<#7B97\>\<#548C\>\<#5173\>\<#7CFB\>\<#8FD0\>\<#7B97\>\<#5916\>\<#FF0C\>\<#589E\>\<#52A0\>\<#4E0B\>\<#6807\>\<#8FD0\>\<#7B97\>\<#3001\>\<#8C03\>\<#7528\>\<#8FD0\>\<#7B97\>\<#3001\>\<#53D6\>\<#5730\>\<#5740\>\<#8FD0\>\<#7B97\>\<#3001\>\<#89E3\>\<#5F15\>\<#8FD0\>\<#7B97\>\<#3001\>\<#6210\>\<#5458\>\<#8BBF\>\<#95EE\>\<#8FD0\>\<#7B97\>\<#3002\>

      <item>\<#7C7B\>

      \<#652F\>\<#6301\>\<#90E8\>\<#5206\>\<#7C7B\>\<#7684\>\<#7279\>\<#6027\>\<#FF0C\>\<#5305\>\<#62EC\>\<#6210\>\<#5458\>\<#51FD\>\<#6570\>\<#548C\>\<#7B97\>\<#6570\>\<#8FD0\>\<#7B97\>\<#7B26\>\<#91CD\>\<#8F7D\>\<#3002\>
    </enumerate>

    \<#5728\>\<#672C\>\<#62A5\>\<#544A\>\<#4E2D\>\<#FF0C\>\<#6211\>\<#5C06\>\<#5C55\>\<#793A\>\<#4E0A\>\<#8FF0\>\<#5185\>\<#5BB9\>\<#7684\>\<#5B9E\>\<#73B0\>\<#65B9\>\<#6CD5\>\<#548C\>\<#5B9E\>\<#73B0\>\<#6548\>\<#679C\>\<#3002\>
  </abstract>>

  <section|\<#65B0\>\<#589E\>\<#8BED\>\<#8A00\>\<#7279\>\<#6027\>\<#53CA\>\<#5B9E\>\<#73B0\>\<#65B9\>\<#6CD5\>>

  <subsection|\<#7C7B\>\<#578B\>\<#62D3\>\<#5C55\>>

  <subsubsection|\<#6307\>\<#9488\>\<#4E0E\>\<#6570\>\<#7EC4\>>

  <\definition*>
    Declaration grammar

    <\eqnarray*>
      <tformat|<table|<row|<cell|<text|var-declaration>>|<cell|\<rightarrow\>>|<cell|<text|type-specifier
      declarator ;>>>>>
    </eqnarray*>
  </definition*>

  <\example*>
    <cpp|int *a;>\<#4E2D\> <verbatim|int> \<#4E3A\> type-specifier,
    <verbatim|*a> \<#4E3A\>declarator.\ 
  </example*>

  <\enumerate>
    <item>type specifier

    <\eqnarray*>
      <tformat|<table|<row|<cell|<text|type-specifier>>|<cell|\<rightarrow\>>|<cell|<text|<with|font-series|bold|int>
      \| <with|font-series|bold|float>>>>>>
    </eqnarray*>

    <item>declarator

    Declarator \<#5206\>\<#4E3A\>\<#4EE5\>\<#4E0B\>3\<#79CD\>\<#FF1A\>

    <\enumerate>
      <item>Pointer declarator

      <cpp|int *a;> \<#4E2D\> <verbatim|*a> \<#5373\>pointer declarator.\ 

      <item>Function declarator

      <cpp|int (*p)(int);> \<#4E2D\> <verbatim|(*p)(int)> \<#5373\> function
      declarator.\ 

      <item>Array declarator

      <verbatim|int a[32];> \<#4E2D\> <verbatim|a[32]> \<#5373\> array
      declarator.\ 
    </enumerate>

    \<#5404\>\<#79CD\>declarator\<#95F4\>\<#53EF\>\<#4EE5\>\<#5D4C\>\<#5957\>\<#FF0C\>\<#5F97\>\<#5230\>\<#66F4\>\<#52A0\>\<#4E30\>\<#5BCC\>\<#7684\>\<#7C7B\>\<#578B\>\<#FF1A\>

    <\eqnarray*>
      <tformat|<table|<row|<cell|<text|declarator>>|<cell|\<rightarrow\>>|<cell|<text|*
      declarator>>>|<row|<cell|>|<cell|\|>|<cell|<text|declarator ( params
      )>>>|<row|<cell|>|<cell|\|>|<cell|<text|declarator [
      <with|font-series|bold|int> ]>>>|<row|<cell|>|<cell|\|>|<cell|<text|(
      declarator )>>>>>
    </eqnarray*>

    <\bothlined>
      Problem: Ambiguity

      <\cpp-code>
        int *ambiguous[42]

        \;

        int (*ptr)[42];

        int*(array[42]);
      </cpp-code>

      Solution: Precedence

      \<#6839\>\<#636E\><verbatim|*>\<#548C\><verbatim|[]>\<#7684\>\<#8FD0\>\<#7B97\>\<#4F18\>\<#5148\>\<#7EA7\>\<#4FEE\>\<#6539\>CFG\<#FF0C\>\<#6D88\>\<#9664\>\<#4E8C\>\<#4E49\>\<#6027\>\<#3002\>

      <\big-table*|<wide-tabular|<tformat|<cwith|1|1|1|-1|font-series|bold>|<cwith|1|-1|1|-1|cell-tborder|1ln>|<cwith|1|-1|1|-1|cell-bborder|1ln>|<cwith|1|-1|1|-1|cell-lborder|1ln>|<cwith|1|-1|1|-1|cell-rborder|1ln>|<cwith|2|-1|1|-1|cell-valign|c>|<cwith|2|-1|1|1|cell-halign|c>|<table|<row|<\cell>
        Precedence
      </cell>|<\cell>
        Description
      </cell>|<\cell>
        Context Free Grammar
      </cell>>|<row|<\cell>
        0
      </cell>|<\cell>
        \;
      </cell>|<\cell>
        <\eqnarray*>
          <tformat|<table|<row|<cell|<text|factor>>|<cell|\<rightarrow\>>|<cell|<text|<with|font-series|bold|ID>>>>|<row|<cell|>|<cell|\|>|<cell|<text|(
          declarator )>>>>>
        </eqnarray*>
      </cell>>|<row|<\cell>
        1
      </cell>|<\cell>
        Function declarator

        Array declarator
      </cell>|<\cell>
        <\eqnarray*>
          <tformat|<table|<row|<cell|<text|declarator-1>>|<cell|\<rightarrow\>>|<cell|<text|declarator-1
          ( params )>>>|<row|<cell|>|<cell|\|>|<cell|<text|declarator-1 [
          <with|font-series|bold|int> ]>>>|<row|<cell|>|<cell|\|>|<cell|<text|factor>>>>>
        </eqnarray*>
      </cell>>|<row|<\cell>
        2
      </cell>|<\cell>
        Pointer declarator
      </cell>|<\cell>
        <\eqnarray*>
          <tformat|<table|<row|<cell|<text|declarator>>|<cell|\<rightarrow\>>|<cell|<text|<with|font-series|bold|*>
          declarator>>>|<row|<cell|>|<cell|\|>|<cell|<text|declarator-1>>>>>
        </eqnarray*>
      </cell>>>>>>
        Declarator CFG
      </big-table*>
    </bothlined>
  </enumerate>

  <\example*>
    An array of function pointers.\ 

    <\verbatim-code>
      int (* func_table[2])(int, int)
    </verbatim-code>

    <\wide-tabular>
      <tformat|<cwith|1|-1|1|-1|cell-tborder|1ln>|<cwith|1|-1|1|-1|cell-bborder|1ln>|<cwith|1|-1|1|-1|cell-lborder|0ln>|<cwith|1|-1|1|-1|cell-rborder|0ln>|<table|<row|<\cell>
        type-specifer
      </cell>|<\cell>
        <verbatim|int>
      </cell>>|<row|<\cell>
        declarator
      </cell>|<\cell>
        <verbatim|(* func_table[2])(int, int)>
      </cell>>>>
    </wide-tabular>

    \<#5404\>\<#5C42\>Declarators\<#5982\>\<#4E0B\>\<#FF1A\>

    <\wide-tabular>
      <tformat|<cwith|1|-1|1|-1|cell-tborder|1ln>|<cwith|1|-1|1|-1|cell-bborder|1ln>|<cwith|1|-1|1|-1|cell-lborder|0ln>|<cwith|1|-1|1|-1|cell-rborder|0ln>|<table|<row|<\cell>
        <with|font-series|bold|Declarators>
      </cell>|<\cell>
        <with|font-series|bold|Type>
      </cell>>|<row|<\cell>
        <verbatim|(* func_table[2])(int, int)>
      </cell>|<\cell>
        <verbatim|int>
      </cell>>|<row|<\cell>
        <verbatim|(* func_table[2])>
      </cell>|<\cell>
        A function who takes 2 <verbatim|int>s as parameters and returns an
        <verbatim|int>
      </cell>>|<row|<\cell>
        <verbatim|func_table[2]>
      </cell>|<\cell>
        A function pointer
      </cell>>|<row|<\cell>
        <verbatim|func_table>
      </cell>|<\cell>
        An array of function pointers
      </cell>>>>
    </wide-tabular>
  </example*>

  <subsubsection|\<#7ED3\>\<#6784\>\<#4F53\>>

  <\example*>
    \<#7ED3\>\<#6784\>\<#4F53\>\<#53CA\>\<#7ED3\>\<#6784\>\<#4F53\>\<#7C7B\>\<#578B\>\<#53D8\>\<#91CF\>\<#5B9A\>\<#4E49\>\<#8BED\>\<#6CD5\>\<#7684\>\<#591A\>\<#6837\>\<#6027\>\<#FF0C\>\<#4F7F\>\<#5F97\>\<#7ED3\>\<#6784\>\<#4F53\>\<#7684\>\<#5B9E\>\<#73B0\>\<#770B\>\<#8D77\>\<#6765\>\<#5341\>\<#5206\>\<#56F0\>\<#96BE\>\<#3002\>

    <\cpp-code>
      // Struct definitions with no variables declared

      struct S

      {

      \ \ \ \ ...

      };

      \;

      // Struct definitions with a variable declared

      struct T

      {

      \ \ \ \ ...

      } t;

      \;

      // Define a variable with struct type

      struct S s;

      \;

      // Anonymous structs

      struct

      {

      \ \ \ \ ...

      } anonymous;

      \;

      // Nested structs

      struct U

      {

      \ \ \ \ struct Nested

      \ \ \ \ {

      \ \ \ \ \ \ \ \ ...

      \ \ \ \ } n;

      };
    </cpp-code>
  </example*>

  <\framed>
    \<#6839\>\<#636E\>\<#89C2\>\<#5BDF\>\<#548C\>\<#8C03\>\<#7814\>\<#FF0C\>\<#53EF\>\<#77E5\>\<#FF1A\>

    Struct definitions <em|are> type specifiers.\ 

    declarators may be omitted if the type-specifier is a struct definition.\ 

    <\example*>
      \;

      <\wide-tabular>
        <tformat|<cwith|1|-1|1|-1|cell-tborder|1ln>|<cwith|1|-1|1|-1|cell-bborder|1ln>|<cwith|1|-1|1|-1|cell-lborder|1ln>|<cwith|1|-1|1|-1|cell-rborder|1ln>|<table|<row|<\cell>
          <with|font-series|bold|Description>
        </cell>|<\cell>
          \<#6574\>\<#6570\>
        </cell>|<\cell>
          \<#7ED3\>\<#6784\>\<#4F53\>\<#5B9A\>\<#4E49\>+\<#53D8\>\<#91CF\>\<#58F0\>\<#660E\>
        </cell>|<\cell>
          \<#7ED3\>\<#6784\>\<#4F53\>\<#5B9A\>\<#4E49\>
        </cell>|<\cell>
          \<#53D8\>\<#91CF\>\<#58F0\>\<#660E\>
        </cell>>|<row|<\cell>
          <with|font-series|bold|Declaration>
        </cell>|<\cell>
          <verbatim|int a;>
        </cell>|<\cell>
          <verbatim|struct S{<text-dots>} s;>
        </cell>|<\cell>
          <verbatim|struct S{<text-dots>};>
        </cell>|<\cell>
          <verbatim|struct S s;>
        </cell>>|<row|<\cell>
          <with|font-series|bold|Type specifier>
        </cell>|<\cell>
          <verbatim|int>
        </cell>|<\cell>
          <verbatim|struct S{<text-dots>}>
        </cell>|<\cell>
          <verbatim|struct S{<text-dots>}>
        </cell>|<\cell>
          <verbatim|struct S>
        </cell>>|<row|<\cell>
          <with|font-series|bold|Declarator>
        </cell>|<\cell>
          <verbatim|a>
        </cell>|<\cell>
          <verbatim|s>
        </cell>|<\cell>
          <em|empty>
        </cell>|<\cell>
          <verbatim|s>
        </cell>>>>
      </wide-tabular>
    </example*>
  </framed>

  \<#4FEE\>\<#6539\>\<#6587\>\<#6CD5\>\<#5982\>\<#4E0B\>\<#FF1A\>

  <\eqnarray*>
    <tformat|<table|<row|<cell|<text|type-specifier>>|<cell|\<rightarrow\>>|<cell|<text|<with|font-series|bold|int>
    \| <with|font-series|bold|float>>>>|<row|<cell|>|<cell|\|>|<cell|<text|<marked|struct-definition>>>>|<row|<cell|<marked|<text|struct-definition>>>|<cell|\<rightarrow\>>|<cell|<text|<with|font-series|bold|struct>
    <with|font-series|bold|ID> { definitions
    }>>>|<row|<cell|>|<cell|\|>|<cell|<text|<with|font-series|bold|struct> {
    definitions }>>>|<row|<cell|>|<cell|\|>|<cell|<text|<with|font-series|bold|struct>
    <with|font-series|bold|ID>>>>|<row|<cell|<text|declarator>>|<cell|\<rightarrow\>>|<cell|\<ldots\>>>|<row|<cell|>|<cell|\|>|<cell|<marked|\<varepsilon\>>>>>>
  </eqnarray*>

  <subsection|\<#8FD0\>\<#7B97\>\<#62D3\>\<#5C55\>>

  \<#652F\>\<#6301\>\<#5982\>\<#4E0B\>\<#8FD0\>\<#7B97\>\<#FF1A\>

  <\enumerate>
    <item>Arithmatic operations

    <item>Relational operations

    <item>Assignment

    <item>Array subscript

    <verbatim|array[subscript]>

    <item>Pointer dereference

    <verbatim|*ptr>

    <item>Address of

    <verbatim|&rval>

    <item>Member access

    <verbatim|inst.mem>

    <item>Function call

    <verbatim|callable(params)>
  </enumerate>

  <subsubsection|\<#8868\>\<#8FBE\>\<#5F0F\>CFG>

  \<#6839\>\<#636E\>\<#8FD0\>\<#7B97\>\<#4F18\>\<#5148\>\<#7EA7\>\<#6D88\>\<#9664\>\<#4E8C\>\<#4E49\>\<#6027\>\<#FF0C\>\<#5F97\>\<#5230\>\<#4EE5\>\<#4E0B\>\<#6587\>\<#6CD5\>\<#FF1A\>

  <\big-table*|<wide-tabular|<tformat|<cwith|1|1|1|-1|font-series|bold>|<cwith|1|-1|1|-1|cell-tborder|1ln>|<cwith|1|-1|1|-1|cell-bborder|1ln>|<cwith|1|-1|1|-1|cell-lborder|1ln>|<cwith|1|-1|1|-1|cell-rborder|1ln>|<cwith|2|-1|1|-1|cell-valign|c>|<cwith|2|-1|1|1|cell-halign|c>|<table|<row|<\cell>
    Precedence
  </cell>|<\cell>
    Operator
  </cell>|<\cell>
    Context Free Grammar
  </cell>>|<row|<\cell>
    0
  </cell>|<\cell>
    \;
  </cell>|<\cell>
    <\eqnarray*>
      <tformat|<table|<row|<cell|<text|expression-0>>|<cell|\<rightarrow\>>|<cell|<text|<with|font-series|bold|ID>>>>|<row|<cell|>|<cell|\|>|<cell|<text|<with|font-series|bold|Integer-literal>><space|8em>>>|<row|<cell|>|<cell|\|>|<cell|<text|<with|font-series|bold|Float-literal>>>>|<row|<cell|>|<cell|\|>|<cell|<text|(
      expression )>>>>>
    </eqnarray*>
  </cell>>|<row|<\cell>
    1
  </cell>|<\cell>
    Function call

    Array subscripting

    Member access
  </cell>|<\cell>
    <\eqnarray*>
      <tformat|<table|<row|<cell|<text|expression-1>>|<cell|\<rightarrow\>>|<cell|<text|expression-1
      ( args )>>>|<row|<cell|>|<cell|\|>|<cell|<text|expression-1 [
      expression ]><space|3em>>>|<row|<cell|>|<cell|\|>|<cell|<text|expression-1
      <with|font-series|bold|.> <with|font-series|bold|ID>>>>|<row|<cell|>|<cell|\|>|<cell|<text|expression-0>>>>>
    </eqnarray*>
  </cell>>|<row|<\cell>
    2
  </cell>|<\cell>
    Dereference

    Address of
  </cell>|<\cell>
    <\eqnarray*>
      <tformat|<table|<row|<cell|<text|expression-2>>|<cell|\<rightarrow\>>|<cell|<text|<with|font-series|bold|*>
      expression-2>>>|<row|<cell|>|<cell|\|>|<cell|<text|<with|font-series|bold|&>
      expression-2><space|8em>>>|<row|<cell|>|<cell|\|>|<cell|<text|expression-1>>>>>
    </eqnarray*>
  </cell>>|<row|<\cell>
    3
  </cell>|<\cell>
    Multiplication, division
  </cell>|<\cell>
    <\eqnarray*>
      <tformat|<table|<row|<cell|<text|expression-3>>|<cell|\<rightarrow\>>|<cell|<text|expression-3
      <with|font-series|bold|MulOp> expression-2>>>|<row|<cell|>|<cell|\|>|<cell|<text|expression-2>>>>>
    </eqnarray*>
  </cell>>|<row|<\cell>
    4
  </cell>|<\cell>
    Addition, subtraction
  </cell>|<\cell>
    <\eqnarray*>
      <tformat|<table|<row|<cell|<text|expression-4>>|<cell|\<rightarrow\>>|<cell|<text|expression-4
      <with|font-series|bold|AddOp> expression-3>>>|<row|<cell|>|<cell|\|>|<cell|<text|expression-3>>>>>
    </eqnarray*>
  </cell>>|<row|<\cell>
    5
  </cell>|<\cell>
    Relational operators
  </cell>|<\cell>
    <\eqnarray*>
      <tformat|<table|<row|<cell|<text|expression-5>>|<cell|\<rightarrow\>>|<cell|<text|expression-5
      <with|font-series|bold|RelOp> expression-4>
      >>|<row|<cell|>|<cell|\|>|<cell|<text|expression-4>>>>>
    </eqnarray*>
  </cell>>|<row|<\cell>
    6
  </cell>|<\cell>
    Assignment
  </cell>|<\cell>
    <\eqnarray*>
      <tformat|<table|<row|<cell|<text|expression>>|<cell|\<rightarrow\>>|<cell|<text|expression-5
      <with|font-series|bold|Assign> expression>
      >>|<row|<cell|>|<cell|\|>|<cell|<text|expression-5>>>>>
    </eqnarray*>
  </cell>>>>>>
    Expression CFG
  </big-table*>

  <subsubsection|\<#7ED3\>\<#6784\>\<#4F53\>\<#76F8\>\<#5173\>\<#7684\>\<#8FD0\>\<#7B97\>>

  \<#652F\>\<#6301\>\<#7ED3\>\<#6784\>\<#4F53\>\<#8FD9\>\<#6837\>\<#7684\>\<#590D\>\<#5408\>\<#7C7B\>\<#578B\>\<#540E\>\<#FF0C\>\<#4E00\>\<#4E9B\>\<#5BF9\>scalar
  type\<#7684\>\<#8FD0\>\<#7B97\>\<#9700\>\<#8981\>\<#62D3\>\<#5C55\>\<#FF0C\>\<#4EE5\>\<#652F\>\<#6301\>\<#7ED3\>\<#6784\>\<#4F53\>\<#7C7B\>\<#578B\>\<#7684\>\<#8FD0\>\<#7B97\>\<#5143\>\<#3002\>\<#6700\>\<#663E\>\<#8457\>\<#7684\>\<#53D8\>\<#5316\>\<#4FBF\>\<#662F\>\<#FF0C\>\<#7531\>\<#4E8E\>\<#8BB8\>\<#591A\>\<#7ED3\>\<#6784\>\<#4F53\>\<#7C7B\>\<#578B\>\<#7684\>\<#53D8\>\<#91CF\>\<#65E0\>\<#6CD5\>\<#50A8\>\<#5B58\>\<#5728\>\<#5355\>\<#4E00\>\<#5BC4\>\<#5B58\>\<#5668\>\<#4E2D\>\<#FF0C\>\<#7ED3\>\<#6784\>\<#4F53\>\<#7684\>\<#8D4B\>\<#503C\>\<#548C\>\<#4F20\>\<#9012\>\<#9700\>\<#8981\>\<#7279\>\<#522B\>\<#7684\>\<#64CD\>\<#4F5C\>\<#3002\>

  <paragraph|Assignment to struct-type variables>\<#901A\>\<#8FC7\>\<#5BF9\>\<#6210\>\<#5458\>\<#9010\>\<#4E2A\>\<#8D4B\>\<#503C\>\<#7684\>\<#65B9\>\<#5F0F\>\<#5B9E\>\<#73B0\>\<#4E86\>\<#7ED3\>\<#6784\>\<#4F53\>\<#53D8\>\<#91CF\>\<#7684\>\<#9ED8\>\<#8BA4\>\<#8D4B\>\<#503C\>\<#64CD\>\<#4F5C\>\<#3002\>

  <\example*>
    \<#8003\>\<#8651\>\<#5982\>\<#4E0B\>\<#5B9A\>\<#4E49\>\<#7684\>\<#7ED3\>\<#6784\>\<#4F53\><verbatim|S>:

    <\cpp-code>
      struct S

      {

      \ \ \ \ int member1;

      \ \ \ \ float member2;

      };
    </cpp-code>

    \<#5BF9\><verbatim|struct S>\<#7C7B\>\<#578B\>\<#7684\>\<#53D8\>\<#91CF\><verbatim|s>\<#7684\>\<#8D4B\>\<#503C\>\<#64CD\>\<#4F5C\>\<#5C06\>\<#4F1A\>\<#88AB\>\<#7FFB\>\<#8BD1\>\<#4E3A\>\<#FF1A\>

    <\wide-tabular>
      <tformat|<cwith|1|-1|1|-1|cell-tborder|1ln>|<cwith|1|-1|1|-1|cell-bborder|1ln>|<cwith|1|-1|1|-1|cell-lborder|1ln>|<cwith|1|-1|1|-1|cell-rborder|1ln>|<cwith|2|2|1|1|cell-valign|c>|<table|<row|<\cell>
        <verbatim|cminusf>
      </cell>|<\cell>
        Translated represent
      </cell>>|<row|<\cell>
        <\verbatim-code>
          s=t
        </verbatim-code>
      </cell>|<\cell>
        <\cpp-code>
          s.member1 = t.member1;

          s.member2 = t.member2;
        </cpp-code>
      </cell>>>>
    </wide-tabular>
  </example*>

  <paragraph|Struct-type parameters>\<#5F53\>\<#51FD\>\<#6570\>\<#8C03\>\<#7528\>\<#65F6\>\<#53C2\>\<#6570\>\<#4E3A\>\<#7ED3\>\<#6784\>\<#4F53\>\<#7C7B\>\<#578B\>\<#65F6\>\<#FF0C\>\<#4F20\>\<#9012\>\<#7ED3\>\<#6784\>\<#4F53\>\<#53D8\>\<#91CF\>\<#7684\>\<#6307\>\<#9488\>\<#FF0C\>\<#5E76\>\<#7531\>callee\<#590D\>\<#5236\>\<#6307\>\<#9488\>\<#6307\>\<#5411\>\<#7684\>\<#7ED3\>\<#6784\>\<#4F53\>\<#53D8\>\<#91CF\>\<#3002\>

  <\example*>
    \<#8003\>\<#8651\>\<#5982\>\<#4E0B\>\<#60C5\>\<#5F62\>

    <\wide-tabular>
      <tformat|<cwith|1|-1|1|-1|cell-tborder|1ln>|<cwith|1|-1|1|-1|cell-bborder|1ln>|<cwith|1|-1|1|-1|cell-lborder|1ln>|<cwith|1|-1|1|-1|cell-rborder|1ln>|<table|<row|<\cell>
        <verbatim|cminusf>
      </cell>|<\cell>
        Translated represent
      </cell>>|<row|<\cell>
        <\verbatim-code>
          void func(struct S s)

          {

          \ \ \ \ <text-dots>

          }

          \;

          int main()

          {

          \ \ \ func(t);

          \ \ \ <text-dots>

          }
        </verbatim-code>
      </cell>|<\cell>
        <\verbatim-code>
          void func(struct S *ptr)

          {

          \ \ \ \ struct S s;

          \ \ \ \ s = *ptr;

          \ \ \ \ <text-dots>

          }

          \;

          int main()

          {

          \ \ \ \ func(&t);

          }
        </verbatim-code>
      </cell>>>>
    </wide-tabular>
  </example*>

  <paragraph|Struct-type return value>\<#5F53\>\<#51FD\>\<#6570\>\<#8FD4\>\<#56DE\>\<#503C\>\<#4E3A\>\<#7ED3\>\<#6784\>\<#4F53\>\<#7C7B\>\<#578B\>\<#65F6\>\<#FF0C\>\<#7531\>caller\<#5F00\>\<#8F9F\>\<#8FD4\>\<#56DE\>\<#503C\>\<#5B58\>\<#653E\>\<#7A7A\>\<#95F4\>\<#FF0C\>\<#5E76\>\<#5C06\>\<#6307\>\<#5411\>\<#8FD9\>\<#5757\>\<#7A7A\>\<#95F4\>\<#7684\>\<#6307\>\<#9488\>\<#4F5C\>\<#4E3A\>\<#53C2\>\<#6570\>\<#4F20\>\<#9012\>\<#81F3\>callee

  <\example*>
    \<#8003\>\<#8651\>\<#5982\>\<#4E0B\>\<#60C5\>\<#5F62\>

    <\wide-tabular>
      <tformat|<cwith|1|-1|1|-1|cell-tborder|1ln>|<cwith|1|-1|1|-1|cell-bborder|1ln>|<cwith|1|-1|1|-1|cell-lborder|1ln>|<cwith|1|-1|1|-1|cell-rborder|1ln>|<table|<row|<\cell>
        <verbatim|cminusf>
      </cell>|<\cell>
        Translated represent
      </cell>>|<row|<\cell>
        <\verbatim-code>
          struct S func()

          {

          \ \ \ \ <text-dots>

          \ \ \ \ return s;

          }

          \;

          int main()

          {

          \ \ \ func();

          }
        </verbatim-code>
      </cell>|<\cell>
        <\verbatim-code>
          void func(struct S* ret_ptr)

          {

          \ \ \ \ <text-dots>

          \ \ \ \ *ret_ptr = ret_value;

          }

          \;

          int main()

          {

          \ \ \ \ struct S ret_value;

          \ \ \ \ func(&ret_value);

          }
        </verbatim-code>
      </cell>>>>
    </wide-tabular>
  </example*>

  <subsection|\<#90E8\>\<#5206\>\<#7C7B\>\<#7684\>\<#7279\>\<#6027\>>

  <subsubsection|Member functions (Non-static)>

  \<#5728\>\<#53C2\>\<#6570\>\<#5217\>\<#8868\>\<#9996\>\<#4F4D\>\<#6DFB\>\<#52A0\><verbatim|this>\<#6307\>\<#9488\>\<#53C2\>\<#6570\>\<#FF0C\>\<#5C06\>\<#6210\>\<#5458\>\<#51FD\>\<#6570\>\<#7FFB\>\<#8BD1\>\<#4E3A\>\<#666E\>\<#901A\>\<#51FD\>\<#6570\>\<#3002\>

  <\example*>
    \;

    <\wide-tabular>
      <tformat|<cwith|1|-1|1|-1|cell-tborder|1ln>|<cwith|1|-1|1|-1|cell-bborder|1ln>|<cwith|1|-1|1|-1|cell-lborder|1ln>|<cwith|1|-1|1|-1|cell-rborder|1ln>|<table|<row|<\cell>
        <verbatim|cminusf>
      </cell>|<\cell>
        Translated represent
      </cell>>|<row|<\cell>
        <\cpp-code>
          struct S

          {

          \ \ \ \ ...

          \ \ \ \ void member_func(){}

          };

          \;

          int main()

          {

          \ \ \ \ struct S s;

          \ \ \ \ s.member_func();

          }
        </cpp-code>
      </cell>|<\cell>
        <\cpp-code>
          struct S

          {

          \ \ \ \ ...

          };

          \;

          void S.member_func(struct S* this){}

          \;

          int main()

          {

          \ \ \ \ struct S s;

          \ \ \ \ S.member_func(&s);

          }
        </cpp-code>
      </cell>>>>
    </wide-tabular>
  </example*>

  <\framed>
    Problem: \<#82E5\><em|\<#6309\>\<#987A\>\<#5E8F\>>\<#5BF9\>\<#7ED3\>\<#6784\>\<#4F53\>\<#5B9A\>\<#4E49\>\<#4E2D\>\<#7684\>\<#6210\>\<#5458\>\<#53D8\>\<#91CF\>\<#548C\>\<#6210\>\<#5458\>\<#51FD\>\<#6570\>\<#7684\>\<#5B9A\>\<#4E49\>\<#904D\>\<#5386\>\<#FF0C\>\<#5219\>\<#5728\>\<#67D0\>\<#51FD\>\<#6570\>\<#5B9A\>\<#4E49\>\<#540E\>\<#7684\>\<#6210\>\<#5458\>\<#53D8\>\<#91CF\>\<#5BF9\>\<#6210\>\<#5458\>\<#51FD\>\<#6570\>\<#4E0D\>\<#53EF\>\<#89C1\>\<#3002\>

    <\example*>
      \;

      <\cpp-code>
        struct S

        {

        \ \ \ \ void mem_func(); // \<#6B64\>\<#5904\>\<#5E76\>\<#4E0D\>\<#77E5\>\<#9053\>struct
        S\<#6709\>\<#6210\>\<#5458\>a

        \ \ \ \ int a;

        }
      </cpp-code>
    </example*>

    Solution: \<#9996\>\<#5148\>\<#904D\>\<#5386\>\<#7ED3\>\<#6784\>\<#4F53\>\<#4E2D\>\<#6240\>\<#6709\>\<#5B9A\>\<#4E49\>\<#FF0C\>\<#5C06\>\<#6210\>\<#5458\>\<#51FD\>\<#6570\>\<#4E0E\>\<#6210\>\<#5458\>\<#53D8\>\<#91CF\>\<#5206\>\<#79BB\>\<#5F00\>\<#FF0C\>\<#5728\>\<#5F97\>\<#5230\>\<#5B8C\>\<#6574\>\<#7684\>\<#7ED3\>\<#6784\>\<#4F53\>\<#6210\>\<#5458\>\<#53D8\>\<#91CF\>\<#5B9A\>\<#4E49\>\<#540E\>\<#518D\>\<#5904\>\<#7406\>\<#6210\>\<#5458\>\<#51FD\>\<#6570\>\<#3002\>
  </framed>

  <subsubsection|Arithmetic Operator Overloading>

  \<#8BC6\>\<#522B\>\<#5173\>\<#952E\>\<#5B57\><verbatim|operator>\<#FF0C\>\<#5C06\>\<#91CD\>\<#8F7D\>\<#64CD\>\<#4F5C\>\<#7684\>\<#51FD\>\<#6570\>\<#8BB0\>\<#5F55\>\<#4E8E\>\<#8868\>\<#4E2D\>\<#FF0C\>\<#5728\>\<#9700\>\<#8981\>\<#65F6\>\<#8C03\>\<#7528\>\<#6B64\>\<#51FD\>\<#6570\>\<#3002\>

  <\example*>
    \;

    <\wide-tabular>
      <tformat|<cwith|1|-1|1|-1|cell-tborder|1ln>|<cwith|1|-1|1|-1|cell-bborder|1ln>|<cwith|1|-1|1|-1|cell-lborder|1ln>|<cwith|1|-1|1|-1|cell-rborder|1ln>|<table|<row|<\cell>
        <verbatim|cminusf>
      </cell>|<\cell>
        Translated represent
      </cell>>|<row|<\cell>
        <\cpp-code>
          struct S

          {

          \ \ \ \ ...

          \ \ \ \ // Overload operator `+'

          \ \ \ \ struct S operator+(struct S rhs)

          \ \ \ \ {...}

          };

          \;

          int main()

          {

          \ \ \ \ struct S s;

          \ \ \ \ struct S t;

          \ \ \ \ ...

          \ \ \ \ s + t; // Call `S::operator+'

          }
        </cpp-code>
      </cell>|<\cell>
        <\cpp-code>
          struct S{...};

          \;

          void S.operator+(

          \ \ \ \ struct S* ret_ptr,\ 

          \ \ \ \ struct S* this

          \ \ \ \ struct S* rhs_ptr)

          {

          \ \ \ \ struct S rhs;

          \ \ \ \ rhs = *rhs_ptr;

          \ \ \ \ ...

          \ \ \ \ *ret_ptr = ret_value;

          }

          \;

          int main()

          {

          \ \ \ \ struct S s;

          \ \ \ \ struct S t;

          \ \ \ \ ...

          \ \ \ \ struct S ret_value;

          \ \ \ \ S.operator+(&ret_value,

          \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ &s,

          \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ &t);

          }
        </cpp-code>
      </cell>>>>
    </wide-tabular>
  </example*>

  <section|Demos>

  I wrote a complex nubmer calculator to exhibit all the extended language
  features mentioned above. The program has following components:

  <\description>
    <item*|Struct <verbatim|complext_num>>A struct representing a complex
    number, which simply contains 2 <verbatim|float>s as real component and
    imaginary component.\ 

    This struct has several member functions and overloads 4 arithmetic
    operators.\ 

    <item*|Functions performing arithmetic operations for complex number>4
    functions that take 2 complex numbers as parameters and calculate plus,
    subtraction, mulpication and division respectively.\ 

    <item*|A function table containing arithmetic calculation functions>An
    array containing all 4 functions mentioned above.\ 

    <item*|<verbatim|main> function>Takes care of IO operations and invokes
    other functions to do the calculation.\ 
  </description>

  <section|\<#5B9E\>\<#73B0\>\<#65F6\>\<#7684\>\<#963B\>\<#788D\>>

  \<#5B9E\>\<#73B0\>\<#4E0A\>\<#8FF0\>\<#7279\>\<#6027\>\<#65F6\>\<#9047\>\<#5230\>\<#8BB8\>\<#591A\>\<#8017\>\<#65F6\>\<#8017\>\<#529B\>\<#7684\>\<#5DE5\>\<#4F5C\>\<#FF0C\>\<#4E3B\>\<#8981\>\<#7684\>\<#56F0\>\<#96BE\>\<#6765\>\<#81EA\>\<#4EE5\>\<#4E0B\>\<#51E0\>\<#70B9\>\<#FF1A\>

  <\enumerate>
    <item>Lab1-Lab3\<#4E2D\>\<#4EE3\>\<#7801\>\<#7684\>\<#5927\>\<#89C4\>\<#6A21\>\<#91CD\>\<#65B0\>\<#8BBE\>\<#8BA1\>\<#53CA\>\<#91CD\>\<#5199\>

    \<#56E0\>\<#62D3\>\<#5C55\>\<#70B9\>\<#8F83\>\<#591A\>\<#FF0C\>\<#539F\>\<#5148\>\<#7684\>\<#7ED3\>\<#6784\>\<#65E0\>\<#6CD5\>\<#652F\>\<#6301\>\<#589E\>\<#52A0\>\<#7684\>\<#8BED\>\<#8A00\>\<#7279\>\<#6027\>\<#FF0C\>\<#6545\>\<#5BF9\>\<#8BCD\>\<#6CD5\>\<#5206\>\<#6790\>\<#3001\>\<#8BED\>\<#6CD5\>\<#5206\>\<#6790\>\<#3001\>AST\<#7684\>\<#8BBE\>\<#8BA1\>\<#53CA\>\<#6784\>\<#5EFA\>\<#3001\>IR\<#7684\>\<#7FFB\>\<#8BD1\>\<#90E8\>\<#5206\>\<#5747\>\<#6709\>\<#8F83\>\<#5927\>\<#7684\>\<#5220\>\<#6539\>\<#3002\>

    <item>LightIR\<#5BF9\>\<#4E00\>\<#4E9B\>\<#7279\>\<#6027\>\<#652F\>\<#6301\>\<#7684\>\<#7F3A\>\<#5931\>

    \<#9700\>\<#81EA\>\<#884C\>\<#4FEE\>\<#6539\>LightIR\<#4F7F\>\<#4E4B\>\<#652F\>\<#6301\>\<#7ED3\>\<#6784\>\<#4F53\>\<#7C7B\>\<#578B\>\<#3002\>

    \<#6B64\>\<#5916\>\<#FF0C\>LightIR\<#4E2D\>\<#53EA\>\<#80FD\>\<#5BF9\>\<#51FD\>\<#6570\>\<#5B9E\>\<#65BD\>\<#8C03\>\<#7528\>\<#64CD\>\<#4F5C\>\<#FF0C\>\<#4F46\>\<#5728\>IR\<#4E2D\>\<#5141\>\<#8BB8\>\<#5BF9\>\<#51FD\>\<#6570\>\<#6307\>\<#9488\>\<#5B9E\>\<#65BD\>\<#8C03\>\<#7528\>\<#3002\>
  </enumerate>

  <section|Unsolved Problems>

  <subsection|Omitting Keyword <verbatim|struct> when refering to a Struct
  Type>

  \<#5728\><verbatim|C++>\<#4E2D\>\<#FF0C\>\<#6307\>\<#4EE3\>\<#7C7B\>\<#7C7B\>\<#578B\>\<#65F6\>\<#53EF\>\<#5FFD\>\<#7565\><verbatim|struct>\<#6216\><verbatim|class>\<#5173\>\<#952E\>\<#5B57\>\<#FF0C\>\<#5982\>

  <\verbatim-code>
    struct S{};

    int func()

    {

    \ \ \ \ S s;

    }
  </verbatim-code>

  \<#4F46\>\<#4F7F\>\<#7528\>CFG\<#4F3C\>\<#4E4E\>\<#5F88\>\<#96BE\>\<#533A\>\<#5206\>\<#5F00\>\<#4EE5\>\<#4E0B\>\<#4E24\>\<#79CD\>\<#60C5\>\<#51B5\>\<#FF1A\>

  <\wide-tabular>
    <tformat|<table|<row|<\cell>
      \<#53D8\>\<#91CF\>\<#58F0\>\<#660E\>
    </cell>|<\cell>
      \<#51FD\>\<#6570\>\<#8C03\>\<#7528\>
    </cell>>|<row|<\cell>
      <verbatim|S (s);>
    </cell>|<\cell>
      <verbatim|func(s);>
    </cell>>>>
  </wide-tabular>
</body>

<\initial>
  <\collection>
    <associate|page-medium|paper>
  </collection>
</initial>

<\references>
  <\collection>
    <associate|auto-1|<tuple|1|1>>
    <associate|auto-10|<tuple|1.2.2.1|4>>
    <associate|auto-11|<tuple|1.2.2.2|5>>
    <associate|auto-12|<tuple|1.2.2.3|5>>
    <associate|auto-13|<tuple|1.3|5>>
    <associate|auto-14|<tuple|1.3.1|5>>
    <associate|auto-15|<tuple|1.3.2|6>>
    <associate|auto-16|<tuple|2|6>>
    <associate|auto-17|<tuple|3|7>>
    <associate|auto-18|<tuple|4|7>>
    <associate|auto-19|<tuple|4.1|7>>
    <associate|auto-2|<tuple|1.1|1>>
    <associate|auto-3|<tuple|1.1.1|1>>
    <associate|auto-4|<tuple|c|2>>
    <associate|auto-5|<tuple|1.1.2|2>>
    <associate|auto-6|<tuple|1.2|3>>
    <associate|auto-7|<tuple|1.2.1|4>>
    <associate|auto-8|<tuple|1.2.1|4>>
    <associate|auto-9|<tuple|1.2.2|4>>
  </collection>
</references>

<\auxiliary>
  <\collection>
    <\associate|table>
      <\tuple|normal>
        Declarator CFG
      </tuple|<pageref|auto-4>>

      <\tuple|normal>
        Expression CFG
      </tuple|<pageref|auto-8>>
    </associate>
    <\associate|toc>
      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|1<space|2spc>\<#65B0\>\<#589E\>\<#8BED\>\<#8A00\>\<#7279\>\<#6027\>\<#53CA\>\<#5B9E\>\<#73B0\>\<#65B9\>\<#6CD5\>>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-1><vspace|0.5fn>

      <with|par-left|<quote|1tab>|1.1<space|2spc>\<#7C7B\>\<#578B\>\<#62D3\>\<#5C55\>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-2>>

      <with|par-left|<quote|2tab>|1.1.1<space|2spc>\<#6307\>\<#9488\>\<#4E0E\>\<#6570\>\<#7EC4\>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-3>>

      <with|par-left|<quote|2tab>|1.1.2<space|2spc>\<#7ED3\>\<#6784\>\<#4F53\>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-5>>

      <with|par-left|<quote|1tab>|1.2<space|2spc>\<#8FD0\>\<#7B97\>\<#62D3\>\<#5C55\>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-6>>

      <with|par-left|<quote|2tab>|1.2.1<space|2spc>\<#8868\>\<#8FBE\>\<#5F0F\>CFG
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-7>>

      <with|par-left|<quote|2tab>|1.2.2<space|2spc>\<#7ED3\>\<#6784\>\<#4F53\>\<#76F8\>\<#5173\>\<#7684\>\<#8FD0\>\<#7B97\>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-9>>

      <with|par-left|<quote|4tab>|Assignment to struct-type variables
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-10><vspace|0.15fn>>

      <with|par-left|<quote|4tab>|Struct-type parameters
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-11><vspace|0.15fn>>

      <with|par-left|<quote|4tab>|Struct-type return value
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-12><vspace|0.15fn>>

      <with|par-left|<quote|1tab>|1.3<space|2spc>\<#90E8\>\<#5206\>\<#7C7B\>\<#7684\>\<#7279\>\<#6027\>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-13>>

      <with|par-left|<quote|2tab>|1.3.1<space|2spc>Member functions
      (Non-static) <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-14>>

      <with|par-left|<quote|2tab>|1.3.2<space|2spc>Arithmetic Operator
      Overloading <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-15>>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|2<space|2spc>\<#5B9E\>\<#73B0\>\<#65F6\>\<#7684\>\<#963B\>\<#788D\>>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-16><vspace|0.5fn>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|3<space|2spc>Demos>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-17><vspace|0.5fn>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|4<space|2spc>Unsolved
      Problems> <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-18><vspace|0.5fn>

      <with|par-left|<quote|1tab>|4.1<space|2spc>Omitting Keyword
      <with|font-family|<quote|tt>|language|<quote|verbatim>|struct> when
      refering to a Struct Type <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-19>>
    </associate>
  </collection>
</auxiliary>