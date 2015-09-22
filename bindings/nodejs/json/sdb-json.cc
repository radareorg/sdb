/* sdb-nan.js - pancake@nopcode.org // 2015 */

#include <node.h>
#include <nan.h>
#include <sdb.h>

using namespace v8;

#define NanReturnThis() info.GetReturnValue().Set(info.This())
#define NanReturnValue(v) info.GetReturnValue().Set(v)

extern "C" {
	char *api_json_get (const char *s, const char *p);
	char *api_json_set (const char *s, const char *p, const char *v);
}

NAN_METHOD(GetVersion) {
	NanReturnValue(Nan::New<v8::String>(SDB_VERSION).ToLocalChecked());
	//info.GetReturnValue().Set(Nan::New<v8::String>(SDB_VERSION).ToLocalChecked());
}

NAN_METHOD(Encode) {
	int len = info.Length();
	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String k (info[0]);
		char *str = sdb_encode ((const ut8*)*k, -1);
		if (str) {
			auto v = Nan::New<v8::String>(str);
			free (str);
			NanReturnValue(v.ToLocalChecked());
		}
	}
}

NAN_METHOD(JsonIndent) {
	int len = info.Length();
	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String k (info[0]);
		char *res = sdb_json_indent (*k);
		auto v = Nan::New<v8::String>(res);
		free (res);
		NanReturnValue(v.ToLocalChecked());
	}
}

NAN_METHOD(JsonUnindent) {
	int len = info.Length();
	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String k (info[0]);
		char *res = sdb_json_unindent (*k);
		auto v = Nan::New<v8::String>(res);
		free (res);
		NanReturnValue(v.ToLocalChecked());
	}
}

NAN_METHOD(TypeOf) {
	int len = info.Length();
	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String k (info[0]);
		NanReturnValue(Nan::New<v8::String>(sdb_type (*k)).ToLocalChecked());
	}
}

NAN_METHOD(Decode) {
	int len = info.Length();
	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String k (info[0]);
		char *str = (char *)sdb_decode (*k, NULL);
		if (str) {
			auto v = Nan::New<v8::String>(str);
			free (str);
			NanReturnValue(v.ToLocalChecked());
		}
	}
}

NAN_METHOD(JsonGet) {
	int len = info.Length();
	if (len == 2) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String k (info[0]);
		Nan::Utf8String v (info[1]);
		char *str = api_json_get (*k, *v);
		if (str) {
			auto v = Nan::New<v8::String>(str);
			free (str);
			NanReturnValue(v.ToLocalChecked());
		}
	} else {
		Nan::ThrowTypeError ("wrong number of arguments");
	}
}

NAN_METHOD(JsonSet) {
	int len = info.Length();
	if (len == 3) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		if (!info[1]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String p (info[0]);
		Nan::Utf8String k (info[1]);
		if (info[1]->IsString()) {
			Nan::Utf8String v (info[2]);
			char *str = api_json_set (*p, *k, *v);
			if (str) {
				auto v = Nan::New<v8::String>(str);
				free (str);
				NanReturnValue(v.ToLocalChecked());
			}
		} else {
			Nan::ThrowTypeError ("TODO: support more types");
		}
	} else {
		Nan::ThrowTypeError ("wrong number of arguments");
	}
}

void Init(Handle<Object> exports, Handle<Value> module) {
#define exportString(a,b) exports->Set(Nan::New<v8::String>(a).ToLocalChecked(),Nan::New<v8::String>(b).ToLocalChecked())
#define exportFunction(a,b) exports->Set(Nan::New<v8::String>(a).ToLocalChecked(),Nan::New <FunctionTemplate>(b)->GetFunction())
	/* generic */
	exportString("version", SDB_VERSION);
	/* base64 */
	exportFunction("encode", Encode);
	exportFunction("decode", Decode);
	/* json */
	exportFunction("typeof", TypeOf);
	exportFunction("indent", JsonIndent);
	exportFunction("unindent", JsonUnindent);
	/* json-path */
	exportFunction("get", JsonGet);
	exportFunction("set", JsonSet);
}

NODE_MODULE(sdb, Init)
