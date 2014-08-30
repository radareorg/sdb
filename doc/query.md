Queries
=======

SDB Queries are strings that are evaluated by the `sdb_query()` parser. This is the textual interface to interact with the Sdb database and their namespaces.

Syntax for sdb queries may look strange, so it requires some explanation and discussion, because maybe it will change/evolve in the future to be more javascript compliant.

Examples
--------

Get the value of 'foo' key:

    foo

Set the new value to 'foo':

    foo=bar

NOTE: that strings are not quoted, numbers, arrays, json, and other values are also defined like this, no quotes or numeric sign needed.

Set a number to 'foo':

    foo=42

Increment that key:

    +foo=1

Decrement it:

    -foo=1

Arrays
------

Array brackets and index are prefixed too, see the following examples:

    foo=one,two,tri
    [1]foo

The first line creates a new array of 3 elements. The second will retrieve the nth element from the array (array indexes in sdb start from 0). In will display 'two'.

Using the same prefix syntax we can set the new value for that element:

    [1]foo=doge

To delete an element we have two different methods to unset or delete, this is, use it as an array or as a list. If we unset an element from an array, the content will be removed and the array will contain empty buckets. For example:

    foo=1,2,3,4
    [1]foo=
    foo

Those lines will show '1,,3,4'. What happened here is that positive array indexes are handled as an `sdb_unset` and the negative ones shift the contents removing the bucket.

    foo=1,2,3,4
    [-1]foo=
    foo

The output is: '1,3,4'

Encoded strings
---------------

Encoding strings work like this:

    %foo=Hello
    foo          ; show 'aGVsbG8='
    %foo         ; show 'Hello'

Namespaces
----------

Accessing namespaces with an sdb query is done by the '/' char in the 'key' part. It will recursively walk inside each namespace. For example:

    foo/bar=3    ; sdb_num_set(DB, "bar", sdb_ns(DB, "foo"), 3);

The use of slashes dynamically creates new Sdb instances under the given namespace. And it also permits to use the array, encoding, numeric prefixes:

    %foo/bar=Hello
