CAS
===

CAS stands for 'check' and 'set'. and it is a trick used by distributed key values to detect failures on the consistence of the contents.

When you are working with several keys that may be accessed by another process or thread in local or remote location we will need to verify that we are modifying known data. This is, everytime someone sets a new value to a key it can define a 'cas' value which is used to later verify that no one else have modified that key.

	TODO: example, more verbosity
