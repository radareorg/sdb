char *json_get (const char *s, const char *k) {
	char *v = "";
	// split 'k' by '.' .. walk the dinosaur
	return strdup (v);
}

char *json_set (const char *s, const char *k, const char *v) {
	return NULL;
}

main() {
	char *a = "{}";
	a = json_seti (a, "id", 123);
	a = json_seti (a, "user.name", "blah");
	printf ("id = %d\n", json_geti ("{'id':123}", "id"));
}
