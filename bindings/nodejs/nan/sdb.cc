/* sdb-nan.js - pancake@nopcode.org // 2015 */

#include <node.h>
#include <nan.h>
#include <sdb.h>

#define MyNanReturnThis() info.GetReturnValue().Set(info.This())

using namespace v8;

class Database : public Nan::ObjectWrap {
	public:
		static void Init(Handle<Object> exports);
		static NAN_METHOD(Open);
		static NAN_METHOD(Close);
		static NAN_METHOD(Ns);
		static NAN_METHOD(New);
		static NAN_METHOD(Add);
		static NAN_METHOD(Set);
		static NAN_METHOD(UnSet);
		static NAN_METHOD(UnSetLike);
		static NAN_METHOD(Drain);
		static NAN_METHOD(Type);
		static NAN_METHOD(Expire);
		static NAN_METHOD(Reset);
		static NAN_METHOD(Sync);
		static NAN_METHOD(KeysOnDisk);
		static NAN_METHOD(KeysOnMemory);
		static NAN_METHOD(Query);
		static NAN_METHOD(Like);
		static NAN_METHOD(UnLink);
		static NAN_METHOD(Get);
		static NAN_METHOD(GetVersion);
		static NAN_METHOD(Exists);
		Database() {
			this->obj = sdb_new0 ();
		}
		Database(const char *file) {
			this->obj = sdb_new (NULL, file, 0);
		}
		Database(Sdb *db) {
			this->obj = db;
		}
		~Database() {
			sdb_free (this->obj);
		}
	private:
		Sdb *obj;
};

NAN_METHOD(Database::Like) {
	Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
	char **res = NULL;

	Nan::HandleScope scope;

	switch (info.Length()) {
	case 1:
		if (info[0]->IsString()) {
			Nan::Utf8String k (info[0]);
			res = sdb_like(sdb, *k, NULL, NULL);
		} else {
			Nan::ThrowTypeError ("string expected");
		}
		break;
	case 2:
		char *key = NULL, *val = NULL;
		// TODO: fail if type is bool, object, ...
		if (info[0]->IsString()) {
			Nan::Utf8String k (info[0]);
			key = *k;
		}
		if (info[1]->IsString()) {
			Nan::Utf8String v (info[1]);
			val = *v;
		}
		res = sdb_like (sdb, key, val, NULL);
		break;
	}
	if (res) {
		Local<Object> obj = Nan::New<Object>();
		for (int i = 0; res[i]; i+=2) {
			obj->Set(
				Nan::New<v8::String>(res[i]).ToLocalChecked(),
				Nan::New<v8::String>(res[i+1]).ToLocalChecked());
		}
		info.GetReturnValue().Set(obj);
	}
}

NAN_METHOD(Database::UnSetLike) {
	int len = info.Length();

	Nan::HandleScope scope;

	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("string expected");
		}
		Nan::Utf8String k (info[0]);
		Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
		sdb_unset_like (sdb, *k);
	}
}

NAN_METHOD(Database::Open) {
	int fd = -1;
	int len = info.Length();

	Nan::HandleScope scope;

	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("string expected");
		}
		Nan::Utf8String k (info[0]);
		Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
		fd = sdb_open (sdb, *k);
	}
	info.GetReturnValue().Set(Nan::New (fd != -1));
}

NAN_METHOD(Database::Close) {
	/* arguments ignored */
	Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
	sdb_close (sdb);
}

NAN_METHOD(Database::Type) {
	int len = info.Length();

	Nan::HandleScope scope;

	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("string expected");
		}
		Nan::Utf8String k (info[0]);
		Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
		const char *t = sdb_type (sdb_const_get (sdb, *k, 0));
		info.GetReturnValue().Set(Nan::New<v8::String>(t).ToLocalChecked());
	}
}

NAN_METHOD(Database::UnSet) {
	int len = info.Length();
	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("string expected");
		}
		Nan::Utf8String k (info[0]);
		Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
		sdb_unset (sdb, *k, 0);
	}
}

NAN_METHOD(Database::Exists) {
	bool ret = false;
	int len = info.Length();

	Nan::HandleScope scope;

	if (len == 1) {
		if (info[0]->IsString()) {
			Nan::Utf8String k (info[0]);
			Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
			int ret = sdb_exists (sdb, *k);
			info.GetReturnValue().Set(Nan::New ((bool)ret));
			// sdb_num_exists
		} else {
			Nan::ThrowTypeError ("string key expected");
		}
	}
	info.GetReturnValue().Set(Nan::New(ret));
}

