Introduction
============

When an application needs to store data it usually makes use an embedded SQLite, text-based configuration files, json or xml. All of them are pretty different in essence and depend on specific parsers to serialize/deserialize the contents from disk to memory.

The added complexity and duplicated data structures creates some issues from the development stage (which may lead to bugs) and in memory usage because the application will be using different data structures to hold the information in ram.

Sdb was born with the aim to address those issues by simplifing the core design and providing a rich api to work with it in different forms.

In SDB the only type is the string. All supported data types are converted into strings, which makes data serialization a first class citizen. You can use this serialization for network syncronization, human-friendly reading or data storage.

The database is composed by a double linked list and a hashtable. Those data structures are supported for both disk and memory. This allows the database to query a huge data file without taking more memory because the file is mmaped and strings are nullterminated so it can just use pointers.

The hashtable stores 'key' and 'value' pairs. This is the most basic way to store information, keys are uniques and values can contain any kind of data serialized into a string. This is, you can get arrays by using comma separated format in the value.

Description
-----------

This is the short description that comes from the README.md:

    sdb is a simple string key/value database based on djb's cdb
    disk storage and supports JSON and arrays introspection.
    
    mcsdbd is a memcache server with disk storage based on sdb.
    It is distributed as a standalone binary and a library.
    
    There's also the sdbtypes: a vala library that implements
    several data structures on top of an sdb or a memcache instance.

Contains
--------

- namespaces (multiple sdb paths)
- atomic database sync (never corrupted)
- bindings for vala, luvit, newlisp and nodejs
- commandline frontend for sdb databases
- memcache client and server with sdb backend
- arrays support (syntax sugar)
- json parser/getter (js0n.c)

Rips
----
- disk storage based on cdb code
- memory hashtable based on wayland code
- linked lists from r2 api

Changes
-------

I have modified cdb code a little to create smaller databases and
be memory leak free in order to use it from a library.

The sdb's cdb database format is 10% smaller than the original
one. This is because keylen and valuelen are encoded in 4 bytes:
1 for the key length and 3 for the value length.

In a test case, a 4.3MB cdb database takes only 3.9MB after this
file format change.
