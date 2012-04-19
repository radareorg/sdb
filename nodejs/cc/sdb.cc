/* node-sdb native module */
/* author: pancake */

#include <string>
#include <node.h>
#include <v8.h>
#include <sdb.h>

using namespace node;
using namespace v8;

class NodeSdb : public ObjectWrap {
	public:
#define getSdb() Unwrap<NodeSdb>(args.This())->sdb
		Sdb *sdb;

		NodeSdb(const char *file, int lock) {
			sdb = sdb_new (file, lock);
		}

		static Handle<Value> node_sdb_sync(const Arguments& args) {
			HandleScope scope;
			return scope.Close (sdb_sync (getSdb ())? True (): False ());
		}

		static Handle<Value> node_sdb_inc(const Arguments& args) {
			HandleScope scope;
			String::Utf8Value pa (args[0]->ToString ());
			Local<Integer> pb (args[1]->ToInteger ());
			// TODO: use pb!
			int foo = sdb_inc (getSdb(), *pa, 1, 0);
			return scope.Close (foo? True (): False ());
		}

		static Handle<Value> node_sdb_exists(const Arguments& args) {
			HandleScope scope;
			Local<String> a = args[0]->ToString (); String::Utf8Value pa(a);
			int foo = sdb_exists (getSdb(), *pa);
			return scope.Close (foo? True (): False ());
		}

		static Handle<Value> node_sdb_new(const Arguments& args) {
			HandleScope scope;

			Local<String> a = args[0]->ToString ();
			String::Utf8Value pa(a);
			NodeSdb *ns = new NodeSdb (*pa, 0);

			Local<Object> obj = args.This ();
			ns->Wrap (args.This());

			obj->Set (String::NewSymbol ("file"), String::New (*pa));
			NODE_SET_METHOD (obj, "exists", node_sdb_exists);
			NODE_SET_METHOD (obj, "sync", node_sdb_sync);
			NODE_SET_METHOD (obj, "set", node_sdb_set);
			NODE_SET_METHOD (obj, "get", node_sdb_get);
			NODE_SET_METHOD (obj, "jsonSet", node_sdb_json_set);
			NODE_SET_METHOD (obj, "jsonGet", node_sdb_json_get);
			//
			NODE_SET_METHOD (obj, "inc", node_sdb_inc);
		//	NODE_SET_METHOD (obj, "dec", node_sdb_dec);
		//	NODE_SET_METHOD (obj, "add", node_sdb_add);
			return scope.Close (obj);
		}

		static Handle<Value> New(const Arguments& args) {
			return node_sdb_new (args);
		}

		static Handle<Value> node_sdb_json_get(const Arguments& args) {
			HandleScope scope;
			
			Local<String> a = args[0]->ToString (); String::Utf8Value pa(a);
			Local<String> b = args[1]->ToString (); String::Utf8Value pb(b);
			char *s = sdb_json_get (getSdb(), *pa, *pb, NULL);
			if (s) return scope.Close (String::New (s));
			return scope.Close (Null ());
		}

		static Handle<Value> node_sdb_json_set(const Arguments& args) {
			HandleScope scope;
			String::Utf8Value pa (args[0]->ToString ());
			String::Utf8Value pb (args[1]->ToString ());
			String::Utf8Value pc (args[2]->ToString ());
			// CAS Local<String> c = args[2]->ToInteger(); 
			int ret = sdb_json_set (getSdb (), *pa, *pb, *pc, NULL);
			if (ret) return scope.Close (Integer::New (ret));
			return scope.Close (Null ());
		}

		static Handle<Value> node_sdb_get(const Arguments& args) {
			HandleScope scope;
			String::Utf8Value pa (args[0]->ToString ());
			char *s = sdb_get (getSdb (), *pa, NULL);
			if (s) return scope.Close (String::New (s));
			return scope.Close (Null ());
		}

		static Handle<Value> node_sdb_set(const Arguments& args) {
			HandleScope scope;
			String::Utf8Value pa (args[0]->ToString ());
			String::Utf8Value pb (args[1]->ToString ());
			NodeSdb *s = ObjectWrap::Unwrap<NodeSdb>(args.This());
			int ret = -1;
			if (s && s->sdb)
				ret = sdb_set (s->sdb, *pa, *pb, 0); 
			Local<Integer> obj = Integer::New(ret);
			return scope.Close (obj);
		}

		static void NODE_EXTERN Initialize(Handle<Object> target) {
			HandleScope scope;

			target->Set (String::NewSymbol ("version"), String::New (SDB_VERSION));

			Local<FunctionTemplate> t = FunctionTemplate::New(New);
			t->InstanceTemplate()->SetInternalFieldCount(1);
			target->Set(String::NewSymbol ("open"), t->GetFunction());
		}
};

extern "C" {
	void init(Handle<Object> target) {
		NodeSdb::Initialize(target);
	}

	NODE_MODULE (sdb, init);
}
