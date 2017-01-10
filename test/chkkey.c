#include <sdb.h>

#define MAX 19999999

int main() {
	int i;
	eprintf ("sdb_check_key: has been deprecated\n");
#if 0
	for (i = 0; i < MAX ; i++) {
		if (!sdb_check_key ("foo123bar.cow0x830803")) {
			return 1;
		}
	}
#endif
	return 0;
}