NAN_METHOD(Database::Set) {
	int len = info.Length();

	Nan::HandleScope scope;

	if (len == 2) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
		Nan::Utf8String k (info[0]);
		if (info[1]->IsBoolean()) {
			ut64 v = info[1]->Uint32Value();
			(void)sdb_bool_set (sdb, *k, v, 0);
		} else if (info[1]->IsNumber()) {
			ut64 v = info[1]->Uint32Value();
			(void)sdb_num_set (sdb, *k, v, 0);
		} else if (info[1]->IsString()) {
			Nan::Utf8String v (info[1]);
			(void)sdb_set (sdb, *k, *v, 0);
		} else {
			Nan::ThrowTypeError ("Second argument must be a string");
		}
	} else {
		Nan::ThrowTypeError ("Sdb.Set Invalid arguments");
	}
	////MyNanReturnThis();
	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Database::Ns) {
	int len = info.Length();

	Nan::HandleScope scope;

	if (len == 1) {
		Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("namespace must be a string");
		}
		Nan::Utf8String k (info[0]);
		Sdb *db = sdb_ns (sdb, *k, 0);
		Database *so = new Database(db);
		info.GetReturnValue().Set(Nan::New (so));
	} else if (len == 2) {
		Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("namespace must be a string");
		}
		Nan::Utf8String k (info[0]);
		ut32 v = info[1]->Uint32Value();
		Sdb *db = sdb_ns (sdb, *k, v);
		Database *so = new Database(db);
		info.GetReturnValue().Set(Nan::New (so));
	} else {
		/* return current namespace */
	}
}

#if 0
NAN_METHOD(Database::Open) {
	/* TODO: call constructor */
}
#endif

NAN_METHOD(Database::Add) {
	int ret = 0;
	int len = info.Length();

	Nan::HandleScope scope;

	if (len == 2) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		} else if (info[1]->IsNumber()) {
			Nan::Utf8String k (info[0]);
			ut64 v = info[1]->Uint32Value();
			Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
			ret = sdb_num_add (sdb, *k, v, 0);
		} else if (info[1]->IsString()) {
			Nan::Utf8String k (info[0]);
			Nan::Utf8String v (info[1]);
			Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
			ret = sdb_add (sdb, *k, *v, 0);
		} else {
			Nan::ThrowTypeError ("Second argument must be a string");
		}
	} else {
		Nan::ThrowTypeError ("Sdb.Set Invalid arguments");
	}
	if (ret == 0) {
		info.GetReturnValue().Set(Nan::New((bool)ret));
	} else {
		info.GetReturnValue().Set(info.This());
	}
}

NAN_METHOD(Database::GetVersion) {
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New(SDB_VERSION).ToLocalChecked());
}

NAN_METHOD(Database::Sync) {
	Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
	bool v = sdb_sync (sdb);
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New(v));
}

NAN_METHOD(Database::Drain) {
	int len = info.Length();
	Nan::HandleScope scope;
	if (len == 1) {
		//Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
		//Sdb *sdb2 = ((Database*)(info[0]->ToObject()->Get(0)))->obj;
		//Sdb *sdb2 = Nan::ObjectWrap::Unwrap<Database>(arg)->obj;
		//sdb_drain (sdb, sdb2);
		info.GetReturnValue().Set(info.This());
	} else {
		Nan::ThrowTypeError ("Missing destination database");
	}
}

NAN_METHOD(Database::Reset) {
	Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
	sdb_reset (sdb);
	Nan::HandleScope scope;
	MyNanReturnThis();
}

NAN_METHOD(Database::UnLink) {
	Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
	bool v = sdb_unlink(sdb);
	Nan::HandleScope scope;
	info.GetReturnValue().Set(Nan::New(v));
}

NAN_METHOD(Database::Get) {
	int len = info.Length();

	Nan::HandleScope scope;

	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String k (info[0]);
		Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
		const char *v = sdb_const_get (sdb, *k, NULL);
		if (v != NULL) {
			info.GetReturnValue().Set(Nan::New(v).ToLocalChecked());
		}
	}
}

NAN_METHOD(Encode) {
	int len = info.Length();

	Nan::HandleScope scope;

	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String k (info[0]);
		char *str = sdb_encode ((const ut8*)*k, -1);
		if (str) {
			Nan::MaybeLocal<String> v = Nan::New<v8::String>(str);
			free (str);
			info.GetReturnValue().Set(v.ToLocalChecked());
		}
	}
}

NAN_METHOD(JsonIndent) {
	int len = info.Length();

	Nan::HandleScope scope;

	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String k (info[0]);
		char *res = sdb_json_indent (*k);
		Nan::MaybeLocal<String> v = Nan::New<String>(res);
		free (res);
		info.GetReturnValue().Set(v.ToLocalChecked());
	}
}

