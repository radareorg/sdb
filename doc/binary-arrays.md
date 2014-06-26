Serializing Arrays
==================

Arrays can be accessed by using the Sdb.Array API as explained in the Arrays section. But if you want a full array serialization/deserialization those methods will not perform as well as the way Sdb.Fmt API does.

That API exposes basic functions to convert an array of numbers or strings from a string to native data. This is a simple example in C:

	/* test array of numbers */
	ut64 *nums = sdb_fmt_array_num (
		"1,3,,5,8,10,0x33");
	int chks[] = {1,3,0,5,8,10,0x33};
	int i;
	for (i=0; i<*nums; i++) {
	    printf ("%d.. ", (int)nums[i+1]);
	    if (chks[i] != nums[i+1]) {
	        free (nums);
	        return 1;
	    }
	}
	printf ("\n");
	free (nums);

The following example shows how to convert an array of strings into a single allocated buffer containing all the elements.

	/* test array of strings */
	char **strs = sdb_fmt_array (
	    "foo,bar,cow,low,mem,,jiji");
	const char *oks[] = {"foo","bar",
	    "cow","low","mem","","jiji"};
	if (strs)
	for (i=0; strs[i]; i++) {
	    printf ("%s %s\n",
		!strcmp (oks[i], strs[i])?
		  "OK":"FAIL", strs[i]);
	}
	free (strs);
