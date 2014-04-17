Arrays
======

Arrays are one of the supported complex types in Sdb. They are stored as comma separated values and can be used as a Stack, a List, an Array or a Queue.

That flexibility comes for free when using strings as a basic type. If you think in comma separated values you will quickly see that there is a flaw in the design because this adds an exception for the comma character.

The way we decided to solve this is by using an encoder and a decoder. This keeps the iterator logic simple, without escaping characters and reduces the size of the strings and adds support for binary blobs.

The encoder used is base64 and it allows us to store binary blobs, multiple line text and strings containing special chars like ';' or ','.
