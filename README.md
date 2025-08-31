# ENMA programming language 2.0
This language is written in C and is improved over the previous version (previous version was written in C++ and had a lot of troubles).

## About
This is just a common programming language which supports variables, conditionals, loops, functions and classes. I created it for my practice, because I love programming <3. 

ENMA is a dynamically typed language. You can assign any value to variables in any time.

Functions take arguments of any type. The only condition is that arguments count must be the same as in definition.

You can forward declare functions (not methods) and define them later.

Functions can return any values in any cases or return nothing. For example, a function can return a string or a number that depends on some condition.

## Value types
ENMA has three four value types:
 1. *Number*
 2. *Boolean*
 3. *String*
 4. *Instance*

## Program structure and syntax
Program consists of declaration which can be either a class declaration, a function declaration, a function definition or a variable definition. All this transforms into statements and expression. More info you can find in **grammar.md**. 

Program must contain **main** function that is an entry function.

## OOP
Every class has its constructors, methods and fields called **properties**. Every class have its own base constructor if it doesn't have any. To define a field used keyword **field**. To define a method used keyword **meth**. To define a constructor used just the name of the class (like in C++). Fields, methods and constructors are all public.

## Program execution
Firstly, it translates user-written code into bytecode for virtual machine. Then my virtual machine executes the code. More info about bytecode you can find in **bytecode.md**.

## Built-in function
You can see the list of built-in function in **builtin.md**

## How to build
```bash
make
```
which will create **enma** executable in **build/release/**.
```bash
make debug
```
which will create **enma.dbg** executable in **build/debug/**.
```bash
make clean
```
which will just delete **debug/** directory.

## How to use
```bash
enma [enma source file]
```
it just takes one argument and interprets the code.

## Program example
```c++
class Dog{
  field name;
  field age;

  Dog(name_, age_){
    name = name_;
    age = age_;
  }

  meth sound(){
    println("Bark!");
  }

  meth getname(){
    return name;
  }

  meth getage(){
    return age;
  }

  meth eat(food){
    println(name, " has eaten ", food, ". Yammy!");
  }
}

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


  var mydog = Dog("Oliver", 5);
  println("Dog's name is ", mydog.getname(), ". It is ", mydog.getage(), " years old.");
  mydog.sound();
  mydog.eat("sandwich");
}

func foo(){
  println(++global_var);
}

func sum(a,b){
  return a + b;
}
```
### Output
```
1
2
3
4
5
6
Hello, world!
a is greater than 10
true
Dog's name is Oliver. It is 5 years old.
Bark!
Oliver has eaten sandwich. Yammy!
```

## TODO list
- Bytecode optimization
- Add more options