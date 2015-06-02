//=============================================================================
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \author John Bridgman
 * \brief Variant tool
 */

#include <Variant/Variant.h>
#include <Variant/Schema.h>
#include <Variant/SchemaLoader.h>
#include <Variant/ArgParse.h>
#include <Variant/Payload.h>
#include <Variant/Extensions.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>

using namespace std;
using namespace libvariant;

struct State {
	SerializeType stype;
	AdvSchemaLoader loader;
	Variant conf;
};

void Process(State &state, ostream &out, istream &in) {
	Variant data;
	SerializeType itype;
	if (state.conf.Get("guess-in", false).AsBool()) { itype = SERIALIZE_GUESS; }
	if (state.conf.Get("json-in", false).AsBool()) { itype = SERIALIZE_JSON; }
	if (state.conf.Get("yaml-in", false).AsBool()) { itype = SERIALIZE_YAML; }
	if (state.conf.Get("plist-in", false).AsBool()) { itype = SERIALIZE_XMLPLIST; }
	if (state.conf.Get("bundlehdr-in", false).AsBool()) { itype = SERIALIZE_BUNDLEHDR; }

	if (state.conf.Get("payload-in", false).AsBool()) {
		data = DeserializeWithPayloadFile(in.rdbuf(), itype);
	} else {
		data = DeserializeFile(in.rdbuf(), itype);
	}

	if (state.conf.GetPath("extension/flatten", false).AsBool()) {
		data = FlattenPath( data, state.conf.GetPath("extension/params") );
	}
	else if (state.conf.GetPath("extension/expand", false).AsBool()) {
		data = ExpandPath( data, state.conf.GetPath("extension/params") );
	}

	Variant conf = state.conf;
	if (conf.Contains("schemas")) {
		for (Variant::ListIterator i(conf["schemas"].ListBegin()), e(conf["schemas"].ListEnd()); i != e; ++i) {
			SchemaResult result = SchemaValidate(*i, data, &state.loader);
			if (result.Error()) {
				cerr << result << endl;
				exit(1);
			}
		}
	}
	if (conf.Contains("set")) {
		if (!conf.Contains("path")) {
			data = conf["set"];
		} else {
			data.SetPath(conf["path"].AsString(), conf["set"]);
		}
	} else {
		if (conf.Contains("path")) {
			data = data.GetPath(conf["path"].AsString());
		}
	}
	if (conf.Contains("list") && conf["list"].AsBool()) {
		for (Variant::MapIterator i(data.MapBegin()), e(data.MapEnd()); i != e; ++i) {
			out << i->first << endl;
		}
	} else if (conf.Contains("len") && conf["len"].AsBool()) {
		out << data.Size() << endl;
	} else {
		if (state.conf.Get("payload", false).AsBool()) {
			SerializeWithPayload(out.rdbuf(), data, state.stype, conf["sparams"]);
		} else {
			Serialize(out.rdbuf(), data, state.stype, conf["sparams"]);
			out << endl;
		}
	}
}

