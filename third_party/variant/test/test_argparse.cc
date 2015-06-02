/** \file
 * \author John Bridgman
 * \brief 
 */

#include <Variant/ArgParse.h>
#include <Variant/Schema.h>
#include <iostream>
#include <unistd.h>
#include <getopt.h>

using namespace libvariant;
using namespace std;

SerializeType stype = SERIALIZE_JSON;
Variant sparams = Variant::MapType;

Variant OtherConfig() {
	ArgParse args;
	args.AddOption("adv-option1", 's', "adv1", "adv1").Advanced(true);
	args.AddOption("adv-option2", 0, "adv2", "adv2").Group("adv");
	args.AddGroup("adv", "adv group").Advanced(true);
	return args.GetDescription();
}

int Compare(Variant exp, Variant opts) {
	for (Variant::MapIterator ci(exp.MapBegin()), ce(exp.MapEnd()); ci != ce; ++ci) {
		if (!opts.Contains(ci->first)) {
			cout << "Missing key " << ci->first << endl;
			return 1;
		}
		if (ci->second.IsMap()) {
			if (Compare(ci->second, opts[ci->first])) {
				cout << "In sub map: \"" << ci->first << "\"\n";
			   	return 1;
		   	}
		} else if (opts[ci->first] != ci->second) {
			cout << "Value at key \"" << ci->first << "\" differs\n";
			cout << "Got:\n";
			Serialize(cout.rdbuf(), opts[ci->first], stype, sparams);
			cout << "\nExpected:\n";
			Serialize(cout.rdbuf(), ci->second, stype, sparams);
			cout << endl;
			return 1;
		}
	}
	return 0;
}

