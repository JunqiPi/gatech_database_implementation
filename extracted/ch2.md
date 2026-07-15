# ch2

## Slide 1

Chapter 2: Intro to Relational Model


## Slide 2

Outline

Structure of Relational Databases
Database Schema
Keys
Schema Diagrams
Relational Query Languages
The Relational Algebra


## Slide 3

Example of a Instructor  Relation

attributes
(or columns)

tuples
(or rows)


## Slide 4

Relation Schema and Instance

A1, A2, …, An are attributes
R = (A1, A2, …, An ) is a relation schema
	Example:
	     instructor  = (ID,  name, dept_name, salary)
A relation instance r defined over schema R is denoted  by r (R).
The current values a relation are specified by a table
An element t of relation r is called a  tuple and is represented by a row in a table


## Slide 5

Attributes

The set of allowed values for each attribute is called the domain of the attribute
Attribute values are (normally) required to be atomic; that is, indivisible
The special value null  is a member of every domain. Indicated that the value is “unknown”
The null value causes complications in the definition of many operations


## Slide 6

Relations are Unordered

Order of tuples is irrelevant (tuples may be stored in an arbitrary order)
Example: instructor  relation with unordered tuples


## Slide 7

Database Schema

Database schema -- is the logical structure of the database.
Database instance -- is a snapshot of the data in the database at a given instant in time. 
Example:
schema:   instructor (ID, name, dept_name, salary)
Instance:


## Slide 8

Keys

Let K ⊆ R
K is a superkey of R if values for K are sufficient to identify a unique tuple of each possible relation r(R) 
Example:  {ID} and {ID,name} are both superkeys of instructor.
Superkey K is a candidate key if K is minimalExample:  {ID} is a candidate key for Instructor
One of the candidate keys is selected to be the primary key.
Which one?
Foreign key constraint: Value in one relation must appear in another
Referencing relation
Referenced relation
Example: dept_name in instructor  is a foreign key from instructor referencing department


## Slide 9

Schema Diagram for University Database


## Slide 10

Relational Query Languages

Procedural versus non-procedural, or declarative
“Pure” languages:
Relational algebra
Tuple relational calculus
Domain relational calculus
The above 3 pure languages are equivalent in computing power
We will concentrate in this chapter on relational algebra


## Slide 11

Relational Algebra

An algebraic language consisting  of a set of operations that take one or two relations as input and produce a new relation as their result. 
Six basic operators
select: σ
project: Π
union: ∪
set difference: – 
Cartesian product: x
rename: ρ


## Slide 12

The  select operation selects tuples that satisfy a given predicate.
Notation:  σ p (r)
p is called the selection predicate
Example: select those tuples of the instructor  relation where the instructor is in the “Physics” department.
Query
   	σ dept_name=“Physics” (instructor)

Select Operation

Result


## Slide 13

Select Operation (Cont.)

We allow comparisons using 
                     =, ≠, >, ≥. <. ≤
       in the selection predicate. 
We can combine several predicates into a larger predicate by using the connectives:
                   ∧ (and), ∨ (or), ¬ (not)
Example: Find the instructors in Physics with a salary greater $90,000, we write:
           σ dept_name=“Physics” ∧ salary > 90,000 (instructor)
 
The select predicate may  include comparisons between two attributes. 
Example, find all departments whose name is the same as their building name:
 σ dept_name=building  (department)


## Slide 14

Project Operation Example

Example: eliminate the dept_name attribute of instructor
Query:          	 ΠID, name, salary (instructor)

Result (order is irrelevant)


## Slide 15

Project Operation

A unary operation that returns its argument relation, with certain attributes left out.  
Notation:
                  Π A1,A2,A3 ….Ak  (r)	
	where A1, A2,  …, Ak  are attribute names and r is a relation name.
The result is defined as the relation of k columns obtained by erasing the columns that are not listed
Duplicate rows removed from result, since relations are sets


## Slide 16

Composition of Relational Operations

The result of a relational-algebra operation is relation  and therefore of relational-algebra operations can be composed together into a relational-algebra expression.
Consider  the query -- Find the names of all instructors in the Physics department.
 
             Πname(σ dept_name =“Physics”  (instructor))
 
Instead of giving the name of a relation as the argument of the projection operation, we give an expression that evaluates to a relation.


## Slide 17

Cartesian-Product Operation

The Cartesian-product operation (denoted by X)  allows us to combine information from any two relations.  
Example: the Cartesian product of the relations instructor and teaches is written  as:
                instructor  X  teaches
We construct a tuple of the result out of each possible pair of tuples: one from the instructor relation and one from the teaches relation (see next slide)
Since the instructor ID appears in both relations we distinguish between these attribute by attaching to the attribute the name of the relation from which the attribute originally came.
instructor.ID
teaches.ID


