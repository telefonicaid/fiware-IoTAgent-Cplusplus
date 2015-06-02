/** \file
 * \author John Bridgman
 * \brief 
 */

#include <Variant/ArgParse.h>
#include <Variant/Variant.h>
#include <Variant/Schema.h>
#include <iostream>

using namespace libvariant;
using namespace std;

void MoreOptions(ArgParse &opts) {
	opts.AddOption("/abc/a", 0, "abc-a", "libabc a").Default("a");
	opts.AddOption("/abc/b", 0, "abc-b", "libabc b").Default("b");
	opts.AddOption("/abc/c", 0, "abc-c", "libabc c");
	opts.AddGroup("abc", "libabc", "Options for libabc").Add("/abc/a").Add("/abc/b")
		.Add("/abc/c");
	opts.AddGroup("abc-exclusive").Add("/abc/b").Add("/abc/c").MutuallyExclusive();
}

int main(int argc, char **argv) {

	SerializeType stype = SERIALIZE_JSON;
	Variant params = Variant().Set("pretty", true);

	ArgParse opts(argv[0]);

	opts.SetDescription("An example application that uses ArgParse.");
	opts.SetEpilog("An epilog of the help text!");
	opts.AddConfigDefaults();

	opts.AddOption("foo-key", 'f', "foo", "What to foo with.").Action(ARGACTION_APPEND)
		.Type(ARGTYPE_FLOAT).Default(0.1234).Minimum(0);
	// Defines an option --foo or -f that takes a number argument, which is 0.1234
	// if not specified and has a minimum value of 0

	opts.AddOption("bar-key", 'b', "bar", "The bar.")
		.Default("frobnar!").MaxLength(100).Pattern("[A-Za-z]*!")
		.Env("BAR");
	// Defines an option --bar or -b that takes a string argument that
	// may be any alphastring ending in an ! up to 100 characters in length.

	opts.AddOption("blurt-key", 0, "blurt", "BLURT!")
		.AddChoice("furble").AddChoice("murble").AddChoice("durble")
		.Default("durble");
	// Defines an option  --blurt that can take the string values
	// 'furble', 'murble', or 'durble' and will error on any other.

	opts.AddFlag("recursive", 'r', "recurse", "Do operation recursively");
	// Add an option -r that sets true if present and false if not
	// Same as opts.AddOption("recursive, 'r', "recurse", "Do operation recursively")
	// 	.Action(ARGACTION_STORE_TRUE).Type(ARGTYPE_BOOL).Default(false);
	
	opts.AddOption("verbose", 'v', "verbose", "Increase verbosity one level.")
		.Action(ARGACTION_COUNT);

	opts.AddOption("quiet", 'q', "quiet", "Decrease verbosity one level.")
		.Action(ARGACTION_COUNT);

	MoreOptions(opts);

	opts.AddArgument("host", "Hostname").Type(ARGTYPE_STRING).Action(ARGACTION_STORE);
	opts.AddArgument("port", "Port number or name").Type(ARGTYPE_STRING).Action(ARGACTION_STORE);
	opts.AddArgument("XXX", "Message IDs").Type(ARGTYPE_INT).Action(ARGACTION_APPEND)
		.Minimum(0).MinArgs(1).MaxArgs(0);
	// Define that the program takes at least 3 positional arguments,
	// the first being a string which will be stored in host, a port name
	// which will be a string stored in port and a list of positive integers
	// which will be stored as a list in key XXX.

	// An example for a single positional optional argument (can  have no
	// required arguments after it):
	// opts.AddArgument("msgid", "Message ID").Type(ARGTYPE_INT).Default(10)
	// 	.Minimum(0).MinArgs(0);

	cout << "The description:\n";
	Serialize(cout.rdbuf(), opts.GetDescription(), stype, params);
	cout << endl;
	cout << "The generated schema for the parsed arguments:\n";
	Variant schema = opts.GenerateSchema();
	Serialize(cout.rdbuf(), schema, stype, params);
	cout << endl;

	opts.Validate();
	cout << "Now parsing the arguments..." << endl;
	Variant args = opts.Parse(argc, argv);

	int verbosity = 1 + args.Get("verbose", 0).As<int>() - args.Get("quiet", 0).As<int>();

	cout << "Using verbosity " << verbosity << endl;

	cout << "The parsed arguments then look like:\n";
	Serialize(cout.rdbuf(), args, stype, params);
	cout << endl;
	SchemaResult validate_result = SchemaValidate(schema, args);
	cout << "Does it validate? " << (validate_result.Error() ? "no" : "yes") << endl;
	if (validate_result.Error()) { cout << validate_result << endl; }
	return 0;
}
