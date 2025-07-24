statement: 'print' expression ';'
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

expression: additive_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| oring_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| string_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| '(' expression ')'
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

additive_expression: multiplicative_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| multiplicative_expression '+' additive_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| multiplicative_expression '-' additive_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

multiplicative_expression: primary_arith_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| primary_arith_expression '*' multiplicative_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| primary_arith_expression '/' multiplicative_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

primary_arith_expression: constant_number
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| numerical_variable
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

oring_expression: primary_bool_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| xoring_expression 'or' oring_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

xoring_expression: primary_bool_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| anding_expression 'xor' xoring_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

anding_expression: primary_bool_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| negate_expression 'and' anding_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

negate_expression: primary_bool_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| 'not' primary_bool_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

primary_bool_expression: constant_boolean
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| boolean_variable
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

constant_number:  T_INT
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

constant_boolean:  T_FALSE
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| T_TRUE
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

constant_string: '"' * '"'
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

numerical_variable:  T_IDENT
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

boolean_variable:  T_IDENT
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

string_variable:  T_IDENT
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;