## Slide 18

The  instructor  X  teaches  table


## Slide 19

Join Operation

The Cartesian-Product 
                    instructor  X  teaches
      associates every  tuple of  instructor with every tuple of teaches.
Most of the resulting rows have information about instructors who did NOT teach a particular course. 
To get only those tuples of  “instructor  X  teaches “ that pertain to instructors and the courses that they taught, we write:
            σ instructor.id =  teaches.id  (instructor  x teaches ))
 
We get only those tuples of “instructor  X  teaches” that pertain to instructors and the courses that they taught.
The result of this expression, shown in the next slide


## Slide 20

Join Operation (Cont.)

The  table corresponding to:
            σ instructor.id =  teaches.id (instructor  x teaches))


## Slide 21

Join Operation (Cont.)


## Slide 22

Union Operation

The union operation allows us to combine two relations 
Notation:  r  ∪ s
For r  ∪ s to be valid.
	1.   r, s must have the same arity (same number of attributes)
	2.   The attribute domains must be compatible (example: 2nd 
     column of r deals with the same type of values as does the 
     2nd column of s)
Example: to find all courses taught in the Fall 2017 semester, or in the Spring 2018 semester, or in both   Πcourse_id (σ semester=“Fall”  Λ year=2017 (section))  ∪     Πcourse_id (σ semester=“Spring”  Λ year=2018 (section))


## Slide 23

section table


## Slide 24

Union Operation (Cont.)

Result of:    Πcourse_id (σ semester=“Fall”  Λ year=2017 (section))  ∪     Πcourse_id (σ semester=“Spring”  Λ year=2018 (section))

∪

=


## Slide 25

Set-Intersection Operation

The  set-intersection  operation  allows us to find tuples that are in both the input relations.
Notation: r ∩ s
Assume: 
r, s have the same arity 
attributes of r and s are compatible
Example: Find the set of all courses taught in both the Fall 2017 and the Spring 2018 semesters.
           Πcourse_id (σ semester=“Fall”  Λ year=2017 (section)) ∩      Πcourse_id (σ semester=“Spring”  Λ year=2018 (section))
 
Result


## Slide 26

Set Difference Operation

The set-difference operation allows us to find tuples that are in one relation but are not in another. 
Notation r – s
Set differences must be taken between compatible relations.
r and s must have the same arity
attribute domains of r and s must be compatible
Example: to find all courses taught in the Fall 2017 semester, but not in the Spring 2018 semester   Πcourse_id (σ semester=“Fall”  Λ year=2017 (section))  −     Πcourse_id (σ semester=“Spring”  Λ year=2018 (section))


## Slide 27

The Assignment  Operation

It is convenient at times to write a relational-algebra expression by assigning parts of it to temporary relation variables.  
The assignment  operation is  denoted by ← and works like assignment in a programming language.
Example: Find all instructor in the “Physics” and Music department.         Physics ← σ dept_name=“Physics” (instructor)
       Music ← σ dept_name=“Music” (instructor)
       Physics  ∪ Music
 
With the assignment operation, a query can be written as a sequential program consisting of a series of assignments followed by an expression whose value is displayed as the result of the query.


## Slide 28

The Rename Operation

The results of relational-algebra expressions do not have a name that we can use to refer to them.  The  rename operator,  ρ ,  is provided  for that purpose
The expression:
                  ρx (E)
      returns the result of expression E under the name x
Another form of the rename operation:
                 ρx(A1,A2, .. An) (E)


## Slide 29

Aggregate Functions

These functions operate on the multiset (set with duplicates) of values of a column of a relation, and return a value
		avg: average value	min:  minimum value	max:  maximum value	sum:  sum of values	count:  number of values


## Slide 30

Aggregate Functions Examples

Find the average salary of instructors
γavg(salary)(instructor)
Find the total number of courses in 2018
γcount(course_id)(σ year =2018 (section))


## Slide 31

Aggregate Functions – Group By

Find the average salary of instructors in each department
dept_nameγavg(salary)(instructor)


## Slide 32

Equivalent Queries

There is more than one way to write a query in relational algebra. 
Example:  Find information about courses taught by instructors in the Physics department with salary greater than 90,000
Query 1
           σ dept_name=“Physics” ∧ salary > 90,000 (instructor)
 
Query 2
          σ dept_name=“Physics” (σ salary > 90.000 (instructor))
 
The two queries are not identical; they are, however, equivalent -- they give the same result on any database.


## Slide 33

Equivalent Queries


## Slide 34

End of Chapter 2
