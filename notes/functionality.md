# A list of functionalities that the new safe pointers should support.

Note: the **ptr** appeared in the following text means the new safe pointer.

- assign the address of newly allocated heap object
- assign NULL
- assign NULL to an array of pointers statically
- assign one safe pointer to another

- have ptr inside a struct; check whether `sizeof` can determine the size correctly
- dereference a ptr pointing to an allocated object
- dereference a null ptr
- dereference a ptr pointing to a freed object

- pass 1 ptr as function argument
- pass n ptr as function arguments
- return ptr from a function

- pointer arithmetic, ptr++/--, ptr + const, ptr1 + ptr2
- Pointer type cast
