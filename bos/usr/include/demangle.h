/* @(#)93	1.1  src/bos/usr/include/demangle.h, xlC, bos411, 9428A410j 2/20/92 11:44:08 */
/*
 * COMPONENT_NAME: (BOSBUILD) Build Tools and Makefiles
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*************************************************************************
 
    utilC++/filter/demangle.h, C++-util, C++.1.0
 
    demangle.h
 
    This file contains the C and C++ interfaces to the C++ name demangler. 
The library provides the function "Demangle" and a number of classes. 
"Demangle" is used to convert mangled C++ names to (pointers to) instances 
of the "Name" class (or some class derived from "Name"). Once such an object 
has been created, the user can find out certain characteristics of the name. 
 
There are five subclasses of "Name", organized in the following way:
 
			Name
		       /  |  \
	    SpecialName  / \  ClassName
		        /   \
                       /  FunctionName  
	              /	        \
	             /           \
              MemberVarName     MemberFunctionName
 
The "SpecialName" class is for special compiler generated class objects, 
while the "ClassName" class is for names that are mangled class names.
The objects that the other subclasses represent should be self-evident. 
 
The demangler will only demangle "special" names and names that are only
class names when the "SpecialNames" and "ClassNames" flags, respectively,
are supplied to the demangler in the "nameSpace" parameter. This parameter
takes the default value of "RegularNames", and so it will by default 
demangle only regular names. "SpecialNames", "ClassNames" and "RegularNames"
can be specified in any combination simply by "or"-ing them together.
 
The "Qualifier" class is an auxiliary class that represents a member name's 
qualifier.
 
An important feature of the demangler is that, while it is a C++ program, it
uses no features requiring the C++ runtime library, and hence it is not
necessary to link libC.a to the program containing demangle.o. This affects
the interface in only one way. Normally, the class "Name" would be a virtual
class, but due to the lack of libC.a, pure virtual functions cannot be used.
The user should always treat the Name class as though it is a virtual.
 
The most common operation is expected to be simply the retrieval of the 
string representation of the demangled name, and is accomplished by the
virtual "Name" member "Text", which returns a pointer to a string. This
string is part of the Name object; if a user wants to change it or have
it exist beyond the lifetime of the object, s/he must make a copy of the
string. There is also a "char *" conversion operator for "Name" (and 
its derived classes).
 
Other information about a demangled name is available. This is accomplished
by first determining the actual kind of the given name, via the virtual
method "Kind" of "Name", which returns a "NameKind" value (see below). Once
this value is determined, the "Name *" can be cast to the appropriate 
derived class, and the methods of this class can then be called. These 
methods are defined below, and their purposes should be clear.
 
Currently, analyzing the contents of a function's argument list is not
possible (despite the "Arguments" method of "FunctionName" and 
"MemberFunctionName") because the appropriate classes are not publically
available; if there is a demand for this capability, it can be provided.
 
***********************************************************************/
 
#ifndef __DEMANGLEH
#define __DEMANGLEH
 
enum Boolean {False = 0, True = 1};
 
enum NameKind { VirtualName, MemberVar, Function, MemberFunction, Class, 
		Special };
enum NameClasses { RegularNames = 0x1, ClassNames = 0x2, SpecialNames = 0x4 };
 
#ifdef __cplusplus
 
class CommonType;
class TypeList;
class Argument;
class ArgumentList;
 
class Name;
 
/*
 * Demangle. Given a valid C "name" and the address of a char pointer, this 
 * function creates a "Name" instance and returns its address. A valid C name
 * is one starting with an "_" or letter followed by a number of letters, digits
 * and "_"s. The name is assumed to start at the beginning of the string, but 
 * there may be trailing characters not part of the mangled name. A pointer 
 * into "name" at the first character not part of the mangled name is returned
 * in "rest".
 *     Demangle will return NULL when the text of the demangled name is the 
 * same as the text of the mangled name. Thus, when NULL is returned, the 
 * character string given as Demangle's first argument is in fact the 
 * demangled name, too.
 */ 
 
Name *Demangle(char *name, char *&rest, ulong nameSpace = RegularNames);
 
class Name {
    public:
	virtual ~Name() {}
	virtual NameKind Kind() { return VirtualName; }
 
	virtual char *Text() { return (char *)NULL; }
	operator char *() { return Text(); }
};
 
class Qualifier {
	unsigned long nQNames;
	unsigned long refCount;
 
	struct ClassName {
	    char *name;
	    ArgumentList *args;
	} **qualifiers;
 
	friend Qualifier *ValidQualifier(char *, unsigned long &);
        friend ClassName *ValidClassName(char *mName, unsigned long &i);
 