int main(int argc, char **argv) {
	sparams["pretty"] = true;

	ArgParse args(*argv);
	// store -- store the value into the key (default)
	args.AddOption("store", 's', "store", "Test store number").Action(ARGACTION_STORE)
		.Type(ARGTYPE_FLOAT).Env("TEST_STORE").Minimum(0).Maximum(1)
		.Default(0.54321);
	args.AddOption("store_bool", 0, "store_bool", "Test store boolean").Action(ARGACTION_STORE)
		.Type(ARGTYPE_BOOL).Env("TEST_STORE_BOOLEAN");
	args.AddOption("store_int", 0, "store_int", "Test store int").Action(ARGACTION_STORE)
		.Type(ARGTYPE_INT).Minimum(-10).Maximum(10).Default(0);
	args.AddOption("store_string", 0, "store_string", "Test store string").Action(ARGACTION_STORE)
		.Type(ARGTYPE_STRING).MinLength(4).MaxLength(10).Pattern("^[0-9a-zA-Z]*$")
		.Default("abc123");
	try {
		args.AddOption("store", 0, "blahblah", "");
		cerr << "Duplicate path successfully added." << endl;
		return 1;
	} catch (const std::runtime_error&) {}
	try {
		args.AddOption("blahblah", 0, "store", "");
		cerr << "Duplicate long option successfully added." << endl;
		return 1;
	} catch (const std::runtime_error&) {}
	try {
		args.AddOption("blahblah", 's', "", "");
		cerr << "Duplicate short option successfully added." << endl;
		return 1;
	} catch (const std::runtime_error&) {}
	try {
		args.AddOption("store", 0, "", "");
		cerr << "Option with no short or long opt sucessfully added." << endl;
		return 1;
	} catch (const std::runtime_error&) {}
	try {
		//args.AddOption("abc", 0, "abc", "").Type("abc");
		args.AddOption("abc", 0, "abc", "").Type((ArgParseType)Variant::MapType);
		cerr << "Option with invalid type successfully added." << endl;
		return 1;
	} catch (const std::runtime_error&) {}

	//cerr << "Turning conflict resolution on" << endl;
	args.ConflictResolution(true);
	try {
		args.AddOption("blahblah", 's', "", "");
		cerr << "Duplicate short option with no long option succesfully added" << endl;
		return 1;
	} catch (const std::runtime_error&) {}
	try {
		args.AddOption("blahblah", 's', "blahblah", "");
	} catch (const std::runtime_error&) {
		cerr << "Failed to add duplicate short option with long option with conflict resolution on." << endl;
		return 1;
	}

	args.AddOption("store_undef", 0, "store_undef", "Test store").Action(ARGACTION_STORE);
	// store_const -- store const (see Const) into the key
	args.AddOption("store_const", 0, "store_const", "Test store const").Action(ARGACTION_STORE_CONST)
		.Const(123);
	// store_true -- store true into the key (i.e. no option argument)
	args.AddOption("store_true", 0, "store_true", "Test store_true").Action(ARGACTION_STORE_TRUE)
		.Default(false).Type(ARGTYPE_BOOL);
	// store_false -- store false into the key
	args.AddOption("store_false", 0, "store_false", "Test store_false").Action(ARGACTION_STORE_FALSE)
		.Default(true).Type(ARGTYPE_BOOL);
	// append -- make the value a list of instances of the option
	args.AddOption("append", 0, "append", "Test append").Action(ARGACTION_APPEND)
		.Type(ARGTYPE_FLOAT).Minimum(0).Maximum(1).MinArgs(1).MaxArgs(10);
	// count -- count the number of instances of this argument
	args.AddOption("count", 0, "count", "Test count").Action(ARGACTION_COUNT);
	// config -- load a file and merge it into the result (uses GuessFormat)
	args.AddOption("load", 0, "load", "Test config").Action(ARGACTION_CONFIG);

	args.AddOption("choice", 0, "choice", "Test choices").AddChoice("a").AddChoice("b");

	args.AddOption("abc/a", 0, "abc-a", "abc-a");
	args.AddOption("abc/b", 0, "abc-b", "abc-b");
	args.AddGroup("abc", "abc").Add("abc/a").Add("abc/b").MutuallyExclusive();
	args.AddOption("abc/c", 0, "abc-c", "Test group mutual exclusion with default").Default(123);
	args.AddOption("abc/d", 0, "abc-d", "Mutually exclusive wth abc-c");
	args.AddGroup("abc-m").Add("abc/c").Add("abc/d").MutuallyExclusive();

	args.AddArgument("comp", "Comparison file for options")
		.Action(ARGACTION_APPEND).Type(ARGTYPE_STRING).MinArgs(0);

	args.AddOption("shortonly", 'a', "", "short only option").Type(ARGTYPE_GUESS);

	args.AddOption("list[0]", 0, "list0", "List zero option").Title("ListOp1");
	args.AddOption("list[1]", 0, "list1", "List one option");
	args.AddOption("list[2]", 0, "list2", "List two option");

	args.Merge(OtherConfig());
	args.AddConfigDefaults();

	args.Validate();

	Variant opts = args.Parse(argc, argv);
	
	ArgParse arg2;
	arg2.Merge(args);

	Variant opt2 = arg2.Parse(argc, argv);
	if (opts != opt2) {
		cout << "Second parsing of option failed...\n";
		cout << "First pass gave:\n\n";
		Serialize(cout.rdbuf(), opts, stype, sparams);
		cout << "\n\nsecond pass gave:\n\n";
		Serialize(cout.rdbuf(), opt2, stype, sparams);
		cout << endl;
		return 1;
	}

	cout << "parsed arguments:\n\n";
	Serialize(cout.rdbuf(), opts, stype, sparams);
	cout << "\n\n";
	cout << "Validating...";
	SchemaResult valres = SchemaValidate(args.GenerateSchema(), opts);
	if (valres.Error()) {
		cout << endl << valres << endl << endl;
		return 1;
	} else { cout << " passed\n\n"; }

	Variant::ListIterator i(opts["comp"].ListBegin()),
		e(opts["comp"].ListEnd());
	while (i != e) {
		Variant comp = DeserializeGuessFile(i->AsString().c_str());
		if (Compare(comp, opts)) { return 1; }
		++i;
	}
	return 0;
}
