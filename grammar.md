# Grammar
**statement**: print_statement ';' \
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| if_statement\
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| for_statement\
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| while_statement\ 
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| break_statement\
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| continue_statement\
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| variable_declaration\
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;; 

**print_statement**: 'print' expression ';' \

**if_statement**: 'if' '(' logical_expression ')' code_block

**for_statement**: 'for' '(' [variable_declaraion, expression] ';' logical_expression ';' expression ')' [code_block, ';'] 

**while_statement**: 'while' '(' logical expression ')' code_block

**break_statement**: 'break' ';'

**continue_statement**: 'continue' ';'

**variable_declaration**: 'var' variable '=' expression ';'

**declaration**: \
**expression**: logical_expression\
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| logical_expression\
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| string_expression\
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| '(' expression ')'\
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;


**constant_number**:  T_INT\
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

**constant_boolean**:  T_FALSE\
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| T_TRUE\
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

**constant_string**: '"' * '"'\
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

**variable**:  T_IDENT\
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;
