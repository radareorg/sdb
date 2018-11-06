SList
=====

The SList structure was living in `r_util` and was almost not used
at all, the purpose of it is to resolve an item which is stored
in a list sorted by offset. The key is an ut64 offset that must be
covered by each item.

This was used before siol. Which currently solves the problem slist
was aiming to achieve. Nevertheless, it's nice to not kill it completely
and keep it in the Attic because it can be useful for someone or
can be improved and even be used back in r2 again.
