# Grammar

## Program
```
<program> ::= <declarations>

<declarations> ::= <declaration> <declaration>*

<declaration> ::= <function_declaration>
| <function_definition>
| <variable_declaration>
| <class_declaration>

<variable_declaration> ::= "var" <variable> "=" <expression> ";"

<function_declaration> ::= "func" <identifier> "(" <arglist>? ")" ";"

<function_definition> ::= "func" <identifier> "(" <arglist>? ")" <code_block>

<class_declaration> ::= "class" <identifier> (":" <classlist>)? <class_block>
```
## Statements

```
<statements> ::= <statement> <statement>*

<statement> ::= <if_statement>
| <for_statement>
| <while_statement> 
| <break_statement>
| <continue_statement>
| <return_statement>
| <variable_declaration>
| <code_block>
| <class_block>
| <expression_statement>

<if_statement> ::= "if" "(" <logical_expression> ")" <code_block>

<for_statement> ::= "for" "(" (<variable_declaration> | <expression>)? ";" <logical_expression>? ";" <expression>? ")" (<code_block> | ";")

<while_statement> ::= "while" "(" <logical_expression> ")" <code_block>

<break_statement> ::= "break" ";"

<continue_statement> ::= "continue" ";"

<return_statement> ::= "return" <expression>? ";"

<code_block> ::= "{" <statements>? "}"

<expression_statement> ::= <expression> ";"

<class_block> ::= "{" <class_statements>? "}"

<class_statements> ::= <class_statement> <class_statement>*

<class_statement> ::= <method_definition> | <field_declaration>

<method_definition> ::= "meth" <identifier> "(" <arglist>? ")" <code_block>

<field_declaration> ::= "field" <identifier> ";"
```

## Expressions
```
<expression> ::= <numerical_expression>
| <logical_expression>
| <string_expression>
| <assignment_expression>
| <function_call>
| <get_property>
| "(" <expression> ")"

<numerical_expression> ::= (<variable> | <number>) (<numerical_op> <numerical_expression>)?

<logical_expression> ::= ("not " <logical_expression>) | ( (<variable> | <boolean>) (<logical_op> <logical_expression>)? )

<string_expression> ::= (<variable> | <string>) (<string_op> <string_expression>)?

<assignment_expression> ::= <variable> "=" <expression>

<function_call> ::= <identifier> "(" <arglist>? ")"

<get_property> ::= <instance> "." (<get_property> | <property>)

<property> ::= <variable> | <function_call>

<instance> ::= <variable> | <constructor>

<constructor> ::= <class_name> "(" <arglist>? ")"

<variable> ::= <identifier>
```
## Lexical grammar
```
<arglist> ::= <identifier> ("," <identifier>)*

<classlist> ::= <class_name> ("," <class_name>)*

<class_name> ::= <identifier>

<string_op> ::= "+"

<logical_op> ::= " and " | " or " | " xor " | " not "

<numerical_op> ::= "+" | "-" | "/" | "*"

<number> ::=  ("+" | "-")? <digits>

<boolean> ::=  ("false" | "true")

<string> ::=  "\"" <chars>? "\""

<identifier> ::=  <alpha> (<chars>)*

<chars> ::= <char> (<char>)*

<char> ::= <alpha> | <digit> | "_"

<digits> ::= <digit> (<digit>)*

<digit> ::= [0-9]

<alpha> ::= [a-z] | [A-Z]
```
