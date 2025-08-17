# ENMA programming language 2.0
This language is written in C and is improved over the previous version (previous version was written in C++ and had a lot of troubles).

## About
This is just a common programming language which supports variables, conditionals, loops, functions and classes. I created it for my practice, because I love programming <3.

## Program structure
Program consists of declaration which can be either a function declaration or a function definition or a variable definition. All this transforms into statements and expression. More info you can find in **grammar.md**. 

Program must contain **main** function that is an entry function.

## OOP
Every class has its constructors, methods and fields called **properties**. You can also create constructors.

## Program execution
Firstly, it translates user-written code into bytecode for virtual machine. Then my virtual machine executes the code. More info about bytecode you can find in **bytecode.md**.

## Program example
```
func main();
func sum(a,b);
func foo();

var global_var = 0;

func main(){
  for(var i = 0; i < 5; i++){
    foo();
  }

  println(sum(1+2, 5 - 6/3));
  println("Hello" + ", " + "world!");

  var a = 11;
  if(a > 10){
    println("a is greater than 10");
  }else{
    println("a is not greater than 10");
  }

  println(a > 10);
}

func foo(){
  println(++global_var);
}

func sum(a,b){
  return a + b;
}
```
