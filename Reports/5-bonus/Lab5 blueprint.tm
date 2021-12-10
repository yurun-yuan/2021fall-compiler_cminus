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

  <math|<tabular|<tformat|<table|<row|<cell|<text|program>>|<cell|\<rightarrow\>>|<cell|<text|declaration-list>>>|<row|<cell|<text|declaration-list>>|<cell|\<rightarrow\>>|<cell|<text|declaration-list><text|declaration>\|<text|declaration>>>|<row|<cell|<text|declaration>>|<cell|\<rightarrow\>>|<cell|<text|var-declaration>\|<text|fun-declaration>>>>>>>

  <paragraph|Declarations>

  <math|<tabular|<tformat|<table|<row|<cell|<text|var-declaration>>|<cell|\<rightarrow\>>|<cell|<text|type-specifier><wide*|<with|font-series|bold|ID>|\<bar\>><wide*|<with|font-series|bold|;>|\<bar\>>>>|<row|<cell|>|<cell|\|>|<cell|<text|type-specifier><wide*|<with|font-series|bold|ID>|\<bar\>>
  <wide*|<with|font-series|bold|[>|\<bar\>>
  <wide*|<with|font-series|bold|INTEGER>|\<bar\>>
  <wide*|<with|font-series|bold|]>|\<bar\>>
  <wide*|<with|font-series|bold|;>|\<bar\>>>>|<row|<cell|<text|type-specifier>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|int>|\<bar\>>
  <around|\||<wide*|<with|font-series|bold|float>|\<bar\>>|\|>
  <wide*|<with|font-series|bold|void>|\<bar\>>>>|<row|<cell|>|<cell|>|<cell|>>|<row|<cell|<text|fun-declaration>>|<cell|\<rightarrow\>>|<cell|<text|type-specifier><wide*|<with|font-series|bold|ID>|\<bar\>>
  <wide*|<with|font-series|bold|(>|\<bar\>>
  <text|params><wide*|<with|font-series|bold|)>|\<bar\>><text|compound-stmt>>>|<row|<cell|<text|params>>|<cell|\<rightarrow\>>|<cell|<text|param-list>\|<wide*|<with|font-series|bold|void>|\<bar\>>>>|<row|<cell|<text|param-list>>|<cell|\<rightarrow\>>|<cell|<text|param-list><wide*|<with|font-series|bold|,>|\<bar\>><text|param>\|<text|param>>>|<row|<cell|<text|param>>|<cell|\<rightarrow\>>|<cell|<text|type-specifier><wide*|<with|font-series|bold|ID>|\<bar\>>\|<text|type-specifier><wide*|<with|font-series|bold|ID>|\<bar\>>
  <wide*|<with|font-series|bold|[>|\<bar\>>
  <wide*|<with|font-series|bold|]>|\<bar\>>>>|<row|<cell|<text|compound-stmt>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|{>|\<bar\>>
  <text|local-declarations><text|statement-list><wide*|<with|font-series|bold|}>|\<bar\>>>>|<row|<cell|<text|local-declarations>>|<cell|\<rightarrow\>>|<cell|<text|local-declarations
  var-declaration>\|<text|empty>>>>>>>

  <paragraph|Statement>

  <math|<tabular|<tformat|<table|<row|<cell|<text|statement-list>>|<cell|\<rightarrow\>>|<cell|<text|statement-list><text|statement>\|<text|empty>>>|<row|<cell|<text|statement>>|<cell|\<rightarrow\>>|<cell|expression-stmt>>|<row|<cell|>|<cell|\|>|<cell|<text|compound-stmt>>>|<row|<cell|>|<cell|\|>|<cell|<text|selection-stmt>>>|<row|<cell|>|<cell|\|>|<cell|<text|iteration-stmt>>>|<row|<cell|>|<cell|\|>|<cell|<text|return-stmt>>>|<row|<cell|<text|expression-stmt>>|<cell|\<rightarrow\>>|<cell|<text|expression><wide*|<with|font-series|bold|;>|\<bar\>>\|<wide*|<with|font-series|bold|;>|\<bar\>>>>|<row|<cell|<text|selection-stmt>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|if>|\<bar\>>
  <wide*|<with|font-series|bold|(>|\<bar\>>
  <text|expression><wide*|<with|font-series|bold|)>|\<bar\>><text|statement>>>|<row|<cell|>|<cell|\|>|<cell|<wide*|<with|font-series|bold|if>|\<bar\>>
  <wide*|<with|font-series|bold|(>|\<bar\>>
  <text|expression><wide*|<with|font-series|bold|)>|\<bar\>> <text|statement>
  <wide*|<with|font-series|bold|else>|\<bar\>>
  <text|statement>>>|<row|<cell|<text|iteration-stmt>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|while>|\<bar\>>
  <wide*|<with|font-series|bold|(>|\<bar\>>
  <text|expression><wide*|<with|font-series|bold|)>|\<bar\>><text|statement>>>|<row|<cell|<text|return-stmt>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|return>|\<bar\>>
  <wide*|<with|font-series|bold|;>|\<bar\>>\|<wide*|<with|font-series|bold|return>|\<bar\>>
  <text|expression><wide*|<with|font-series|bold|;>|\<bar\>>>>>>>>

  <paragraph|Expressions>

  <math|<tabular|<tformat|<table|<row|<cell|<text|expression>>|<cell|\<rightarrow\>>|<cell|<text|var><wide*|<with|font-series|bold|=>|\<bar\>><text|expression>\|<text|simple-expression>>>|<row|<cell|<text|var>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|ID>|\<bar\>>\|<wide*|<with|font-series|bold|ID>|\<bar\>>
  <wide*|<with|font-series|bold|[>|\<bar\>>
  <text|expression><wide*|<with|font-series|bold|]>|\<bar\>>>>|<row|<cell|<text|simple-expression>>|<cell|\<rightarrow\>>|<cell|<text|additive-expression><text|relop><text|additive-expression>\|<text|additive-expression>>>|<row|<cell|<text|relop>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|\<less\>=>|\<bar\>>
  <around|\||<wide*|<with|font-series|bold|\<less\>>|\<bar\>>|\|>
  <wide*|<with|font-series|bold|\<gtr\>>|\<bar\>>
  <around|\||<wide*|<with|font-series|bold|\<gtr\>=>|\<bar\>>|\|>
  <wide*|<with|font-series|bold|==>|\<bar\>>\|<wide*|<with|font-series|bold|!=>|\<bar\>>>>|<row|<cell|<text|additive-expression>>|<cell|\<rightarrow\>>|<cell|<text|additive-expression><text|addop><text|term>\|<text|term>>>|<row|<cell|<text|addop>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|+>|\<bar\>>\|<wide*|<with|font-series|bold|->|\<bar\>>>>|<row|<cell|<text|term>>|<cell|\<rightarrow\>>|<cell|<text|term><text|mulop><text|factor>\|<text|factor>>>|<row|<cell|<text|mulop>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|>|\<bar\>>\|<wide*|<with|font-series|bold|/>|\<bar\>>>>|<row|<cell|<text|factor>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|(>|\<bar\>>
  <text|expression><wide*|<with|font-series|bold|)>|\<bar\>>
  <around|\||<text|var>|\|> <text|call><around|\||<text|integer>|\|><text|float>>>|<row|<cell|<text|integer>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|INTEGER>|\<bar\>>>>|<row|<cell|<text|float>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|FLOATPOINT>|\<bar\>>>>|<row|<cell|<text|call>>|<cell|\<rightarrow\>>|<cell|<wide*|<with|font-series|bold|ID>|\<bar\>>
  <wide*|<with|font-series|bold|(>|\<bar\>>
  <text|args><wide*|<with|font-series|bold|)>|\<bar\>>>>|<row|<cell|<text|args>>|<cell|\<rightarrow\>>|<cell|<text|arg-list>\|<text|empty>>>|<row|<cell|<text|arg-list>>|<cell|\<rightarrow\>>|<cell|<text|arg-list><wide*|<with|font-series|bold|,>|\<bar\>><text|expression>\|<text|expression>>>>>>>
</body>

<initial|<\collection>
</collection>>

<\references>
  <\collection>
    <associate|auto-1|<tuple|1|?>>
    <associate|auto-10|<tuple|2.2.1.4|?>>
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
    </associate>
  </collection>
</auxiliary>