NAN_METHOD(JsonUnindent) {
	int len = info.Length();

	Nan::HandleScope scope;

	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String k (info[0]);
		char *res = sdb_json_unindent (*k);
		Nan::MaybeLocal<v8::String> v = Nan::New<v8::String>(res);
		free (res);
		info.GetReturnValue().Set(v.ToLocalChecked());
	}
}

NAN_METHOD(TypeOf) {
	int len = info.Length();

	Nan::HandleScope scope;

	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String k (info[0]);
		info.GetReturnValue().Set(Nan::New (sdb_type (*k)).ToLocalChecked());
	}
}

NAN_METHOD(Decode) {
	int len = info.Length();

	Nan::HandleScope scope;

	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String k (info[0]);
		char *str = (char *)sdb_decode (*k, NULL);
		if (str) {
			Nan::MaybeLocal<v8::String> v = Nan::New<v8::String>(str);
			free (str);
			info.GetReturnValue().Set(v.ToLocalChecked());
		}
	}
}

NAN_METHOD(Database::Query) {
	int len = info.Length();

	Nan::HandleScope scope;

	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String k (info[0]);
		Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
		char *res = sdb_querys (sdb, *k, -1, NULL);
		if (res != NULL) {
			Nan::MaybeLocal<v8::String> v = Nan::New<v8::String>(res);
			free (res);
			info.GetReturnValue().Set(v.ToLocalChecked());
		}
	}
}

NAN_METHOD(Database::KeysOnDisk) {
	Nan::HandleScope scope;
	Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
	ut32 count;
	if (sdb_stats (sdb, &count, NULL)) {
		Local<Uint32> v = Nan::New<Uint32>(count);
		info.GetReturnValue().Set(v);
	} else {
		info.GetReturnValue().Set(0);
	}
}

NAN_METHOD(Database::KeysOnMemory) {
	Nan::HandleScope scope;
	Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;
	ut32 count;
	if (sdb_stats (sdb, NULL, &count)) {
		Local<Uint32> v = Nan::New(count);
		info.GetReturnValue().Set(v);
	} else {
		info.GetReturnValue().Set(0);
	}
}

