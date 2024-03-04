# SIR
## Overview
The project `Shizuku` mainly uses `SIR` (Shizuku intermediate representation) to optimize the design and generate executable code.

## Data type
There are 5 primitive data types and 3 aggregate data types:

1. Primitive data types:
* Variable  
Variable is specified by keyword `var`. It has 3 parameters: `width`, `xz`, `signed`
```
	var<width=1, xz=false, signed=false>
```
* Net  
Net is specified by keyword `net`, it takes 3 parameters:
`width`, `strength`, `signed`
```
	net<width=1, strength=false, signed=false>
```
The differences of `net` & `var` is the mostly described by the `Verilog` LRM.

* Pointer  
Pointer is specified by keyword `pt`. It takes only 1 parameter `pty` which specifies the type of pointee, and it must be specified explicitly.
```
	pt<pty>
```
* String  
String is specified by keyword `str`. It takes no parameter.
* Real  
Real is specified by keyword `real`. It takes no parameter.

2. Aggregate data type: 
* Array  
`Array` is specified by keyword `array`, it takes 2 parameters:
`width` and `ety`.  
`width` defines the width of array, it must be unsigned integer number. In case `width` is positive number, it declares array width fixed width. If `width` is 0, it declares array width dynamic width.  
`ety` defines the element type, it can be any primitive or aggregation date type.
```
	array<var>
```
* Structure  
Structure uses keyword `struct` to specify data types assembled together. It takes no parameter. And the member date types are enclosed by parentheses, the data members are separated by comma.
```
	struct {
		var,
		var,
	}
```
* Union  
The syntax of union is the same as structure except that it is declared by keyword `union`. Unlike structure, the members of union share the same memory.  

For any data type, user can create an alias for it using keyword `alias`.  
```
	alias struct { var, var } myStruct;
``` 
Please note `SIR` check if data types are compatible by its specified content data types rather than any alias name. Alias name is actually not really by `SIR` used after it is expanded to real data type, it's just for the ease of reading.

## Variable
There are 2 kinds variables, global variable and local variable.

* Global variable is declared outside any routine, it always has static lifetime.
* Local variable is declared in routine, it may has static or dynamic lifetime.

### Variable naming convention
* Global variable name starts with a leading '@' character, global variable name is always globally unique and can't be declared more than once. 
* Local variable name starts with a leading % character. Local global name is unique only current routine. 
* Any other leading character is considered as illegal. And the convention that @ is for global variable and % for local is mandatory.  
* The string follows leading character must contain at least 1 character. And following regular expression describes a variable name.
```
	[@|%][a-zA-z0-9_:]+	// regular expression of variable name

	// examples
	@globalname			// legal
	%localname			// legal
	localname			// illegal, leading character is not @ or $
	@a::b__				// legal
	@a$					// illegal, $ is not a legal character
	@a@%				// illegal, @ % is for leading character only
```

### Declare variable and SSA
Like many other IR language, `SIR` also follows the `SSA` (single static assignment) rule. In other word, variable must and only be assigned when it is declared.

Variable can be declared at global scope (global variable) or under the scope of routine (local variable). Both share the same syntax.
```
var_name = var_value;
```
The type of declared variable is implicitly specified by var value. It works similarly like `auto` keyword of `c++`, which helps eliminating unexpected casting.

