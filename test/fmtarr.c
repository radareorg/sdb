#include <sdb.h>

int main() {
	int i;
	/* test array of numbers */
	ut64 *nums = sdb_fmt_array_num("1,3,,5,8,10,0x33");
	int chks[] = {1,3,0,5,8,10,0x33};
	for (i=0;i<*nums;i++) {
		printf ("%d.. ", (int)nums[i+1]);
		if (chks[i] != nums[i+1]) {
			printf ("FAIL FAIL FAIL\n");
			free (nums);
			return 1;
		}
	}
	printf ("\n");
	free (nums);

	/* test array of strings */

	char **strs = sdb_fmt_array ("foo,bar,cow,low,mem,,jiji");
	const char *oks[] = {"foo","bar","cow","low","mem","","jiji"};
	if (strs)
		for (i=0;strs[i];i++) {
			if (!strcmp (oks[i], strs[i]))
				printf ("OK - %s \n", strs[i]);
			else printf ("FAIL - %s vs %s\n", strs[i], oks[i]);
		}
	printf ("\n");
	free (strs);
	return 0;
}