	friend class MemberVarName;
	friend class MemberFunctionName;
 
	char *text;
    public:
	Qualifier();
	~Qualifier();
 
	virtual char *Text();
	unsigned long nQualifiers() { return nQNames; }
};
 
class ClassName: public Name {
	Qualifier *qualifier;
    public:
	ClassName(Qualifier *q) { qualifier = q; }
	~ClassName() { delete qualifier; }
	virtual NameKind Kind() { return Class; }
 
	virtual char *Text() { return qualifier->Text(); }
	unsigned long nQualifiers() { return qualifier->nQualifiers(); }
};
	
class SpecialName: public Name {
	char *text;
    public:
	SpecialName(char *t) { text = t; }
	virtual ~SpecialName() { delete text; }
	virtual NameKind Kind() { return Special; }
 
	virtual char *Text() { return text; }
};
	
class MemberVarName: public Name {
	char *text;
	// the raw variable name
	char *name;
	// the qualifier
	Qualifier *qualifier;
 
	// if the member is const ...
	Boolean isConstant:8;
	// if the member is static ...
	Boolean isStatic:8;
	// if the member is volatile ...
	Boolean isVolatile:8;
 
    public:
	MemberVarName(char *, unsigned long, Qualifier *, Boolean, Boolean, 
		      Boolean);
	virtual ~MemberVarName();
 
	virtual NameKind Kind() { return MemberVar; }
	char *VarName() { return name; }
	Qualifier *Scope() { return qualifier; }
	virtual char *Text() { return text; }
 
	Boolean IsConstant() { return (Boolean)isConstant; }
	Boolean IsStatic() { return (Boolean)isStatic; }
	Boolean IsVolatile() { return (Boolean)isVolatile; }
};
 
class FunctionName: public Name {
    protected:
	char *name;          // the function name
	TypeList *arguments; // the arguments of the function 
 
	char *text;
    public:
	FunctionName(char *, unsigned long, TypeList *);
	virtual ~FunctionName();
 
	virtual NameKind Kind() { return Function; }
	char *RootName() { return name; }
	virtual char *Text() { return text; }
};
 
class MemberFunctionName: public FunctionName {
	Qualifier *qualifier;  // the qualifier
	Boolean isConstant: 8; // if the function is const ...
	Boolean isStatic: 8;   // if the function is static ...
	Boolean isVolatile: 8; // if the function is volatile ...
 
	char *text;
    public:
	MemberFunctionName(char *, unsigned long, Qualifier *, TypeList *, 
			   Boolean, Boolean, Boolean);
	virtual ~MemberFunctionName();
 
	virtual NameKind Kind() { return MemberFunction; }
	Qualifier *Scope() { return qualifier; }
	virtual char *Text() { return text; }
 
	Boolean IsConstant() { return (Boolean)isConstant; }
	Boolean IsStatic() { return (Boolean)isStatic; }
	Boolean IsVolatile() { return (Boolean)isVolatile; }
};
 
#else
 
/*
 * The C Interface
 */
 
/*
 *     demangle. Given a valid C "name" and the address of a char pointer, this 
 * function creates a "Name" instance and returns its address. A valid C name
 * is one starting with an "_" or letter followed by a number of letters, digits
 * and "_"s. The name is assumed to start at the beginning of the string, but 
 * there may be trailing characters not part of the mangled name. A pointer 
 * into "name" at the first character not part of the mangled name is returned
 * in "rest":
 *     struct Name *demangle(char *name, char **rest, unsigned long);
 */ 
 
    struct Name *demangle();
 
/*
 * Each of the following functions takes a pointer to a Name as its only 
 * parameter.
 */ 
 
    enum NameKind kind(/* struct Name * */);
 
    /* return the character representation of a given struct Name */
    char *text(/* struct Name * */);
 
    /* return the actual name of a given Var- or MemberVar-type struct Name */
    char *varName(/* struct Name * */);
 
    /* return the qualifier text of the given Member-type struct Name */
    char *qualifier(/* struct Name * */);
 
    /* return the actual name of a given Function- or MemberFunction- type  */
    /* struct Name 							    */
    char *functionName(/* struct Name * */);
 
    /* is a Member-type struct Name constant? */
    enum Boolean isConstant(/* struct Name * */);
 
    /* is a Member-type struct Name static? */
    enum Boolean isStatic(/* struct Name * */);
 
    /* is a Member-type struct Name volatile? */
    enum Boolean isVolatile(/* struct Name * */);
 
    /* delete the Name instance */
    void erase(/* struct Name * */);
 
#endif
#endif
