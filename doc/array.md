Arrays
======

Arrays are one of the supported complex types in Sdb. They are stored as comma separated values and can be used as a Stack, a List, an Array or a Queue.

That flexibility comes for free when using strings as a basic type. If you think in comma separated values you will quickly see that there is a flaw in the design because this adds an exception for the comma character.

The way we decided to solve this is by using an encoder and a decoder. This keeps the iterator logic simple, without escaping characters and reduces the size of the strings and adds support for binary blobs.

The encoder used is base64 and it allows us to store binary blobs, multiple line text and strings containing special chars like ';' or ','.

    > array=1,2,3,4
    > [0]array
    < 1

Stack
-----

Arrays in Sdb can be used a stack. Stack operations prepend the new value to the original list. The syntax for using it from `sdb_query` is `[++]` and `[--]`.

    > [++]foo=3
    > [++]foo=2
    > [++]foo=1
    > foo
    < 1,2,3

The following syntax corner case will push an empty element in the stack.

    > [++]foo=

And finally we can construct an invalid query which results in unexpected behaviour:

    > [--]foo=

As long as the syntax of the query commands is so flexible, it may lead to several syntax corner cases that must be handled properly. This is up for discussion and requires some feedback and proper regression tests to ensure nothing breaks without control.
