/* sdb-nan.js - pancake@nopcode.org // 2015 */

#include <nan.h>
#include <sdb.h>

using namespace v8;

class SdbObject : public node::ObjectWrap {
	public:
		static void Init(Handle<Object> exports);
		static NAN_METHOD(New);
		static NAN_METHOD(Set);
		static NAN_METHOD(Get);
		static NAN_METHOD(GetVersion);
		SdbObject() {
			this->obj = sdb_new0();
		}
		~SdbObject() {
			sdb_free (this->obj);
		}

	private:
		Sdb *obj;
};

NAN_METHOD(SdbObject::Set) {
	int len = args.Length();
	if (len == 2) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		if (!args[1]->IsString()) {
			NanThrowTypeError ("Second argument must be a string");
		}
		NanUtf8String k (args[0]);
		NanUtf8String v (args[1]);
		Sdb *sdb = ObjectWrap::Unwrap<SdbObject>(args.This())->obj;
		sdb_set (sdb, *k, *v, 0);
	} else {
		NanThrowTypeError ("Sdb.Set Invalid arguments");
	}
}

NAN_METHOD(SdbObject::GetVersion) {
	NanReturnValue(NanNew(SDB_VERSION));
}

NAN_METHOD(SdbObject::Get) {
	int len = args.Length();
	if (len == 1) {
		if (!args[0]->IsString()) {
			NanThrowTypeError ("First argument must be a string");
		}
		NanUtf8String k (args[0]);
		Sdb *sdb = ObjectWrap::Unwrap<SdbObject>(args.This())->obj;
		const char *v = sdb_const_get (sdb, *k, NULL);
		if (v != NULL) {
			NanReturnValue(NanNew(v));
		}
	}
	NanReturnUndefined();
}

void SdbObject::Init(Handle<Object> exports) {
	Local<v8::String> name = NanNew("SdbObject");
	Local<v8::FunctionTemplate> ft = NanNew<FunctionTemplate>(New);
	ft->SetClassName(name);
	ft->InstanceTemplate()->SetInternalFieldCount(1);
#if 0
	auto data = Handle<Value>();
	auto ift = ft->InstanceTemplate();
	auto signature = AccessorSignature::New(isolate, ft);
	ift->SetAccessor(String::NewFromUtf8(isolate, "id"),
		GetId, 0, data, DEFAULT, ReadOnly, signature);
#endif
	NODE_SET_PROTOTYPE_METHOD(ft, "set", Set);
	NODE_SET_PROTOTYPE_METHOD(ft, "get", Get);

	exports->Set(name, ft->GetFunction());
	exports->Set(NanNew("version"), NanNew(SDB_VERSION));
}

NAN_METHOD(SdbObject::New) {
	NanScope();
	v8::Isolate *isolate = args.GetIsolate();
	if (!args.IsConstructCall()) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate,
			"SdbObject requires new")));
		return;
	}
	SdbObject *wrapper = new SdbObject();
	if (wrapper->obj == NULL) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate,
			"sdb_new0 returns null")));
		return;
	}
	Local<v8::Object> obj = args.This();
	wrapper->Wrap(obj);
	NanReturnValue(obj);
}

void Init(Handle<Object> exports) {
	SdbObject::Init(exports);
}

NODE_MODULE(sdb, Init)
