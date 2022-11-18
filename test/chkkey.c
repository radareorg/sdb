#include <sdb/sdb.h>

#define MAX 19999999

int main() {
	eprintf ("sdb_check_key: has been deprecated\n");
#if 0
	int i;
	for (i = 0; i < MAX ; i++) {
		if (!sdb_check_key ("foo123bar.cow0x830803")) {
			return 1;
		}
	}
#endif
	return 0;
}
