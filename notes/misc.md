### Questions 

1. *What does the pointer type hierarchy look like if we design the pointer types from an OO view?* 
2. *How to handle pointer arithmetic, especially situations where* *a pointer temporarily goes out of boundary?* 
3. *Should Checked C do checking when dangling pointer is created or when a dereference happens?* 
4. *What's a very efficient data structure to store the metadata of the point-to relations?* 
5. *What kinds of static analysis can we apply to avoid as many dynamic checking as possible?* 
6. *How should the checked code and unchecked code interact?* 
7. *Challenges in multi-threading programs.* 

 

These questions are somewhat intertwined. It's almost impossible to answer one well without considering the design and implementation of others.  

 

#### What does the pointer type hierarchy look like if we design the pointer types in an Object-Oriented style?  

We can design pointer types in an OO style. One general rule is that the child type inherits the restrictions enforced on its parent pointer type and carries more restrictions about what the legal operations are allowed on it. As a result, using a child pointer type is usually more secure than its parent type. Casting from the a child type to its parent type is allowed while the reverse is usually prohibited (at least with constraints), as most OO programming languages do. When checked code calls unchecked code and needs pass a safe pointer, we can first cast the safe pointer to its parent type. 

The root type is the generic void * pointer.  The _Ptr and _Array_Ptr both inherit from the root type.  For each of the two types, we can have a _Stack_Ptr and a _Heap_Ptr inherit from _Ptr, and a _Stack_Array_Ptr and a _Heap_Array_Ptr inherit from _Array_Ptr  to denotes a pointer that points to object living on the stack, i.e., local variables allocated during function prologue,  and a pointer that points to an object living on the heap , i.e., from calling a memory allocator, respectively. This classification may enable better and easier optimization of removing unnecessary checks during compilation.   

We can have more fine-grained pointer types based on not only where a pointer is pointing to but also on where the pointer itself lives.  For example, inheriting from _Ptr, there could be 6 different subtypes of pointers: 

- _Stack_Stack_Ptr (a pointer living on the stack and pointing to an object on the stack) 
- _Stack_Heap_Ptr (a pointer living on the stack and pointing to an object on the heap) 
- _Heap_Stack_Ptr (a pointer living on the heap and pointing to an object on the stack) 
- _Heap_Heap_Ptr (a pointer living on the heap and pointing to an object on the heap; this type is the most dangerous: UAF vulnerabilities usually happen by exploiting these kinds of pointers) 
- _Global_Stack_Ptr (a pointer living on the data region and pointing to an object on the stack) 
- _Global_Heap_Ptr(a pointer living on the  data region and pointing to an object on the heap)  

Inheriting from _Array_Ptr, there would also be 6 children pointers. 

It's arguable that we need not keep track of pointers living on the stack because they are usually short-lived and thus are very difficult to exploit. If we want to take the risk, we can get rid of the first two types of pointers listed above.  

 

#### How to handle pointer arithmetic, especially situations a pointer temporarily goes out of boundary? 

This largely depends on how the data structures for the metadata of point-to relations are designed and implemented. We discuss more about this in Question 4. 

Pointer arithmetic can be roughly categorized into two types. One is that static analysis knows if the result pointer is in or out of bounds of the base pointer; the other is not. Different types require different actions when it comes to tracking the point-to relations. We discuss more about this later. 

This idiom that there are temporal out-of-bounds pointers is not very common for most programs. The [Beyond BDP11 CHERI paper](https://www.cl.cam.ac.uk/research/security/ctsrd/pdfs/201503-asplos2015-cheri-cmachine.pdf) analyzed 13 programs (1.9 millions SLOC in total). 6 programs don't have any this kind of pointers; 6 programs have at most 11; and only one program (tcpdump) has 1,299 this kind of invalid intermediate results among 66,555 SLOC.  

 

#### Should Checked C do checking when dangling pointer is created or when dereferenced? 

 I think this question and the data structure and optimization questions are tightly intertwined with each other. Each approach can provide thorough inspection. I don't see a strong reason for choosing any one of them.   

Let's dub checking on dangling pointer creation CDPC and checking on pointer dereference CPD for future reference. 

Further reading and thinking is needed. 

 

#### What's a very efficient data structure to store the metadata of the point-to relations? 

We can keep the backward compatibility by keeping the metadata of pint-to relations in a disjoint data structure rather than adopting a "fat-pointer" strategy, just like for _Array_Ptr we require bounds information for it when a pointer is declared but don't incorporate the bounds information into the pointer itself.  How we design the data structure for keeping the metadata largely depend on our checking strategy.  

 

##### A very naïve approach 

A very simple and straightforward method is to label each allocated memory object (for stack object, we can treat the whole stack for the current function as one object) with a unique identifier (or capability, versioning, etc.), and put all the labels of living objects in a pool (presumably implemented as a set). How the point-to relations are maintained depend on the checking strategy we adopt.  

 

*Checking When a Pointer is Dereferenced (CPD)* 

Besides the set that contains all the living IDs, we need a hash table whose <key, value> == <addr_of_ptr, ID>. 

 Each time a new point-to relation is created, e.g., pointer assignment or new memory allocation,  Checked C puts the address (not the value) of the pointer and the ID of the object as a pair in the hash table.   

 

Questions 2 

Checked C puts the new pointer and the object pointed to by the base pointer in the pointer arithmetic expression as a pair to the hash table. For CPD, Checked C relies on the bounds checking that has been done. If the access is within the legal bounds, Checked C inserts a dynamic check on if the ID coupled with the pointer's address is still alive in the ID pool. For every memory deallocation, Checked C removes the ID from the ID pool. There is no need removing the point-to relation from the hash table. 

This might be the most naïve approach. Its memory consumption could be high.  

 

*Checking When Dangling Pointers are Created (CDPC)* 

We only need maintain a hash table whose <key, value> == <ID, set<addr_of_ptr>>. When a new object is created, we insert a new pair of <ID, ptr> to the hash table and each time other point-to relation is created, we updated the corresponding set. For each memory deallocation, we removed the corresponding item in the hash table and invalidate all the pointers in the corresponding set.  

 

Questions 2 

When pointer arithmetic generates a new pointers that can be statically proven to be in the bounds of the base pointer, the ID is the object that pointed by the base pointer; if the result can be proven to be out of bounds, we don't put anything into the hash table. If the new pointer is never dereferenced, great; if it's dereferenced, it'll be caught by the bounds-checking mechanism.  

If Checked C cannot decides where the new pointer points to, we can instrument the pointer arithmetic to get the new bounds dynamically and check if it's in or out of bounds. If it's in bounds, then we put new data in the hash table and if it's out of bounds, do nothing. I think this could incur insanely high runtime overhead for pointer arithmetic intensive programs.  

 

 ##### What kinds of static analysis can we apply to avoid as many dynamic checking as possible? 

- Path compression? 



### Some observations

- No matter which way we choose – checking on the creation or use of dangling pointers, it'd be super expensive to do a check on each event. For good performance, Checked C must do aggressive static analysis to remove unnecessary checks
- It seems unavoidable that we need some extra data structures to keep information of pointer – object relations. It'd very expensive to update relevant data structures each time we update a pointer.  
- There is no temporal memory safety guarantee when checked and on unchecked code interacts.  