/* node-sdb native module */

#include <string>
#include <node.h>
#include <v8.h>
#include <sdb.h>

using namespace node;
using namespace v8;

class NodeSdb : public ObjectWrap {
	public:
		Sdb *sdb;
		static Persistent<FunctionTemplate> constructor_template;
		static Persistent<FunctionTemplate> s_ct;

		NodeSdb(const char *file, int lock) {
			sdb = sdb_new (file, lock);
		}

		static Handle<Value> node_sdb_new(const Arguments& args) {
			HandleScope scope;

			Local<String> a = args[0]->ToString ();
			String::Utf8Value pa(a);
			NodeSdb *ns = new NodeSdb (*pa, 0);

			Local<Object> obj = args.This ();
			ns->Wrap (args.This());

			obj->Set (String::NewSymbol ("file"), String::New (*pa));
			NODE_SET_METHOD (obj, "set", node_sdb_set);
			NODE_SET_METHOD (obj, "get", node_sdb_get);
			//NODE_SET_METHOD (obj, "jsonSet", node_sdb_set);
			NODE_SET_METHOD (obj, "jsonGet", node_sdb_json_get);
			return scope.Close (obj);
		}

		static Handle<Value> New(const Arguments& args) {
			return node_sdb_new (args);
		}

		static Handle<Value> node_sdb_json_get(const Arguments& args) {
			HandleScope scope;
			Local<String> a = args[0]->ToString (); String::Utf8Value pa(a);
			Local<String> b = args[1]->ToString (); String::Utf8Value pb(b);
			NodeSdb *ns = Unwrap<NodeSdb>(args.This());
			char *s = sdb_json_get (ns->sdb, *pa, *pb, NULL);
			if (s) return scope.Close (String::New (s));
			return scope.Close (Null ());
		}

		static Handle<Value> node_sdb_get(const Arguments& args) {
			HandleScope scope;
			Local<String> a = args[0]->ToString ();
			String::Utf8Value pa(a);
			NodeSdb *ns = Unwrap<NodeSdb>(args.This());
			char *s = sdb_get (ns->sdb, *pa, NULL);
			if (s) return scope.Close (String::New (s));
			return scope.Close (Null ());
		}

		static Handle<Value> node_sdb_set(const Arguments& args) {
			HandleScope scope;
			Local<String> a = args[0]->ToString ();
			Local<String> b = args[1]->ToString ();
			String::Utf8Value pa(a);
			String::Utf8Value pb(b);
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
		printf ("cinit\n");
		NodeSdb::Initialize(target);
	}

	NODE_MODULE (sdb, init);
}
