#include <sdb.h>

int main() {
	int i = 0;
	for (i=0;i<1999999;i++) {
		sdb_now ();
	}
	return 0;
}