int main(int argc, char **argv) {
	ArgParse opts(*argv);
	opts.AddOption("schema_path", 0, "schema-path", "Add a search path for schemas.")
		.Action(ARGACTION_APPEND).Type(ARGTYPE_STRING);
	opts.AddOption("schemas", 0, "schema", "Add a schema uri to verify against.")
		.Action(ARGACTION_APPEND).Type(ARGTYPE_STRING);
	opts.AddOption("path", 0, "path", "Set a path name to operate on rather than the whole document.")
		.Type(ARGTYPE_STRING);

	opts.AddOption("set", 0, "set", "Specify the value to set at the acting path.");
	opts.AddOption("set-file", 0, "set-file", "Specify a file that should be read to set at the acting path.")
		.Type(ARGTYPE_STRING);
	opts.AddFlag("list", 0, "list", "Print a list of keys at the acting path.");
	opts.AddFlag("len", 0, "length", "Print the length of the object at the acting path.");
	opts.AddGroup("set").Add("set").Add("set-file").Add("list").Add("len").MutuallyExclusive();

	opts.AddOption("output", 'o', "out", "Specify a file to output rather than standard out.")
		.Type(ARGTYPE_STRING);

	opts.AddOption("guess-in", 'G', "guess", "Specify input format should be guessed (only guesses between json/yaml/plist)").Type(ARGTYPE_BOOL).Action(ARGACTION_STORE_TRUE)
		.Default(true);
	opts.AddOption("json-in", 'J', "json-in", "Specify input to be JSON").Type(ARGTYPE_BOOL).Action(ARGACTION_STORE_TRUE);
#ifdef ENABLE_YAML
	opts.AddOption("yaml-in", 'Y', "yaml-in", "Specify input to be YAML").Type(ARGTYPE_BOOL).Action(ARGACTION_STORE_TRUE).Group("type-in");
#endif
#ifdef ENABLE_XML
	opts.AddOption("plist-in", 'X', "xml-in", "Specify input to be XML-PLIST").Type(ARGTYPE_BOOL).Action(ARGACTION_STORE_TRUE).Group("type-in");
#endif
	opts.AddOption("bundlehdr-in", 'B', "bundlehdr-in", "Specify input to be bundle header").Type(ARGTYPE_BOOL).Action(ARGACTION_STORE_TRUE)
		.Group("type-in");
	opts.AddGroup("type-in").Add("guess-in").Add("json-in").MutuallyExclusive();

	opts.AddFlag("payload-in", 'P', "payload-in", "Run input through the WithPayload functions instead of regular");

	opts.AddOption("json", 'j', "json", "Specify output to be JSON").Default(true).Type(ARGTYPE_BOOL).Action(ARGACTION_STORE_TRUE);
#ifdef ENABLE_YAML
	opts.AddOption("yaml", 'y', "yaml", "Specify output to be YAML").Type(ARGTYPE_BOOL).Action(ARGACTION_STORE_TRUE).Group("type");
#endif
#ifdef ENABLE_XML
	opts.AddOption("plist", 'x', "xml", "Specify output to be XML-PLIST").Type(ARGTYPE_BOOL).Action(ARGACTION_STORE_TRUE).Group("type");
#endif
	opts.AddOption("bundlehdr", 'b', "bundlehdr", "Specify output to be bundle header").Type(ARGTYPE_BOOL).Action(ARGACTION_STORE_TRUE)
		.Group("type");
	opts.AddGroup("type").Add("json").MutuallyExclusive();

	opts.AddFlag("payload", 'p', "payload", "Run output through the WithPayload functions instead of regular");

	opts.AddGroup("format", "Format specifiers");
	opts.AddOption("sparams/pretty", 0, "pretty", "Pretty print.").Default(true).Type(ARGTYPE_BOOL).Group("format");
	opts.AddOption("sparams/indent", 0, "indent", "Set indent.").Type(ARGTYPE_INT).Minimum(0).Group("format");
	opts.AddOption("sparams/precision", 0, "precision", "Floating point precision.").Type(ARGTYPE_INT).Minimum(0).Group("format");

	opts.AddOption("sparams/width", 0, "width", "Set prefered line width (YAML only)").Type(ARGTYPE_INT).Minimum(0).Group("format");

	opts.AddOption("sparams/scalar_style", 0, "scalar-style", "Set default scalar style (YAML only)").Type(ARGTYPE_STRING).Group("format")
		.AddChoice("any").AddChoice("plain").AddChoice("single quoted").AddChoice("double quoted").AddChoice("folded");

	opts.AddOption("sparams/map_style", 0, "map-style", "Set default map style (YAML only)").Type(ARGTYPE_STRING).Group("format")
		.AddChoice("any").AddChoice("block").AddChoice("flow");

	opts.AddOption("sparams/list_style", 0, "list-style", "Set default list style (YAML only)").Type(ARGTYPE_STRING).Group("format")
		.AddChoice("any").AddChoice("block").AddChoice("flow");

	opts.AddOption("sparams/implicit_document_begin", 0, "implicit-document-begin", "Set to let the emitter omit the document begin tag (YAML only)")
		.Group("format") .Type(ARGTYPE_BOOL);

	opts.AddOption("sparams/implicit_document_end", 0, "implicit-document-end", "Set to let the emitter omit the document end tag (YAML only)")
		.Group("format") .Type(ARGTYPE_BOOL);

	opts.AddFlag("extension/flatten", 'f', "flatten-path", "Flatten the paths using Flatten/Expand path format specifiers");
	opts.AddFlag("extension/expand", 'e', "expand-path", "Expand the paths using Flatten/Expand path format specifiers");
	
	opts.AddGroup("extension-format", "Flatten/Expand path format specifiers");
	
	opts.AddOption("extension/params/noSimpleLists", 0, "no-simple-lists")
		.Type(ARGTYPE_BOOL).Default(false).Group("extension-format");
	
	opts.AddOption("extension/params/pathPrefix", 0, "path-prefix")
		.Type(ARGTYPE_STRING).Default("").Group("extension-format").MaxLength(1);
	
	opts.AddOption("extension/params/pathDelimiter", 0, "path-delimiter")
		.Type(ARGTYPE_STRING).Default("/").Group("extension-format").MaxLength(1);
	
	opts.AddOption("extension/params/indexPrefix", 0, "index-prefix")
		.Type(ARGTYPE_STRING).Default("[").Group("extension-format").MaxLength(1);
	
	opts.AddOption("extension/params/indexSuffix", 0, "index-suffix")
		.Type(ARGTYPE_STRING).Default("]").Group("extension-format").MaxLength(1);
	
	opts.AddOption("extension/params/listDelimiter", 0, "list-delimiter")
		.Type(ARGTYPE_STRING).Default(",").Group("extension-format").MaxLength(1);

	opts.AddArgument("input", "The files to read in, if absent read stdin.").Action(ARGACTION_APPEND).MinArgs(0).MaxArgs(0);

	State state;
	state.conf = opts.Parse(argc, argv);

	if (state.conf.Get("json", false).AsBool()) { state.stype = SERIALIZE_JSON; }
	if (state.conf.Get("yaml", false).AsBool()) { state.stype = SERIALIZE_YAML; }
	if (state.conf.Get("plist", false).AsBool()) { state.stype = SERIALIZE_XMLPLIST; }
	if (state.conf.Get("bundlehdr", false).AsBool()) { state.stype = SERIALIZE_BUNDLEHDR; }

	state.loader.AddPath(".");

	if (state.conf.Contains("schema_path")) {
		for (Variant::ListIterator i(state.conf["schema_path"].ListBegin()), e(state.conf["schema_path"].ListEnd()); i != e; ++i) {
			state.loader.AddPath(i->AsString());
		}
	}

	if (state.conf.Contains("set-file")) {
	   	state.conf["set"] = DeserializeGuessFile(state.conf["set-file"].AsString().c_str());
   	}

	ofstream output_file;
	ostream *output = &cout;
	if (state.conf.Contains("output")) {
		output_file.open(state.conf["output"].AsString().c_str());
		output = &output_file;
	}

	if (state.conf.Contains("input")) {
		for (Variant::ListIterator i(state.conf["input"].ListBegin()), e(state.conf["input"].ListEnd()); i != e; ++i) {
			ifstream input(i->AsString().c_str());
			Process(state, *output, input);
		}
	} else {
		Process(state, *output, cin);
	}
	return 0;
}


