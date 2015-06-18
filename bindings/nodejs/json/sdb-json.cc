/* sdb-nan.js - pancake@nopcode.org // 2015 */

#include <nan.h>
#include <sdb.h>

using namespace v8;

extern "C" {
	char *api_json_get (const char *s, const char *p);
	char *api_json_set (const char *s, const char *p, const char *v);
}


NAN_METHOD(GetVersion) {
	NanReturnValue(NanNew(SDB_VERSION));
}

NAN_METHOD(Encode) {
	int len = args.Length();
	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		char *str = sdb_encode ((const ut8*)*k, -1);
		if (str) {
			Local<String> v = NanNew(str);
			free (str);
			NanReturnValue(v);
		}
	}
}

NAN_METHOD(JsonIndent) {
	int len = args.Length();
	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		char *res = sdb_json_indent (*k);
		Local<String> v = NanNew(res);
		free (res);
		NanReturnValue(v);
	}
}

NAN_METHOD(JsonUnindent) {
	int len = args.Length();
	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		char *res = sdb_json_unindent (*k);
		Local<String> v = NanNew(res);
		free (res);
		NanReturnValue(v);
	}
}

NAN_METHOD(TypeOf) {
	int len = args.Length();
	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		NanReturnValue(NanNew(sdb_type (*k)));
	}
}

NAN_METHOD(Decode) {
	int len = args.Length();
	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		char *str = (char *)sdb_decode (*k, NULL);
		if (str) {
			Local<String> v = NanNew(str);
			free (str);
			NanReturnValue(v);
		}
	}
}

NAN_METHOD(JsonGet) {
	int len = args.Length();
	if (len == 2) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		NanUtf8String v (args[1]);
		char *str = api_json_get (*k, *v);
		if (str) {
			Local<String> v = NanNew(str);
			free (str);
			NanReturnValue(v);
		}
	} else {
		NanThrowTypeError ("wrong number of arguments");
	}
}

NAN_METHOD(JsonSet) {
	int len = args.Length();
	if (len == 3) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		if (!args[1]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String p (args[0]);
		NanUtf8String k (args[1]);
		if (args[1]->IsString()) {
			NanUtf8String v (args[2]);
			char *str = api_json_set (*p, *k, *v);
			if (str) {
				Local<String> v = NanNew(str);
				free (str);
				NanReturnValue(v);
			}
		} else {
			NanThrowTypeError ("TODO: support more types");
		}
	} else {
		NanThrowTypeError ("wrong number of arguments");
	}
}

void Init(Handle<Object> exports, Handle<Value> module) {
	/* generic */
	exports->Set(NanNew("version"), NanNew(SDB_VERSION));
	/* base64 */
	exports->Set(NanNew("encode"), NanNew <FunctionTemplate>(Encode)->GetFunction());
	exports->Set(NanNew("decode"), NanNew <FunctionTemplate>(Decode)->GetFunction());
	/* json */
	exports->Set(NanNew("typeof"), NanNew <FunctionTemplate>(TypeOf)->GetFunction());
	exports->Set(NanNew("indent"), NanNew <FunctionTemplate>(JsonIndent)->GetFunction());
	exports->Set(NanNew("unindent"), NanNew <FunctionTemplate>(JsonUnindent)->GetFunction());
	/* json-path */
	exports->Set(NanNew("get"), NanNew <FunctionTemplate>(JsonGet)->GetFunction());
	exports->Set(NanNew("set"), NanNew <FunctionTemplate>(JsonSet)->GetFunction());
}

NODE_MODULE(sdb, Init)
