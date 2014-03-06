#include <sdb.h>

int main() {
	#define STRUCT_PERSON_FORMAT "dsqd"
	#define STRUCT_PERSON_SIZE sizeof (struct Person)
	typedef struct person {
		int foo;
		char *str;
		ut64 fin;
		int end;
	} Person;

	Person p;

	sdb_fmt_init (&p, "dsqd");
	sdb_fmt_tobin ("123,bar,321,1", "dsqd", &p);
	eprintf ("--> %d,%s\n", p.foo, p.str);
	eprintf ("--> %lld,%d\n", p.fin, p.end);

	{
		char *o = sdb_fmt_tostr (&p, "dsqd");
		eprintf ("== %s\n", o);
		free (o);
	}
	sdb_fmt_free (&p, "dsqd");
}
