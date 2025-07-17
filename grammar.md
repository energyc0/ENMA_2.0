statement: 'print' expression ';'
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

expression: additive_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| '(' additive_expression ')'
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

additive_expression: multiplicative_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| multiplicative_expression '+' additive_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| multiplicative_expression '-' additive_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

multiplicative_expression: primary_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| primary_expression '*' multiplicative_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| primary_expression '/' multiplicative_expression
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

primary_expression: constant_number
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;| variable
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

constant_number:  T_INT
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;

variable:  T_IDENT
&ensp; &ensp; &ensp; &ensp; &ensp; &ensp; &ensp;;