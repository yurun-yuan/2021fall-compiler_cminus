<TeXmacs|2.1>

<style|generic>

<\body>
  <doc-data|<doc-title|Blueprint of Lab 5: <verbatim|cminusf> Extension>>

  <section|Language Feature Extensions>

  <\enumerate>
    <item>Basic types

    <\description>
      <item*|Pointers>

      <\description>
        <item*|Function pointers>
      </description>

      <item*|Arrays>

      <\description>
        <item*|Multi-dimension arrays>
      </description>

      <item*|References>
    </description>

    <item>Classes

    <\description>
      <item*|Structures>

      <\description>
        <item*|Embedded structures>

        <item*|Anonymous structures>
      </description>

      <item*|Member functions>

      <item*|Lambda expressions>
    </description>

    <item>Template

    <\description>
      <item*|Function templates>

      <\description>
        <item*|Template argument inference>
      </description>

      <item*|Class templates>

      <\description>
        <item*|Template arguments inference>
      </description>
    </description>
  </enumerate>

  <section|Workload>

  <subsection|<verbatim|LightIR>>

  <subsubsection|<verbatim|struct> Support>

  <subsection|<verbatim|cminusf> Syntax>

  <subsubsection|CFG>

  <paragraph|Program structure>

  <math|<tabular|<tformat|<table|<row|<cell|<text|program>>|<cell|\<rightarrow\>>|<cell|<text|definition-list>>>|<row|<cell|<text|definition-list>>|<cell|\<rightarrow\>>|<cell|<text|definition-list>
  <text|definition>\|<text|definition>>>|<row|<cell|<text|definition>>|<cell|\<rightarrow\>>|<cell|<text|var-definition>\|<text|fun-definition>>>>>>>

  <paragraph|Definitions and declarations>

  Used in

  <\enumerate>
    <item>Variable definition

    <item>Argument declarations

    <item>Return type
  </enumerate>

  <em|Attention>: ID is optional in this CFG, i.e, empty declarations are
  possible. Please determine whether ID is null during semantic analysis.\ 

  <\eqnarray*>
    <tformat|<table|<row|<cell|ref>|<cell|\<rightarrow\>>|<cell|ID>>|<row|<cell|>|<cell|>|<cell|&ID>>|<row|<cell|>|<cell|>|<cell|empty>>|<row|<cell|>|<cell|>|<cell|&>>|<row|<cell|decl
    atom>|<cell|\<rightarrow\>>|<cell|ref>>|<row|<cell|>|<cell|>|<cell|<around*|(|decl
    expression|)>>>|<row|<cell|decl element>|<cell|\<rightarrow\>>|<cell|decl
    element<around*|(|params|)>>>|<row|<cell|>|<cell|>|<cell|decl
    element<around*|[|INTEGER|]>>>|<row|<cell|>|<cell|>|<cell|decl
    element<around*|[||]>>>|<row|<cell|>|<cell|>|<cell|atom>>|<row|<cell|expression>|<cell|\<rightarrow\>>|<cell|\<ast\>expression>>|<row|<cell|>|<cell|>|<cell|decl
    element>>|<row|<cell|>|<cell|>|<cell|>>|<row|<cell|params>|<cell|\<rightarrow\>>|<cell|param
    list>>|<row|<cell|>|<cell|>|<cell|void>>|<row|<cell|>|<cell|>|<cell|empty>>|<row|<cell|param
    list>|<cell|\<rightarrow\>>|<cell|param
    list,varDecl>>|<row|<cell|>|<cell|>|<cell|varDecl>>|<row|<cell|funcDefinition>|<cell|\<rightarrow\>>|<cell|varDec
    compoundStmt>>|<row|<cell|>|<cell|>|<cell|>>|<row|<cell|TypeSpecifier>|<cell|\<rightarrow\>>|<cell|ScalarType>>|<row|<cell|>|<cell|>|<cell|StructSpecification>>|<row|<cell|>|<cell|>|<cell|ID>>|<row|<cell|>|<cell|>|<cell|struct
    ID>>|<row|<cell|Scalar Type>|<cell|\<rightarrow\>>|<cell|int>>|<row|<cell|>|<cell|>|<cell|float>>|<row|<cell|>|<cell|>|<cell|void>>|<row|<cell|>|<cell|>|<cell|>>|<row|<cell|StructSpecification>|<cell|\<rightarrow\>>|<cell|struct
    ID<around*|{|definitions|}>>>|<row|<cell|>|<cell|>|<cell|struct
    <around*|{|definitions|}>>>|<row|<cell|definitions>|<cell|\<rightarrow\>>|<cell|definitionList>>|<row|<cell|>|<cell|>|<cell|empty>>>>
  </eqnarray*>

  <paragraph|Statement>

  <math|<tabular|<tformat|<table|<row|<cell|<text|compound-stmt>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|{>|\<bar\>>
  <text|statements><wide*|<with|font-series|bold|}>|\<bar\>>>|<cell|>>|<row|<cell|statements>|<cell|\<rightarrow\>>|<cell|statements
  statement>|<cell|>>|<row|<cell|>|<cell|>|<cell|empty>|<cell|>>|<row|<cell|<text|statement>>|<cell|\<rightarrow\>>|<cell|expression-stmt>|<cell|>>|<row|<cell|>|<cell|\|>|<cell|<text|compound-stmt>>|<cell|>>|<row|<cell|>|<cell|\|>|<cell|<text|selection-stmt>>|<cell|>>|<row|<cell|>|<cell|\|>|<cell|<text|iteration-stmt>>|<cell|>>|<row|<cell|>|<cell|\|>|<cell|<text|return-stmt>>|<cell|>>|<row|<cell|>|<cell|>|<cell|varDefinition>|<cell|>>|<row|<cell|varDec>|<cell|\<rightarrow\>>|<cell|TypeSpecifier
  declExpr>|<cell|int a>>|<row|<cell|varDefinition>|<cell|\<rightarrow\>>|<cell|varDecl=expression;>|<cell|int
  a=1;>>|<row|<cell|>|<cell|>|<cell|auto ref=expression;>|<cell|auto
  a=1;>>|<row|<cell|>|<cell|>|<cell|varDecl;>|<cell|int
  a;>>|<row|<cell|<text|expression-stmt>>|<cell|\<rightarrow\>>|<cell|<text|expression><wide*|<with|font-series|bold|;>|\<bar\>>\|<wide*|<with|font-series|bold|;>|\<bar\>>>|<cell|>>|<row|<cell|<text|selection-stmt>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|if>|\<bar\>>
  <wide*|<with|font-series|bold|(>|\<bar\>>
  <text|expression><wide*|<with|font-series|bold|)>|\<bar\>><text|statement>>|<cell|>>|<row|<cell|>|<cell|\|>|<cell|<wide*|<with|font-series|bold|if>|\<bar\>>
  <wide*|<with|font-series|bold|(>|\<bar\>>
  <text|expression><wide*|<with|font-series|bold|)>|\<bar\>> <text|statement>
  <wide*|<with|font-series|bold|else>|\<bar\>>
  <text|statement>>|<cell|>>|<row|<cell|<text|iteration-stmt>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|while>|\<bar\>>
  <wide*|<with|font-series|bold|(>|\<bar\>>
  <text|expression><wide*|<with|font-series|bold|)>|\<bar\>><text|statement>>|<cell|>>|<row|<cell|<text|return-stmt>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|return>|\<bar\>>
  <wide*|<with|font-series|bold|;>|\<bar\>>\|<wide*|<with|font-series|bold|return>|\<bar\>>
  <text|expression><wide*|<with|font-series|bold|;>|\<bar\>>>|<cell|>>>>>>

  <paragraph|Expressions>

  Supported operators:

  Arithmetic, logical, relation, dereference, address-of

  \;

  Precedence:

  <\enumerate>
    <item>id, literal, (Expression)

    Atom

    <item>call, subscript, member access

    Element

    <item>Unary plus, unary minus, deref, address-of (Right to left)

    Factor

    <item>Mul, div

    Mul-expr

    <item>add, sub

    Add-expr

    <item>\<less\>,\<less\>=,\<gtr\>,\<gtr\>=

    order-expr

    <item>==,!=

    eq-expr

    <item>= (right to left)

    Assignment
  </enumerate>

  R-value:

  <\eqnarray*>
    <tformat|<table|<row|<cell|atom>|<cell|\<rightarrow\>>|<cell|ID>>|<row|<cell|>|<cell|>|<cell|integer>>|<row|<cell|>|<cell|>|<cell|float>>|<row|<cell|>|<cell|>|<cell|string>>|<row|<cell|>|<cell|>|<cell|<around*|(|expression|)>>>|<row|<cell|element>|<cell|\<rightarrow\>>|<cell|element<around*|(|args|)>>>|<row|<cell|>|<cell|>|<cell|element<around*|[|expression|]>>>|<row|<cell|>|<cell|>|<cell|element.id>>|<row|<cell|>|<cell|>|<cell|atom>>|<row|<cell|factor>|<cell|\<rightarrow\>>|<cell|UnaryAddOp
    factor>>|<row|<cell|>|<cell|>|<cell|\<ast\>factor>>|<row|<cell|>|<cell|>|<cell|&factor>>|<row|<cell|>|<cell|>|<cell|element>>|<row|<cell|mulExpr>|<cell|\<rightarrow\>>|<cell|mulExpr
    mulOp factor>>|<row|<cell|>|<cell|>|<cell|factor>>|<row|<cell|addExpr>|<cell|\<rightarrow\>>|<cell|addExpr
    addOp mulExpr>>|<row|<cell|>|<cell|>|<cell|mulExpr>>|<row|<cell|orderExpr>|<cell|\<rightarrow\>>|<cell|orderExpr
    orderOp addExpr>>|<row|<cell|>|<cell|>|<cell|addExpr>>|<row|<cell|//EqExpr>|<cell|\<rightarrow\>>|<cell|EqExpr
    eqOp orderExpr>>|<row|<cell|//>|<cell|>|<cell|orderExpr>>|<row|<cell|assign>|<cell|\<rightarrow\>>|<cell|EqExpr=assign>>|<row|<cell|>|<cell|>|<cell|EqExpr>>|<row|<cell|Expression>|<cell|\<rightarrow\>>|<cell|//assign,Expression>>|<row|<cell|>|<cell|>|<cell|assign>>>>
  </eqnarray*>

  L-value: (Consider merging with R-value. Detect L-value/R-value during
  semantic analysis)

  <\eqnarray*>
    <tformat|<table|<row|<cell|L atom>|<cell|\<rightarrow\>>|<cell|ID>>|<row|<cell|>|<cell|>|<cell|<around*|(|L
    expression|)>>>|<row|<cell|L element>|<cell|\<rightarrow\>>|<cell|atom<around*|(|argList|)>>>|<row|<cell|>|<cell|>|<cell|atom<around*|[|expression|]>>>|<row|<cell|>|<cell|>|<cell|atom.id>>|<row|<cell|>|<cell|>|<cell|atom>>|<row|<cell|L
    factor>|<cell|\<rightarrow\>>|<cell|\<ast\>L
    element>>|<row|<cell|>|<cell|>|<cell|L
    element>>|<row|<cell|assign>|<cell|\<rightarrow\>>|<cell|L expr=expr>>>>
  </eqnarray*>

  <subsubsection|AST>

  <paragraph|Program structure>

  <math|<tabular|<tformat|<table|<row|<cell|<text|program>>|<cell|\<rightarrow\>>|<cell|vec<around*|(|definition|)>>>>>>>

  <paragraph|Definitions and declarations>

  <\eqnarray*>
    <tformat|<table|<row|<cell|ref>|<cell|\<rightarrow\>>|<cell|ID>>|<row|<cell|>|<cell|>|<cell|&ID>>|<row|<cell|>|<cell|>|<cell|empty>>|<row|<cell|>|<cell|>|<cell|&>>|<row|<cell|decl
    atom>|<cell|\<rightarrow\>>|<cell|ref>>|<row|<cell|>|<cell|>|<cell|<around*|(|decl
    expression|)>>>|<row|<cell|decl element>|<cell|\<rightarrow\>>|<cell|decl
    element<around*|(|params|)>>>|<row|<cell|>|<cell|>|<cell|decl
    element<around*|[|INTEGER|]>>>|<row|<cell|>|<cell|>|<cell|decl
    element<around*|[||]>>>|<row|<cell|>|<cell|>|<cell|atom>>|<row|<cell|expression>|<cell|\<rightarrow\>>|<cell|\<ast\>expression>>|<row|<cell|>|<cell|>|<cell|decl
    element>>>>
  </eqnarray*>

  <\enumerate>
    <item>decl expression

    <\description>
      <item*|identifier>
    </description>

    <\description>
      <item*|decl dereference>

      expression
    </description>

    <\description>
      <item*|declcall>

      expression

      params

      <item*|subscript>

      expression

      optional\<less\>int\<gtr\>
    </description>

    <item>var declaration
  </enumerate>

  \;

  <\eqnarray*>
    <tformat|<table|<row|<cell|params>|<cell|\<rightarrow\>>|<cell|param
    list>>|<row|<cell|>|<cell|>|<cell|void>>|<row|<cell|>|<cell|>|<cell|empty>>|<row|<cell|param
    list>|<cell|\<rightarrow\>>|<cell|param
    list,varDecl>>|<row|<cell|>|<cell|>|<cell|varDecl>>|<row|<cell|funcDefinition>|<cell|\<rightarrow\>>|<cell|varDec
    compoundStmt>>>>
  </eqnarray*>

  <\enumerate>
    <item>Definition

    <\description>
      <item*|function definition>

      var-decl

      compoundStmt
    </description>
  </enumerate>

  \;

  <\eqnarray*>
    <tformat|<table|<row|<cell|TypeSpecifier>|<cell|\<rightarrow\>>|<cell|ScalarType>>|<row|<cell|>|<cell|>|<cell|StructSpecification>>|<row|<cell|>|<cell|>|<cell|ID>>|<row|<cell|>|<cell|>|<cell|struct
    ID>>|<row|<cell|StructSpecification>|<cell|\<rightarrow\>>|<cell|struct
    ID<around*|{|definitions|}>>>|<row|<cell|>|<cell|>|<cell|struct
    <around*|{|definitions|}>>>|<row|<cell|Scalar
    Type>|<cell|\<rightarrow\>>|<cell|int>>|<row|<cell|>|<cell|>|<cell|float>>|<row|<cell|>|<cell|>|<cell|void>>|<row|<cell|>|<cell|>|<cell|auto>>>>
  </eqnarray*>

  <\enumerate>
    <item>TypeSpecifier

    <\description>
      <item*|StructSpecification>

      string id,vec\<less\>definition\<gtr\>

      <item*|NamedType>

      string
    </description>
  </enumerate>

  \;

  <\eqnarray*>
    <tformat|<table|<row|<cell|definitions>|<cell|\<rightarrow\>>|<cell|definitionList>>|<row|<cell|>|<cell|>|<cell|empty>>>>
  </eqnarray*>

  <\enumerate>
    <item>Definition

    <\description>
      <item*|varDef>

      var_decl,optional\<less\>expression\<gtr\>
    </description>
  </enumerate>

  <paragraph|Statement>

  <math|<tabular|<tformat|<table|<row|<cell|<text|compound-stmt>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|{>|\<bar\>>
  <text|statements><wide*|<with|font-series|bold|}>|\<bar\>>>|<cell|>>|<row|<cell|statements>|<cell|\<rightarrow\>>|<cell|statements
  statement>|<cell|>>|<row|<cell|>|<cell|>|<cell|empty>|<cell|>>|<row|<cell|<text|statement>>|<cell|\<rightarrow\>>|<cell|expression-stmt>|<cell|>>|<row|<cell|>|<cell|\|>|<cell|<text|compound-stmt>>|<cell|>>|<row|<cell|>|<cell|\|>|<cell|<text|selection-stmt>>|<cell|>>|<row|<cell|>|<cell|\|>|<cell|<text|iteration-stmt>>|<cell|>>|<row|<cell|>|<cell|\|>|<cell|<text|return-stmt>>|<cell|>>|<row|<cell|>|<cell|>|<cell|varDefinition>|<cell|>>|<row|<cell|varDec>|<cell|\<rightarrow\>>|<cell|TypeSpecifier
  declExpr>|<cell|int a>>|<row|<cell|varDefinition>|<cell|\<rightarrow\>>|<cell|varDecl=expression;>|<cell|int
  a=1;>>|<row|<cell|>|<cell|>|<cell|varDecl;>|<cell|int
  a;>>|<row|<cell|<text|expression-stmt>>|<cell|\<rightarrow\>>|<cell|<text|expression><wide*|<with|font-series|bold|;>|\<bar\>>\|<wide*|<with|font-series|bold|;>|\<bar\>>>|<cell|>>|<row|<cell|<text|selection-stmt>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|if>|\<bar\>>
  <wide*|<with|font-series|bold|(>|\<bar\>>
  <text|expression><wide*|<with|font-series|bold|)>|\<bar\>><text|statement>>|<cell|>>|<row|<cell|>|<cell|\|>|<cell|<wide*|<with|font-series|bold|if>|\<bar\>>
  <wide*|<with|font-series|bold|(>|\<bar\>>
  <text|expression><wide*|<with|font-series|bold|)>|\<bar\>> <text|statement>
  <wide*|<with|font-series|bold|else>|\<bar\>>
  <text|statement>>|<cell|>>|<row|<cell|<text|iteration-stmt>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|while>|\<bar\>>
  <wide*|<with|font-series|bold|(>|\<bar\>>
  <text|expression><wide*|<with|font-series|bold|)>|\<bar\>><text|statement>>|<cell|>>|<row|<cell|<text|return-stmt>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|return>|\<bar\>>
  <wide*|<with|font-series|bold|;>|\<bar\>>\|<wide*|<with|font-series|bold|return>|\<bar\>>
  <text|expression><wide*|<with|font-series|bold|;>|\<bar\>>>|<cell|>>>>>>

  <paragraph|Expressions>

  R-value:

  <\eqnarray*>
    <tformat|<table|<row|<cell|atom>|<cell|\<rightarrow\>>|<cell|ID>>|<row|<cell|>|<cell|>|<cell|integer>>|<row|<cell|>|<cell|>|<cell|float>>|<row|<cell|>|<cell|>|<cell|string>>|<row|<cell|>|<cell|>|<cell|<around*|(|expression|)>>>|<row|<cell|element>|<cell|\<rightarrow\>>|<cell|element<around*|(|args|)>>>|<row|<cell|>|<cell|>|<cell|element<around*|[|expression|]>>>|<row|<cell|>|<cell|>|<cell|element.id>>|<row|<cell|>|<cell|>|<cell|atom>>|<row|<cell|factor>|<cell|\<rightarrow\>>|<cell|UnaryAddOp
    factor>>|<row|<cell|>|<cell|>|<cell|\<ast\>factor>>|<row|<cell|>|<cell|>|<cell|&factor>>|<row|<cell|>|<cell|>|<cell|element>>|<row|<cell|mulExpr>|<cell|\<rightarrow\>>|<cell|mulExpr
    mulOp factor>>|<row|<cell|>|<cell|>|<cell|factor>>|<row|<cell|addExpr>|<cell|\<rightarrow\>>|<cell|addExpr
    addOp mulExpr>>|<row|<cell|>|<cell|>|<cell|mulExpr>>|<row|<cell|orderExpr>|<cell|\<rightarrow\>>|<cell|orderExpr
    orderOp addExpr>>|<row|<cell|>|<cell|>|<cell|addExpr>>|<row|<cell|//EqExpr>|<cell|\<rightarrow\>>|<cell|EqExpr
    eqOp orderExpr>>|<row|<cell|//>|<cell|>|<cell|orderExpr>>|<row|<cell|assign>|<cell|\<rightarrow\>>|<cell|EqExpr=assign>>|<row|<cell|>|<cell|>|<cell|EqExpr>>|<row|<cell|Expression>|<cell|\<rightarrow\>>|<cell|//assign,Expression>>|<row|<cell|>|<cell|>|<cell|assign>>>>
  </eqnarray*>

  <\enumerate>
    <item>expression

    <\description>
      <item*|var>

      id

      <item*|num>

      flag, union

      <item*|call>

      expression

      params

      <item*|subscript>

      expression

      expression

      <item*|MemberAccess>

      expression, member id

      <item*|UnaryAddExpr>

      op, expression

      <item*|dereference>

      expression

      <item*|addr-of>

      expression

      <item*|mulExpr>

      op, expr, expr

      <item*|addExpr>

      op, expr, expr

      <item*|relationalExpr>

      op, expr, expr

      <item*|assignExpr>

      l-expr,r-expr
    </description>
  </enumerate>

  <section|Problems>

  <\enumerate>
    <item>Function with L-reference return type.\ 

    e.g. <verbatim|int& function()> may be recognized as <verbatim|int
    (&function)()>, which is a reference to a function with <verbatim|int>
    return type, instead of <verbatim|(int&) function()>.\ 
  </enumerate>
