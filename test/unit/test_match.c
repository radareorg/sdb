#include "minunit.h"
#include <sdb/sdb.h>

bool test_match_glob_prefix(void) {
	mu_assert ("prefix glob matches", sdb_match ("fcnlink.0x100", "fcnlink.*"));
	mu_assert ("star matches empty run", sdb_match ("fcnlink.", "fcnlink.*"));
	mu_assert ("prefix glob rejects other prefix", !sdb_match ("fcn.0x100", "fcnlink.*"));
	mu_assert ("glob is anchored at start", !sdb_match ("xfcnlink.0x100", "fcnlink.*"));
	mu_assert ("pattern longer than string", !sdb_match ("fcn", "fcnlink.*"));
	mu_end;
}

bool test_match_glob_contains(void) {
	mu_assert ("contains glob matches middle", sdb_match ("axb", "*x*"));
	mu_assert ("contains glob matches edges", sdb_match ("xb", "*x*"));
	mu_assert ("contains glob rejects", !sdb_match ("ab", "*x*"));
	mu_assert ("contains glob with dots", sdb_match ("fcn.arg.0", "*.arg.*"));
	mu_assert ("contains glob with dots rejects", !sdb_match ("fcn.argx.0", "*.arg.*"));
	mu_end;
}

bool test_match_glob_suffix(void) {
	mu_assert ("suffix glob matches", sdb_match ("asfx", "*sfx"));
	mu_assert ("suffix glob matches whole string", sdb_match ("sfx", "*sfx"));
	mu_assert ("suffix glob rejects", !sdb_match ("sfxa", "*sfx"));
	mu_end;
}

bool test_match_glob_all(void) {
	mu_assert ("star matches anything", sdb_match ("anything", "*"));
	mu_assert ("star matches empty string", sdb_match ("", "*"));
	mu_assert ("double star matches too", sdb_match ("anything", "**"));
	mu_end;
}

bool test_match_glob_icase(void) {
	mu_assert ("icase glob matches", sdb_match ("FCNLINK.FOO", "fcnlink.*?i"));
	mu_assert ("icase glob matches lower", sdb_match ("fcnlink.foo", "fcnlink.*?i"));
	mu_assert ("case sensitive glob rejects", !sdb_match ("FCNLINK.FOO", "fcnlink.*"));
	mu_assert ("icase contains glob", sdb_match ("aXb", "*x*?i"));
	mu_end;
}

bool test_match_glob_anchors(void) {
	// ^ and $ are redundant in globs, they must not change the meaning
	mu_assert ("^ prefix ignored in glob", sdb_match ("fcnlink.0x100", "^fcnlink.*"));
	mu_assert ("$ suffix ignored in glob", sdb_match ("fcnlink.0x100", "fcnlink.*$"));
	mu_assert ("^..$ ignored in glob", sdb_match ("fcnlink.0x100", "^fcnlink.*$"));
	mu_assert ("^..$ glob still anchored", !sdb_match ("xfcnlink.y", "^fcnlink.*$"));
	// the $ anchor must stay a no-op when hidden by the ?i suffix
	mu_assert ("$?i ignored in glob", sdb_match ("FCNLINK.X", "fcnlink.*$?i"));
	mu_assert ("^..$?i ignored in glob", sdb_match ("FCNLINK.X", "^fcnlink.*$?i"));
	mu_assert ("$?i glob not literal", !sdb_match ("fcnlink.x$", "fcnlink.*x$?i"));
	mu_end;
}

bool test_match_glob_backtrack(void) {
	mu_assert ("multi star", sdb_match ("xxaxxbxxc", "*a*b*c*"));
	mu_assert ("interleaved stars", sdb_match ("abc", "a*b*c"));
	mu_assert ("stars require order", !sdb_match ("acb", "a*b*c"));
	mu_assert ("backtracking terminates", !sdb_match ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "*a*a*a*a*b"));
	mu_assert ("backtracking matches late", sdb_match ("aaab", "*a*a*a*b"));
	mu_end;
}

