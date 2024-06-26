# OS Assignments in Xv6

## Problem
Implement a slab allocator in xv6 for allocating struct file.
Rewrite all the relevant parts of the code.
Submit a patch that is created with the master branch.

> Resources: virtual-memory-3-thrashing-...-mem-mgmt.odp

### Notes
> slab allocator - technique of managing mem allocation for the kernel's own needs
> buddy allocator - problems solved with slab allocator

- file.h
- filealloc -> creates a struct file and gives a pointer to f
- ftable.file -> array of struct files
all struct files come from this ftable.file array
- alocated statically
- same approach for struct proc, inode etc.

Slab Allocator:
- supposed to change filealloc
- memory should come from a slab, not a global array
- allocate a page, the page is equally divided into same-sized objects
- logic of checking ref count won't change: 0 : not in use, else in use
- once one page is full, allocate another
- linked list of pages - all serve as cache

```
Q. How to figure out cache is full or not?
- bitmap or refcount loop - your choice
```
### Sample snippet
```
file *filecache;
void create_file_cache() {
    k = kalloc();
    create the cache in this page pointed by k;
    filecache = created cache;
}
struct file *get_file_from_filecache() {
    iterate over the file cache & find empty struct file
    if cache is full, grow it;
    return it
}
file *return file_to_file_cache() {
    mark a file object as unused;
    if cache is empty, shrink it, if needed;
}
fileaaloc()
{
    acquire(ftable.lock)
    f = get_file_from_filecache();
    release(ftable.lock)
}
in main() {
    call after kvmalloc, create_file_cache();
}
```
Figure out which function is the reverse of filealloc, and change it too.


