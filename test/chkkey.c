#include <sdb.h>

#define MAX 19999999

int main() {
	int i;
	for (i = 0; i < MAX ; i++) {
		if (!sdb_check_key ("foo123bar.cow0x830803")) {
			return 1;
		}
	}
	return 0;
}