bool test_match_glob_base64(void) {
	char *e = sdb_encode ((const ut8 *)"fcnlink.foo", -1);
	mu_assert_notnull (e, "encode works");
	mu_assert ("base64 glob matches", sdb_match (e, "%fcnlink.*"));
	mu_assert ("base64 glob rejects", !sdb_match (e, "%fcn.*x"));
	free (e);
	// decoded data with embedded nuls must not truncate the glob match
	e = sdb_encode ((const ut8 *)"ab\0cd", 5);
	mu_assert_notnull (e, "encode works");
	mu_assert ("base64 nul glob suffix", sdb_match (e, "%*cd"));
	mu_assert ("base64 nul glob span", sdb_match (e, "%ab*cd"));
	mu_assert ("base64 nul glob rejects", !sdb_match (e, "%*cx"));
	free (e);
	mu_end;
}

bool test_match_substring_regression(void) {
	// patterns without '*' must keep the old substring semantics
	mu_assert ("substring matches", sdb_match ("food", "oo"));
	mu_assert ("substring rejects", !sdb_match ("food", "ox"));
	mu_assert ("prefix anchor", sdb_match ("food", "^foo"));
	mu_assert ("prefix anchor rejects", !sdb_match ("sfood", "^foo"));
	mu_assert ("suffix anchor", sdb_match ("food", "ood$"));
	mu_assert ("suffix anchor rejects", !sdb_match ("foods", "ood$"));
	mu_assert ("exact match", sdb_match ("food", "^food$"));
	mu_assert ("exact match rejects", !sdb_match ("foods", "^food$"));
	mu_assert ("icase substring", sdb_match ("FOOD", "oo?i"));
	mu_assert ("plain substring key", sdb_match ("fcnlink.0x100", "fcnlink"));
	mu_end;
}

bool test_unset_like_glob(void) {
	Sdb *db = sdb_new0 ();
	sdb_set (db, "fcnlink.0x100", "a", 0);
	sdb_set (db, "fcnlink.0x200", "b", 0);
	sdb_set (db, "fcn.0x300", "c", 0);
	sdb_set (db, "other", "fcnlink.0x400", 0);
	int n = sdb_unset_like (db, "fcnlink.*");
	mu_assert_eq (n, 2, "Wiped keys");
	mu_assert ("fcnlink.0x100 gone", !sdb_exists (db, "fcnlink.0x100"));
	mu_assert ("fcnlink.0x200 gone", !sdb_exists (db, "fcnlink.0x200"));
	mu_assert ("fcn.0x300 survives", sdb_exists (db, "fcn.0x300"));
	mu_assert ("other survives", sdb_exists (db, "other"));
	sdb_free (db);
	mu_end;
}

bool test_unset_like_substring(void) {
	// no '*' in the pattern: keys containing the substring are wiped
	Sdb *db = sdb_new0 ();
	sdb_set (db, "alinka", "a", 0);
	sdb_set (db, "boo", "b", 0);
	int n = sdb_unset_like (db, "link");
	mu_assert_eq (n, 1, "Wiped keys");
	mu_assert ("alinka gone", !sdb_exists (db, "alinka"));
	mu_assert ("boo survives", sdb_exists (db, "boo"));
	sdb_free (db);
	mu_end;
}

int all_tests() {
	mu_run_test (test_match_glob_prefix);
	mu_run_test (test_match_glob_contains);
	mu_run_test (test_match_glob_suffix);
	mu_run_test (test_match_glob_all);
	mu_run_test (test_match_glob_icase);
	mu_run_test (test_match_glob_anchors);
	mu_run_test (test_match_glob_backtrack);
	mu_run_test (test_match_glob_base64);
	mu_run_test (test_match_substring_regression);
	mu_run_test (test_unset_like_glob);
	mu_run_test (test_unset_like_substring);
	return tests_passed != tests_run;
}

int main(int argc, char **argv) {
	return all_tests ();
}