</body>

<initial|<\collection>
</collection>>

<\references>
  <\collection>
    <associate|auto-1|<tuple|1|?>>
    <associate|auto-10|<tuple|2.2.1.4|?>>
    <associate|auto-11|<tuple|2.2.2|?>>
    <associate|auto-12|<tuple|2.2.2.1|?>>
    <associate|auto-13|<tuple|2.2.2.2|?>>
    <associate|auto-14|<tuple|2.2.2.3|?>>
    <associate|auto-15|<tuple|2.2.2.4|?>>
    <associate|auto-16|<tuple|3|?>>
    <associate|auto-2|<tuple|2|?>>
    <associate|auto-3|<tuple|2.1|?>>
    <associate|auto-4|<tuple|2.1.1|?>>
    <associate|auto-5|<tuple|2.2|?>>
    <associate|auto-6|<tuple|2.2.1|?>>
    <associate|auto-7|<tuple|2.2.1.1|?>>
    <associate|auto-8|<tuple|2.2.1.2|?>>
    <associate|auto-9|<tuple|2.2.1.3|?>>
  </collection>
</references>

<\auxiliary>
  <\collection>
    <\associate|toc>
      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|1<space|2spc>Language
      Feature Extensions> <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-1><vspace|0.5fn>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|2<space|2spc>Workload>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-2><vspace|0.5fn>

      <with|par-left|<quote|1tab>|2.1<space|2spc><with|font-family|<quote|tt>|language|<quote|verbatim>|LightIR>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-3>>

      <with|par-left|<quote|2tab>|2.1.1<space|2spc><with|font-family|<quote|tt>|language|<quote|verbatim>|struct>
      Support <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-4>>

      <with|par-left|<quote|1tab>|2.2<space|2spc><with|font-family|<quote|tt>|language|<quote|verbatim>|cminusf>
      Syntax <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-5>>

      <with|par-left|<quote|2tab>|2.2.1<space|2spc>CFG
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-6>>

      <with|par-left|<quote|4tab>|Program structure
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-7><vspace|0.15fn>>

      <with|par-left|<quote|4tab>|Declarations
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-8><vspace|0.15fn>>

      <with|par-left|<quote|4tab>|Statement
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-9><vspace|0.15fn>>

      <with|par-left|<quote|4tab>|Expressions
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-10><vspace|0.15fn>>
    </associate>
  </collection>
</auxiliary>