NAN_METHOD(Database::Expire) {
	ut32 v = 0; // should be 64 bit!
	Sdb *sdb = Nan::ObjectWrap::Unwrap<Database>(info.This())->obj;

	Nan::HandleScope scope;

	switch (info.Length()) {
	case 1:
		if (info[0]->IsString()) {
			Nan::Utf8String k (info[0]);
			v = sdb_expire_get (sdb, *k, NULL);
		} else {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		break;
	case 2:
		if (info[0]->IsString()) {
			Nan::Utf8String k (info[0]);
			if (info[1]->IsNumber()) {
				ut64 n = (ut64)info[1]->Uint32Value();
				v = sdb_expire_set (sdb, *k, n, 0);
			} else {
				Nan::ThrowTypeError ("2nd arg expects a number");
			}
		} else {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		break;
	default:
		Nan::ThrowTypeError ("Missing parameters");
		break;
	}
	info.GetReturnValue().Set(Nan::New(v));
}

void Database::Init(Handle<Object> exports) {
	Local<v8::String> name = Nan::New("Database").ToLocalChecked();
	Local<v8::FunctionTemplate> ft = Nan::New<FunctionTemplate>(New);
	ft->SetClassName(name);
	ft->InstanceTemplate()->SetInternalFieldCount(1);
#if 0
	auto data = Handle<Value>();
	auto ift = ft->InstanceTemplate();
	auto signature = AccessorSignature::New(isolate, ft);
	Nan::SetAccessor(ift, String::NewFromUtf8(isolate, "id"),
			GetId, 0, data, DEFAULT, ReadOnly, signature);
#endif
	Nan::SetPrototypeMethod(ft, "add", Add);
	Nan::SetPrototypeMethod(ft, "set", Set);
	//Nan::SetPrototypeMethod(ft, "get_length", GetLength);
	Nan::SetPrototypeMethod(ft, "unset", UnSet);
	Nan::SetPrototypeMethod(ft, "unset_like", UnSetLike);
	Nan::SetPrototypeMethod(ft, "reset", Reset);
	Nan::SetPrototypeMethod(ft, "keys_on_disk", KeysOnDisk);
	Nan::SetPrototypeMethod(ft, "keys_on_memory", KeysOnMemory);
	Nan::SetPrototypeMethod(ft, "drain", Drain);
	Nan::SetPrototypeMethod(ft, "unlink", UnLink);
	Nan::SetPrototypeMethod(ft, "get", Get);
	Nan::SetPrototypeMethod(ft, "type", Type);
	Nan::SetPrototypeMethod(ft, "sync", Sync);
	Nan::SetPrototypeMethod(ft, "expire", Expire);
	Nan::SetPrototypeMethod(ft, "query", Query);
	Nan::SetPrototypeMethod(ft, "exists", Exists);
	Nan::SetPrototypeMethod(ft, "ns", Ns);
	Nan::SetPrototypeMethod(ft, "like", Like);
	Nan::SetPrototypeMethod(ft, "open", Open);
	Nan::SetPrototypeMethod(ft, "close", Close);
	/* numeric stuff */
#if 0
	Nan::SetPrototypeMethod(ft, "inc", Inc);
	Nan::SetPrototypeMethod(ft, "dec", Dec);
	Nan::SetPrototypeMethod(ft, "min", Min);
	Nan::SetPrototypeMethod(ft, "max", Max);
#endif

#if 0
	/* ARRAY */
	var foo = db.array_get("foo", idx);
	Nan::SetPrototypeMethod(ft, "array_get", ArrayGet);
	Nan::SetPrototypeMethod(ft, "array_add", ArrayAdd);
	Nan::SetPrototypeMethod(ft, "array_add_sorted", ArrayAddSorted);
	Nan::SetPrototypeMethod(ft, "array_set", ArraySet);
	Nan::SetPrototypeMethod(ft, "array_unset", ArrayUnset);
	Nan::SetPrototypeMethod(ft, "array_remove", ArrayRemove);
	Nan::SetPrototypeMethod(ft, "array_delete", ArrayDelete);
	Nan::SetPrototypeMethod(ft, "array_contains", ArrayContains);
	Nan::SetPrototypeMethod(ft, "array_size", ArraySize);
	Nan::SetPrototypeMethod(ft, "array_length", ArrayLength);
	Nan::SetPrototypeMethod(ft, "array_indexOf", ArrayIndexOf);
	Nan::SetPrototypeMethod(ft, "array_insert", ArrayInsert);
	Nan::SetPrototypeMethod(ft, "array_push", ArrayPush);
	Nan::SetPrototypeMethod(ft, "array_pop", ArrayPop);
	Nan::SetPrototypeMethod(ft, "array_sort", ArraySort);
	Nan::SetPrototypeMethod(ft, "array_sort_num", ArraySortNum);
#endif
#if 0
	/* NUM */
	num_exists (isNumber())
	num_get
	num_add
	num_set
	num_inc
	num_dec
	num_min
	bool_set
	bool_get
#endif
#if 0
	/* JSON */
	json_get
	json_num_inc
#endif
	exports->Set(name, ft->GetFunction());
	exports->Set(Nan::New("version").ToLocalChecked(), Nan::New(SDB_VERSION).ToLocalChecked());
	exports->Set(Nan::New("encode").ToLocalChecked(), Nan::New <FunctionTemplate>(Encode)->GetFunction());
	exports->Set(Nan::New("decode").ToLocalChecked(), Nan::New <FunctionTemplate>(Decode)->GetFunction());
	exports->Set(Nan::New("typeof").ToLocalChecked(), Nan::New <FunctionTemplate>(TypeOf)->GetFunction());

	/* TODO: implement under the sdb.json object */
	exports->Set(Nan::New("json_indent").ToLocalChecked(), Nan::New <FunctionTemplate>(JsonIndent)->GetFunction());
	exports->Set(Nan::New("json_unindent").ToLocalChecked(), Nan::New <FunctionTemplate>(JsonUnindent)->GetFunction());
}

NAN_METHOD(Database::New) {
	Nan::HandleScope scope;

	if (!info.IsConstructCall()) {
		Nan::ThrowTypeError ("Database requires new");
	}

	Database *wrapper;
	int len = info.Length();
	if (len == 1) {
		if (!info[0]->IsString()) {
			Nan::ThrowTypeError ("First argument must be a string");
		}
		Nan::Utf8String k (info[0]);
		wrapper = new Database(*k);
	} else {
		wrapper = new Database();
	}

	if (wrapper->obj == NULL) {
		Nan::ThrowTypeError ("sdb_new0 returns null");
	}
	Local<v8::Object> obj = info.This();
	wrapper->Wrap(obj);
	info.GetReturnValue().Set(obj);
}

#if 0
NAN_METHOD(NewDatabase) {
	Database *db = new Database();
	Local<Value> v = obj.CallAsConstructor(0, Handle<Value>[]);
	info.GetReturnValue().Set(v);
}
#endif

void Init(Handle<Object> exports, Handle<Value> module) {
#if 0
	module.As<Object>()->Set(Nan::New("exports").ToLocalChecked(),
			Nan::New <FunctionTemplate>(NewDatabase)->GetFunction());
#endif
	Database::Init(exports);
}

NODE_MODULE(sdb, Init)
