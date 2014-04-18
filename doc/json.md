JSON
====

JSON is a popular format to store structured data into arrays and hashtables in textual form. The name stands for JavaScript Object Notation, but it is in fact a little more strict because it requires all keys to be quoted (not in js).

Sdb supports to parse and modify jsons using a query path which points to the target and then allows to modify its value which is directly serialized into a valid JSON value (boolean, decimal number, quoted string).

This is the most basic example:

    > json={"hello":"world"}
    > json:hello
    < world

You may notice the ':' character used to split the key containing the json string and the path to access the given element.

We can also set a new value for this field:

    > json:hello=123
    > json
    < {"hello":123}

JSON also supports arrays, and they can be used as a replacement for `sdb_array`, but it should not beat in speed.

Those are some other examples using more JSON features:

    > json={"foo":[1,2,3],"bar":true}
    > json:foo[0]
    < 1

    > json={"foo":[1,2,3],"bar":true}
    > json:foo[1]=false
    > json:bar=
    > json
    {"foo":[1,false,3]}
