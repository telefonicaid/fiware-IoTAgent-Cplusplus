/** \file
 * \author John Bridgman
 * \brief 
 */

#include <Variant/ArgParse.h>
#include <Variant/Extensions.h>
#include <Variant/Schema.h>
#include <Variant/Path.h>
#include <iostream>
#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <getopt.h>
#include <vector>
#include <set>
#include <errno.h>
#include <regex.h>
#include <limits.h>
#include <ctype.h>
#include <libgen.h>
#include <unistd.h>
#include "GuessScalar.h"
#include "ParseBool.h"

#if 0
#define DBPRINT(x) std::cout << x
#else
#define DBPRINT(X)
#endif

namespace libvariant {

	// -----------------
	// Static data
	// -----------------

	extern const char ARG_PARSE_SCHEMA_STR[];
	static const struct ArgParseStatic {
		ArgParseStatic() {
			schema = DeserializeJSON(ARG_PARSE_SCHEMA_STR);
		}
		Variant schema;
	} arg_parse_static;


	// ----------------
	// Static and global functions
	// ----------------

	ArgParseAction ArgParseActionFromStr(const std::string &action) {
		if (action == "store") {
			return ARGACTION_STORE;
		} else if (action == "store_const") {
			return ARGACTION_STORE_CONST; 
		} else if (action == "store_true") {
			return ARGACTION_STORE_TRUE;
		} else if (action == "store_false") {
			return ARGACTION_STORE_FALSE;
		} else if (action == "append") {
			return ARGACTION_APPEND;
		} else if (action == "count") {
			return ARGACTION_COUNT;
		} else if (action == "help") {
			return ARGACTION_HELP;
		} else if (action == "adv_help") {
			return ARGACTION_ADVHELP;
		} else if (action == "config") {
			return ARGACTION_CONFIG;
		} else if (action == "config_json") {
			return ARGACTION_CONFIG_JSON;
		} else if (action == "config_yaml") {
			return ARGACTION_CONFIG_YAML;
		} else if (action == "config_plist") {
			return ARGACTION_CONFIG_PLIST;
		} else if (action == "config_print") {
			return ARGACTION_PRINT_CONFIG;
		} else {
			return ARGACTION_UNKNOWN;
		}
	}

	std::string ArgParseActionToStr(ArgParseAction action) {
		switch (action) {
		case ARGACTION_STORE:
			return "store";
		case ARGACTION_STORE_CONST:
			return "store_const";
		case ARGACTION_STORE_TRUE:
			return "store_true";
		case ARGACTION_STORE_FALSE:
			return "store_false";
		case ARGACTION_APPEND:
			return "append";
		case ARGACTION_COUNT:
			return "count";
		case ARGACTION_HELP:
			return "help";
		case ARGACTION_ADVHELP:
			return "adv_help";
		case ARGACTION_CONFIG:
			return "config";
		case ARGACTION_CONFIG_JSON:
			return "config_json";
		case ARGACTION_CONFIG_YAML:
			return "config_yaml";
		case ARGACTION_CONFIG_PLIST:
			return "config_plist";
		case ARGACTION_PRINT_CONFIG:
			return "config_print";
		case ARGACTION_UNKNOWN:
		default:
			throw std::runtime_error("ArgParseActionToStr action enum an unknown value.");
		}
	}

	static bool ArgParseActionHasArg(ArgParseAction action) {
		switch (action) {
		case ARGACTION_STORE:
		case ARGACTION_APPEND:
		case ARGACTION_CONFIG:
		case ARGACTION_CONFIG_JSON:
		case ARGACTION_CONFIG_YAML:
		case ARGACTION_CONFIG_PLIST:
			return true;
		case ARGACTION_STORE_CONST:
		case ARGACTION_STORE_TRUE:
		case ARGACTION_STORE_FALSE:
		case ARGACTION_COUNT:
		case ARGACTION_HELP:
		case ARGACTION_ADVHELP:
			return false;
		default:
			return false;
		}
	}

	static bool ArgParseActionHasResult(ArgParseAction action) {
		switch (action) {
		case ARGACTION_STORE:
		case ARGACTION_APPEND:
		case ARGACTION_STORE_CONST:
		case ARGACTION_STORE_TRUE:
		case ARGACTION_STORE_FALSE:
		case ARGACTION_COUNT:
			return true;
		case ARGACTION_CONFIG:
		case ARGACTION_CONFIG_JSON:
		case ARGACTION_CONFIG_YAML:
		case ARGACTION_CONFIG_PLIST:
		case ARGACTION_HELP:
		case ARGACTION_ADVHELP:
			return false;
		default:
			return false;
		}
	}

	ArgParseType ParseType(const Variant &type) {
		if (type.IsString()) {
			if (type == "string") { return ARGTYPE_STRING; }
			if (type == "number") { return ARGTYPE_FLOAT; }
			if (type == "integer") { return ARGTYPE_INT; }
			if (type == "boolean") { return ARGTYPE_BOOL; }
			if (type == "null") { return ARGTYPE_GUESS; }

			std::ostringstream oss;
			oss << "libvariant::ArgParse unrecognized type \"" << type.AsString() << "\"";
			throw std::runtime_error(oss.str());
		}
		return ARGTYPE_GUESS;
	}

	std::string TypeString(Variant::Type_t type) {
		switch (type) {
		case Variant::NullType:
			return "null";
		case Variant::BoolType:
			return "boolean";
		case Variant::IntegerType:
		case Variant::UnsignedType:
			return "integer";
		case Variant::FloatType:
			return "number";
		case Variant::StringType:
			return "string";
		case Variant::ListType:
		case Variant::MapType:
		case Variant::BlobType:
		default:
			return "null";
		}
	}

	//-----------------
	//

	struct ArgParseOptImpl {
		ArgParseOptImpl(ArgParseImpl *i, int id, const Path &p, int sOpt, const std::string &lOpt,
				const std::string &htxt, ArgParseAction a);
		void Set(const std::string &key, Variant v) { opt.Set(key, v); }
		bool HasArg() const { return ArgParseActionHasArg(action); }
		bool HasResult() { return ArgParseActionHasResult(action); }
		void HandleOption(const char *arg) { Handle(ParseArg(arg)); }
		void Handle(Variant arg);
		void CheckRequirements(Variant val);
		Variant ParseArg(const char *arg);
		void Init(Variant o);

		void FinalizeResult();
		void FormResult(VariantRef res);

		std::ostream &MakeArgString(std::ostream &os) const;
		void HelpLine(std::ostream &os, bool is_arg, bool adv, unsigned indent) const;
		Variant GetDescription() const;
		void GenerateSchema(VariantRef schema) const;
		void GenerateSchema(VariantRef schema, Path::const_iterator i, Path::const_iterator e) const;
		void GenerateObjectSchema(VariantRef schema, Path::const_iterator i, Path::const_iterator e) const;
		void GenerateArraySchema(VariantRef schema, Path::const_iterator i, Path::const_iterator e) const;
		Variant GenerateSchema() const;

		Variant::Type_t type;
		Path path;
		int id;
		std::set<std::string> groups;
		int shortOpt;
		std::string longOpt;
		ArgParseAction action;
		bool required;
		bool advanced;
		ArgParseImpl *impl;
		Variant opt;
		Variant result;
	};

	struct ArgString {
		ArgString(const ArgParseOptImpl &o) : opt(o) {}
		const ArgParseOptImpl &opt;
	};
	static std::ostream &operator<<(std::ostream &os, const ArgString &as) {
		return as.opt.MakeArgString(os);
	}

	struct OptName {
		OptName(ArgParseOptImpl *o, bool a=true) : opt(o), arg(a) {}
		ArgParseOptImpl *opt;
		bool arg;
	};

	static std::ostream &operator<<(std::ostream &os, const OptName &o) {
		if (o.opt->longOpt != "") {
			os << "option --" << o.opt->longOpt;
			if (o.arg) os << " argument";
		} else if (o.opt->shortOpt != 0) {
				os << "option -" << (char)o.opt->shortOpt;
				if (o.arg) os << " argument";
			} else {
				os << "argument " << o.opt->opt.Get("title").AsString();
			}
		return os;
	}

	struct ArgParseGroupImpl {

		ArgParseGroupImpl(ArgParseImpl *i, const std::string &name_)
			: name(name_),
			required(false),
			mutually_exclusive(false),
			advanced(false),
			impl(i)
		{}

		void Add(const Path &path);
		void Add(ArgParseOptImpl *i);
		void Init(Variant o);
		void FinalizeResult();
		Variant GetDescription() const;
		void GenerateSchema(VariantRef schema) const;
		void GroupSchema(VariantRef schema, const ArgParseOptImpl *opt,
			   	Path::const_iterator i, Path::const_iterator e) const;

		std::string name;
		std::string title;
		std::string description;
		std::set<ArgParseOptImpl*> members;
		bool required;
		bool mutually_exclusive;
		bool advanced;
		ArgParseImpl *impl;
	};

	struct ArgParseImpl {

		ArgParseImpl(const std::string &pname, bool addhelp)
			: option_count(256), resolve_conflicts(false), have_advanced(false),
		   	add_help(addhelp), print_help(false),
		   	print_adv_help(false), print_config(false)
		{
			SetProgramName(pname);
		}

		ArgParseOptImpl *AddOption(const std::string &keypath, int shortOpt, const std::string &longOpt,
				const std::string &helptxt, ArgParseAction action)
		{
			Path path;
			ParsePath(path, keypath);
			int id = CheckExisting(path, shortOpt, longOpt);
			if (longOpt.empty() && shortOpt == 0) {
				throw std::runtime_error("ArgParse: Error attempting to add an option without"
						" a short or long option.");
			}
			options.push_back(id);
			optmap_iter i = option_map.insert(std::make_pair(id,
						ArgParseOptImpl(this, id, path, shortOpt, longOpt, helptxt, action))).first;
			if (!longOpt.empty()) {
				longOpts.insert(std::make_pair(longOpt, id));
			}
			if (shortOpt > 0) {
				shortOpts.insert(std::make_pair((char)shortOpt, id));
			}
			paths.insert(std::make_pair(PathString(path), id));
			return &i->second;
		}

		ArgParseOptImpl *At(const std::string &keypath) {
			Path path;
			ParsePath(path, keypath);
			return At(path);
		}

		ArgParseOptImpl *At(const Path &path) {
			std::map<std::string, int>::iterator path_iter;
			path_iter = paths.find(PathString(path));
			if (path_iter == paths.end()) {
				throw std::runtime_error("ArgParse error, attempting to reference an non-existing option");
			}
			return GetOpt(path_iter->second);
		}

		ArgParseOptImpl *GetOpt(int id) {
			return &option_map.find(id)->second;
		}

		const ArgParseOptImpl *GetOpt(int id) const {
			return &option_map.find(id)->second;
		}

		ArgParseOptImpl *AddArgument(const std::string &keypath, const std::string &helptxt,
				ArgParseAction action=ARGACTION_STORE) {
			Path path;
			ParsePath(path, keypath);
			int sopt = 0;
			int id = CheckExisting(path, sopt, "");
			arguments.push_back(id);
			optmap_iter i = option_map.insert(std::make_pair(id,
					ArgParseOptImpl(this, id, path, 0, "", helptxt, action)
						)).first;
			paths.insert(std::make_pair(PathString(path), id));
			i->second.Init(Variant().Set("maxArgs", 1).Set("minArgs", 1));
			return &i->second;
		}

		ArgParseOptImpl *At(unsigned arg_index) {
			return GetOpt(arguments.at(arg_index));
		}

		ArgParseGroupImpl *GetGroup(const std::string &name) {
			return &groups.insert(std::make_pair(name, ArgParseGroupImpl(this, name))).first->second;
		}

		void ConflictResolution(bool resolve) {
			resolve_conflicts = resolve;
		}

		void AddHelp() {
			add_help = true;
		}

		void AddConfigDefaults() {
			AddOption("default-config-load", 0, "config", "Load options from file.", ARGACTION_CONFIG);
			AddOption("default-config-print", 0, "print-config", "Print the configuration and exit.",
					ARGACTION_PRINT_CONFIG);
		}

		int CheckExisting(const Path &path, int &shortOpt, const std::string &longOpt) {
			if (paths.find(PathString(path)) != paths.end()) {
				std::ostringstream oss;
				oss << "ArgParse option key \"" << PathString(path) << "\" already exists in options.";
				throw std::runtime_error(oss.str());
			}
			if (shortOpt > 0 && shortOpts.find(shortOpt) != shortOpts.end()) {
				if (resolve_conflicts) {
					shortOpt = 0;
				} else {
					std::ostringstream oss;
					oss << "ArgParse duplicate short option \"" << std::string(1, (char)shortOpt)
						<< "\"";
					throw std::runtime_error(oss.str());
				}
			}
			if (!longOpt.empty() && longOpts.find(longOpt) != longOpts.end()) {
				std::ostringstream oss;
				oss << "ArgParse duplicate long option \"" << longOpt << "\"";
				throw std::runtime_error(oss.str());
			}
			if (shortOpt > 0) { return shortOpt; }
			else { return ++option_count; }
		}

		void SetProgramName(const std::string &pname, bool do_basename=true);

		void Parse(int argc, char *const *argv);
		Variant FormResult();

		void PrintHelp(bool advanced) const;
		void PrintConfig();

		void MergeDescription(Variant desc);
		void MergeOptions(Variant opts);
		void MergeOptions(Variant opt, Path &path);

		void GenerateSchema(VariantRef schema) const;
		Variant GetDescription() const;

		std::string title;
		std::string description;
		std::string epilog;
		int option_count;
		std::map<int, ArgParseOptImpl> option_map;
		typedef std::map<int, ArgParseOptImpl>::iterator optmap_iter;
		std::vector<int> options;
		typedef std::vector<int>::iterator opt_iter;
		typedef std::vector<int>::const_iterator const_opt_iter;
		std::map<std::string, int> longOpts;
		std::map<char, int> shortOpts;
		std::map<std::string, int> paths;
		std::vector<int> arguments;
		typedef std::vector<int>::iterator arg_iter;
		typedef std::vector<int>::const_iterator const_arg_iter;

		std::map<std::string, ArgParseGroupImpl> groups;
		typedef std::map<std::string, ArgParseGroupImpl>::iterator group_iter;
		typedef std::map<std::string, ArgParseGroupImpl>::const_iterator const_group_iter;

		bool resolve_conflicts;
		bool have_advanced;
		bool add_help;
		bool print_help;
		bool print_adv_help;
		bool print_config;
	};


	// -------------
	// ArgParseOption implemenatation
	// -------------

	ArgParseOption &ArgParseOption::Title(const std::string &title) {
		impl->Set("title", title);
		return *this;
	}

	ArgParseOption &ArgParseOption::Action(const std::string &action) {
		return Action(ArgParseActionFromStr(action));
	}

	ArgParseOption &ArgParseOption::Action(ArgParseAction action) {
		if (action == ARGACTION_UNKNOWN) {
			throw std::runtime_error("ArgParse: Unknown action");
		}
		impl->action = action;
		return *this;
	}

	ArgParseOption &ArgParseOption::Type(ArgParseType type) {
		switch (type) {
		case ARGTYPE_GUESS:
		case ARGTYPE_INT:
		case ARGTYPE_UINT:
		case ARGTYPE_STRING:
		case ARGTYPE_BOOL:
		case ARGTYPE_FLOAT:
			impl->type = (Variant::Type_t)type;
			return *this;
		default:
			throw std::runtime_error("ArgParse: Unknown argument type specified.");
		}
	}
	
	ArgParseOption &ArgParseOption::Type(const std::string &type) {
		return Type(ParseType(type));
	}

	ArgParseOption &ArgParseOption::Type(Variant::Type_t type) {
		if (type == Variant::MapType || type == Variant::ListType) {
			std::ostringstream oss;
			oss << "ArgParse: Type " << type << " cannot be specified as an option type.";
			throw std::runtime_error(oss.str());
		}
		impl->type = type;
		return *this;
	}

	ArgParseOption &ArgParseOption::Const(Variant val) {
		impl->Set("const", val);
		return *this;
	}

	ArgParseOption &ArgParseOption::Default(Variant val) {
		impl->Set("default", val);
		return *this;
	}

	ArgParseOption &ArgParseOption::Env(const char *env) {
		impl->Set("env", env);
		return *this;
	}

	ArgParseOption &ArgParseOption::Minimum(Variant val) {
		if (!val.IsNumber()) {
			throw std::runtime_error("Minimum requires a number.");
		}
		if (impl->type == Variant::NullType) { impl->type = Variant::FloatType; }
		impl->Set("minimum", val);
		return *this;
	}

	ArgParseOption &ArgParseOption::Maximum(Variant val) {
		if (!val.IsNumber()) {
			throw std::runtime_error("Maximum requires a number.");
		}
		if (impl->type == Variant::NullType) { impl->type = Variant::FloatType; }
		impl->Set("maximum", val);
		return *this;
	}

	ArgParseOption &ArgParseOption::AddChoice(Variant val) {
		if (!impl->opt.Contains("enum")) { impl->Set("enum", Variant::ListType); }
		Variant e = impl->opt["enum"];
		if (std::find(e.ListBegin(), e.ListEnd(), val) == e.ListEnd()) {
			e.Append(val);
		}
		return *this;
	}

	ArgParseOption &ArgParseOption::SetChoices(const std::vector<Variant> &val) {
		impl->Set("enum", val);
		return *this;
	}

	const std::vector<Variant> &ArgParseOption::GetChoices() {
		return impl->opt["enum"].AsList();
	}

	ArgParseOption &ArgParseOption::Required(bool req) {
		impl->required = true;
		return *this;
	}

	ArgParseOption &ArgParseOption::Help(const std::string &helptxt) {
		impl->Set("description", helptxt);
		return *this;
	}

	ArgParseOption &ArgParseOption::Pattern(const std::string &pattern) {
			if (impl->type == Variant::NullType) { impl->type = Variant::StringType; }
		impl->Set("pattern", pattern);
		return *this;
	}

	ArgParseOption &ArgParseOption::MaxLength(unsigned l) {
		if (impl->type == Variant::NullType) { impl->type = Variant::StringType; }
		impl->Set("maxLength", l);
		return *this;
	}

	ArgParseOption &ArgParseOption::MinLength(unsigned l) {
		if (impl->type == Variant::NullType) { impl->type = Variant::StringType; }
		impl->Set("minLength", l);
		return *this;
	}

	ArgParseOption &ArgParseOption::MinArgs(unsigned l) {
		impl->Set("minArgs", l);
		if (l > 0 && !impl->required) {
			impl->required = true;
		}
		return *this;
	}

	ArgParseOption &ArgParseOption::MaxArgs(unsigned l) {
		impl->Set("maxArgs", l);
		return *this;
	}

	ArgParseOption &ArgParseOption::NumArgs(unsigned l) {
		MinArgs(l);
		MaxArgs(l);
		return *this;
	}

	ArgParseOption &ArgParseOption::Group(const std::string &gname) {
		impl->impl->GetGroup(gname)->Add(impl);
		return *this;
	}

	ArgParseOption &ArgParseOption::Advanced(bool adv) {
		impl->advanced = adv;
		impl->impl->have_advanced = true;
		return *this;
	}

	OptionGroup &OptionGroup::Title(const std::string &t) {
		impl->title = t;
		return *this;
	}

	OptionGroup &OptionGroup::Description(const std::string &d) {
		impl->description = d;
		return *this;
	}

	OptionGroup &OptionGroup::MutuallyExclusive(bool exclusive) {
		impl->mutually_exclusive = exclusive;
		return *this;
	}

	OptionGroup &OptionGroup::Required(bool req) {
		impl->required = req;
		return *this;
	}

	OptionGroup &OptionGroup::Add(const std::string &keypath) {
		Path path;
		ParsePath(path, keypath);
		impl->Add(path);
		return *this;
	}

	OptionGroup &OptionGroup::Add(const Path &keypath) {
		impl->Add(keypath);
		return *this;
	}

	OptionGroup &OptionGroup::Advanced(bool adv) {
		impl->advanced = adv;
		impl->impl->have_advanced = true;
		return *this;
	}

	// -----------------------
	// ArgParse implementation
	// -----------------------

	ArgParse::ArgParse(bool addhelp)
		: impl(shared_ptr<ArgParseImpl>(new ArgParseImpl("", addhelp)))
   	{}

	ArgParse::ArgParse(const std::string &pname, bool addhelp)
		: impl(shared_ptr<ArgParseImpl>(new ArgParseImpl(pname, addhelp)))
	{}

	ArgParseOption ArgParse::AddOption(const std::string &keypath, int shortOpt, const std::string &longOpt,
				const std::string &helptxt)
	{
		return ArgParseOption(impl->AddOption(keypath, shortOpt, longOpt, helptxt, ARGACTION_STORE));
	}

	ArgParseOption ArgParse::At(const std::string &keypath) {
		return ArgParseOption(impl->At(keypath));
	}

	ArgParseOption ArgParse::AddFlag(const std::string &keypath, int shortOpt, const std::string &longOpt,
			const std::string &helptxt)
	{
		return ArgParseOption(impl->AddOption(keypath, shortOpt, longOpt, helptxt, ARGACTION_STORE_TRUE))
			.Type(ARGTYPE_BOOL).Default(false);
	}

	OptionGroup ArgParse::AddGroup(const std::string &name, const std::string &title,
		   	const std::string &description) {
		return OptionGroup(impl->GetGroup(name)).Title(title).Description(description);
	}

	OptionGroup ArgParse::GetGroup(const std::string &name) {
		return OptionGroup(impl->GetGroup(name));
	}

	ArgParseOption ArgParse::AddArgument(const std::string &keypath, const std::string &helptxt)
   	{
		return ArgParseOption(impl->AddArgument(keypath, helptxt));
	}

	ArgParseOption ArgParse::At(size_t argumentIndex) {
		return ArgParseOption(impl->At(argumentIndex));
	}

	void ArgParse::ConflictResolution(bool resolve) {
		impl->ConflictResolution(resolve);
	}

	void ArgParse::AddConfigDefaults() {
		impl->AddConfigDefaults();
	}

	void ArgParse::AddHelp() {
		impl->AddHelp();
	}

	void ArgParse::SetDescription(const std::string &txt) {
		impl->description = txt;
	}

	void ArgParse::SetEpilog(const std::string &txt) {
		impl->epilog = txt;
	}

	void ArgParse::SetProgramName(const std::string &pname, bool do_basename) {
		impl->SetProgramName(pname, do_basename);
	}

	std::string ArgParse::GetProgramName() const {
		return impl->title;
	}

	void ArgParse::Merge(Variant newopts) {
		impl->MergeDescription(newopts);
	}

	void ArgParse::Merge(const ArgParse &other) {
		Merge(other.GetDescription());
	}

	void ArgParse::Validate() const {
		SchemaResult result = SchemaValidate(arg_parse_static.schema, GetDescription());
		if (result.Error()) {
			throw std::runtime_error(result.PrettyPrintMessage());
		}
	} 

	Variant ArgParse::Parse(int argc, char *const *argv) {
		impl->Parse(argc, argv);
		return impl->FormResult();
	}

	void ArgParse::PrintHelp(bool advanced) const {
		impl->PrintHelp(advanced);
	}

	Variant ArgParse::GetDescription() const {
		return impl->GetDescription();
	}

	Variant ArgParse::GenerateSchema() const {
		Variant schema = Variant::MapType;
		impl->GenerateSchema(schema);
		return schema;
	}

	//-------------------
	// ArgParseGroupImpl
	//-------------------

	void ArgParseGroupImpl::Add(const Path &path) {
		ArgParseOptImpl *o = impl->At(path);
		Add(o);
	}

	void ArgParseGroupImpl::Add(ArgParseOptImpl *o) {
		members.insert(o);
		o->groups.insert(name);
	}

	void ArgParseGroupImpl::Init(Variant o) {
		o.GetInto(title, "title", title);
		o.GetInto(description, "description", description);
		o.GetInto(required, "required", required);
		o.GetInto(mutually_exclusive, "mutually exclusive", mutually_exclusive);
		o.GetInto(advanced, "advanced", false);
		if (advanced) { impl->have_advanced = true; }
		if (o.Contains("members") && o["members"].IsList()) {
			for (Variant::ListIterator i(o["members"].ListBegin()), e(o["members"].ListEnd());
					i != e; ++i)
			{
				Add(impl->At(i->AsString()));
			}
		}
	}

	void ArgParseGroupImpl::FinalizeResult() {
		if (mutually_exclusive) {
			ArgParseOptImpl *last = 0;
			for (std::set<ArgParseOptImpl*>::iterator i(members.begin()), e(members.end());
					i != e; ++i)
			{
				ArgParseOptImpl *o = *i;
				if (!o->result.IsNull()) {
					if (!last) {
						last = o;
					} else {
						std::ostringstream oss;
						oss << "Error parsing arguments\n"
							<< OptName(last, false) << " and " << OptName(o, false)
							<< " are mutually exclusive.";
						throw std::runtime_error(oss.str());
					}
				}
			}
			if (required && !last) {
				std::ostringstream oss;
				oss << "Error parsing arguments\n"
					<< "One of the following options must be specified:\n";
				for (std::set<ArgParseOptImpl*>::iterator i(members.begin()), e(members.end());
						i != e; ++i)
				{
					oss << "\t" << OptName(*i, false) << "\n";
				}
				throw std::runtime_error(oss.str());
			}
		}
	}

	Variant ArgParseGroupImpl::GetDescription() const {
		Variant desc = Variant::MapType;
		if (!title.empty()) { desc.Set("title", title); }
		if (!description.empty()) { desc.Set("description", description); }
		desc.Set("required", required);
		desc.Set("mutually exclusive", mutually_exclusive);
		Variant mem = Variant::ListType;
		for (std::set<ArgParseOptImpl*>::iterator i(members.begin()), e(members.end());
				i != e; ++i)
		{
			mem.Append(PathString((*i)->path));
		}
		desc.Set("advanced", advanced);
		desc.Set("members", mem);
		return desc;
	}

	void ArgParseGroupImpl::GenerateSchema(VariantRef schema) const {
		// The basic pattern for mutual exclusion with required is:
		// "oneOf": [
		// { "required": [ "key" ] },
		// { "required": [ "key2" ] },
		// ]
		// just mutual exclusion:
		// "oneOf": [
		// { "required": [ "key" ] },
		// { "required": [ "key2" ] },
		// { "not": { "required": [ "key", "key2" ] }
		// ]
		// What makes things funny is paths.
		if (mutually_exclusive) {
			Variant oneof = Variant::ListType;
			for (std::set<ArgParseOptImpl*>::const_iterator i(members.begin()), e(members.end());
					i != e; ++i)
			{
				Variant instance;
				GroupSchema(instance, *i, (*i)->path.begin(), (*i)->path.end());
				oneof.Append(instance);
			}
			if (!required) {
				Variant anyof = Variant().Set("anyOf", oneof.Copy());
				oneof.Append(Variant().Set("not", anyof));
			}
			if (!schema.Contains("allOf")) { schema["allOf"] = Variant::ListType; }
			schema["allOf"].Append(Variant().Set("oneOf", oneof));
		}
	}

	void ArgParseGroupImpl::GroupSchema(VariantRef schema, const ArgParseOptImpl *opt,
		   	Path::const_iterator i, Path::const_iterator e) const
	{
		if (i != e) {
			if (i->IsString()) {
				schema["required"].Append(i->AsString());
				GroupSchema(schema["properties"][i->AsString()], opt, i+1, e);
			} else if (i->IsNumber()) {
				if (!schema.Contains("items")) { schema["items"] = Variant::ListType; }
				if (schema["items"].Size() < i->AsUnsigned()) {
					schema["items"].AsList().resize(i->AsUnsigned(), Variant::MapType);
				}
				GroupSchema(schema["items"][unsigned(i->AsUnsigned())], opt, i+1, e);
			}
		} else {
			schema = Variant::MapType;
		}
	}

	//-------------
	// ArgParseImpl
	//-------------

#if __APPLE__
#include <mach-o/dyld.h>
#endif
	void ArgParseImpl::SetProgramName(const std::string &pname, bool do_basename) {
		if (pname.length() > 0) {
			if (do_basename) {
				char buf[pname.size()+1];
				strcpy(buf,pname.c_str());
				title = basename(buf);
			}
			else {
				title = pname;
			}
		}
		else {
			// try to figure out the program name on our own

			char *progname = 0;
			char buf[PATH_MAX*2];
			memset(buf, 0, PATH_MAX*2);

			#if __linux__
			if (!progname || !strlen(progname)) {
				size_t bufsize = sizeof(buf);
				if (0 == readlink("/proc/self/exe", buf, bufsize)) {
					progname = buf;
				}
			}
			#endif /* __linux__ */

			#if __APPLE__
			if (!progname || !strlen(progname)) {
				uint32_t bufsize = sizeof(buf);
				if (0 == _NSGetExecutablePath(buf, &bufsize)) {
					progname = buf;
				}
			}
			#endif /* __APPLE__ */

			if (!progname || !strlen(progname)) {
				// Note: this is not accurate if the shell that set "_"
				// called a program that called another program
				progname = getenv("_");
			}

			if (!progname || !strlen(progname)) {
				title = "ArgParse";
			} else {
				title = std::string(basename(progname));
			}
		}
	}

	void ArgParseImpl::Parse(int argc, char *const *argv) {

		if (add_help) {
			bool old_conflict = resolve_conflicts;
			resolve_conflicts = true;
			try {
				AddOption("default-help-option", 'h', "help", "Print this message and exit.", ARGACTION_HELP);
				if (have_advanced) {
					AddOption("default-adv-help-option", 0, "adv-help", "Print help message"
							" including options marked as advanced.", ARGACTION_ADVHELP);
				}
			} catch (const std::runtime_error &) {}
			resolve_conflicts = old_conflict;
		}

		try {
			std::string shortOpts;
			std::vector<struct option> longOpts;
			for (optmap_iter i(option_map.begin()), e(option_map.end()); i != e; ++i) {
				ArgParseOptImpl &o = i->second;
				int id = i->first;
				if (o.shortOpt > 0) {
					shortOpts.append(1, (char)o.shortOpt);
					if (o.HasArg()) { shortOpts.append(1, ':'); }
				}
				if (!o.longOpt.empty()) {
					struct option tmp_option =
					{ o.longOpt.c_str(), (o.HasArg() ? 1 : 0), 0, id };
					longOpts.push_back(tmp_option);
				}
				if (o.action == ARGACTION_COUNT) { o.result = 0u; }
				// Handle environment variables
				if (o.opt.Contains("env")) {
					const char *env = getenv(o.opt["env"].AsString().c_str());
					if (env) { o.HandleOption(env); }
				}
			}
			{
				// End the option list
				struct option tmp_option = {0, 0, 0, 0};
				longOpts.push_back(tmp_option);
			}

			optind = 1; // Make sure getopt is in initial state
			while (true) {
				int c = getopt_long(argc, argv, shortOpts.c_str(), &longOpts[0], 0);
				if (c == -1) { break; }
				if (c == '?') { PrintHelp(false); }
				optmap_iter o = option_map.find(c);
				if (o == option_map.end()) { PrintHelp(false); }
				ArgParseOptImpl &opt = o->second;
				opt.HandleOption(optarg);
			}

			// Handle arguments
			unsigned minimum_arguments = 0;
			for (arg_iter i(arguments.begin()), e(arguments.end()); i != e; ++i) {
				ArgParseOptImpl *o = GetOpt(*i);
				unsigned minarg; o->opt.GetInto(minarg, "minArgs", 0u);
				if (minarg > 0) { minimum_arguments += minarg; }
				else { break; }
			}
			unsigned numarg = argc - optind;
			unsigned index = optind;
			unsigned maximum_arguments = 0;
			for (arg_iter i(arguments.begin()), e(arguments.end()); i != e; ++i) {
				unsigned minarg, maxarg;
				ArgParseOptImpl *o = GetOpt(*i);
				o->opt.GetInto(minarg, "minArgs", 0u);
				o->opt.GetInto(maxarg, "maxArgs", 0u);
				if (maxarg != 0) { maximum_arguments += maxarg; }
				else { maximum_arguments = 0; }
				if (i + 1 == e && maxarg != 0 && argc - index > maxarg) {
					std::ostringstream oss;
					oss << "Error parsing arguments\nToo many arguments specified, "
						<< numarg << " given, maximum: " << maximum_arguments;
					throw std::runtime_error(oss.str());
				} else {
					unsigned num = 0;
					while (index < (unsigned)argc && (maxarg == 0 || num < maxarg)) {
						o->HandleOption(argv[index]);
						++index;
						++num;
					}
				}
			}
			if (index < (unsigned)argc) {
				throw std::runtime_error("Error parsing arguments\nExtra arguments given.");
			}

			for (optmap_iter i(option_map.begin()), e(option_map.end()); i != e; ++i) {
				i->second.FinalizeResult();
			}
			for (group_iter i(groups.begin()), e(groups.end()); i != e; ++i) {
				i->second.FinalizeResult();
			}
			if (print_adv_help) { PrintHelp(true); }
			if (print_help) { PrintHelp(false); }
			if (print_config) { PrintConfig(); }
		} catch (const std::exception &e) {
			if (print_adv_help) { PrintHelp(true); }
			if (print_help) { PrintHelp(false); }
			if (print_config) { PrintConfig(); }
			std::cerr << e.what() << std::endl;
			exit(1);
		}
	}

	Variant ArgParseImpl::FormResult() {
		Variant ret = Variant::MapType;
		for (optmap_iter i(option_map.begin()), e(option_map.end()); i != e; ++i) {
			i->second.FormResult(ret);
		}
		return ret;
	}


	struct PrintHelpGroup {
		std::string title;
		std::string description;
		bool advanced;
		std::vector<const ArgParseOptImpl*> members;
	};

	void ArgParseImpl::PrintHelp(bool advanced) const {
		std::cerr << title << ": [options]";
		for (const_arg_iter i(arguments.begin()), e(arguments.end()); i != e; ++i) {
			const ArgParseOptImpl &arg = *GetOpt(*i);
			unsigned minArgs, maxArgs;
			arg.opt.GetInto(minArgs, "minArgs", 0u);
			arg.opt.GetInto(maxArgs, "maxArgs", 0u);
			if (minArgs == 0 && maxArgs == 0) {
				std::cerr << " [" << ArgString(arg) << " [" << ArgString(arg) << " ...]]";
			} else {
				for (unsigned i = 0; i < minArgs; ++i) {
					std::cerr << " " << ArgString(arg);
				}
				if (maxArgs > minArgs) {
					for (unsigned i = minArgs; i < maxArgs; ++i) {
						std::cerr << " [" << ArgString(arg);
					}
					for (unsigned i = minArgs; i < maxArgs; ++i) {
						std::cerr << "]";
					}
				} else if (maxArgs < minArgs) {
					std::cerr << " [" << ArgString(arg) << " ...]";
				}
			}
		}
		std::cerr << "\n\n";
		if (!description.empty()) {
			std::cerr << description << "\n\n";
		}
		if (!arguments.empty()) {
			std::cerr << "Arguments:\n";
		}
		for (const_arg_iter i(arguments.begin()), e(arguments.end()); i != e; ++i) {
			const ArgParseOptImpl &arg = *GetOpt(*i);
			arg.HelpLine(std::cerr, false, true, 1);
		}
		if (!options.empty()) {
			std::cerr << "Options:\n";
		}
		std::vector<const ArgParseOptImpl*> ungrouped;
		std::map<std::string, PrintHelpGroup> groupings;

		for (const_opt_iter i(options.begin()), e(options.end()); i != e; ++i) {
			bool in_group = false;
			const ArgParseOptImpl &opt = *GetOpt(*i);
			std::set<std::string>::const_iterator g(opt.groups.begin()), gend(opt.groups.end());
			for (; g != gend; ++g) {
				const_group_iter group_i = groups.find(*g);
				if (group_i == groups.end()) {
				   	std::cerr << "Group " << *g << " referenced but does not exist\n";
					continue;
			   	}
				const ArgParseGroupImpl *gimpl = &group_i->second;
				if (!gimpl->title.empty() || !gimpl->description.empty()) {
					in_group = true;
					std::map<std::string, PrintHelpGroup>::iterator giter;
					giter = groupings.find(*g);
					if (giter == groupings.end()) {
						PrintHelpGroup group;
						group.title = gimpl->title;
						group.description = gimpl->description;
						group.advanced = gimpl->advanced;
						group.members.push_back(&opt);
						groupings[*g] = group;
					} else {
						giter->second.members.push_back(&opt);
					}
					break;
				}
			}
			if (!in_group) {
				ungrouped.push_back(&opt);
			}
		}
		for (std::vector<const ArgParseOptImpl*>::iterator i(ungrouped.begin()), e(ungrouped.end());
				i != e; ++i)
		{
			(*i)->HelpLine(std::cerr, true, advanced, 1);
		}
		for (std::map<std::string, PrintHelpGroup>::iterator i(groupings.begin()), e(groupings.end());
				i != e; ++i)
		{
			if ( !(!i->second.advanced || advanced) ) { continue; }
			std::cerr << "\n";
			if (! i->second.title.empty()) {
				std::cerr << i->second.title << "\n";
			}
			if (! i->second.description.empty()) {
				std::cerr << i->second.description << "\n";
			}
			for (std::vector<const ArgParseOptImpl*>::iterator j(i->second.members.begin()),
				   	je(i->second.members.end());
					j != je; ++j)
			{
				(*j)->HelpLine(std::cerr, true, advanced, 1);
			}
		}
		if (!epilog.empty()) {
			std::cerr << "\n" << epilog << "\n" << std::endl;
		}
		exit(0);
	}

	void ArgParseImpl::PrintConfig() {
		Variant config = FormResult();
		Serialize(std::cout.rdbuf(), config, SERIALIZE_JSON, Variant().Set("pretty", true));
		std::cout << std::endl;
		exit(0);
	}

	void ArgParseImpl::MergeDescription(Variant desc) {
		if (!desc.IsMap()) { return; }
		if (desc.Contains("options") && desc["options"].IsMap()) {
			Variant opts = desc["options"];
			for (Variant::MapIterator i(opts.MapBegin()), e(opts.MapEnd());
					i != e; ++i)
			{
				Variant o = i->second;
				std::string keypath = i->first;
				int shortOpt = 0;
				if (o.Contains("shortOption")) {
					shortOpt = o.Get("shortOption").AsString()[0];
				}
				std::string longOpt = o.Get("longOption", "").AsString();
				std::string helptxt = o.Get("description", "").AsString();
				ArgParseAction action = ArgParseActionFromStr(o.Get("action", "store").AsString());
				AddOption(keypath, shortOpt, longOpt, helptxt, action)->Init(o);
			}
		}
		if (desc.Contains("groups") && desc["groups"].IsMap()) {
			Variant grps = desc["groups"];
			for (Variant::MapIterator i(grps.MapBegin()), e(grps.MapEnd());
					i != e; ++i)
			{
				Variant g = i->second;
				std::string name = i->first;
				GetGroup(name)->Init(g);
			}
		}
		if (desc.Contains("arguments") && desc["arguments"].IsList()) {
			Variant args = desc["arguments"];
			for (Variant::ListIterator i(args.ListBegin()), e(args.ListEnd());
					i != e; ++i)
			{
				Variant o = *i;
				if (!o.Contains("keypath")) { continue; }
				std::string keypath = o.Get("keypath").AsString();
				std::string helptxt = o.Get("description", "").AsString();
				ArgParseAction action = ArgParseActionFromStr(o.Get("action", "store").AsString());
				AddArgument(keypath, helptxt, action)->Init(o);
			}
		}
	}

	void ArgParseImpl::MergeOptions(Variant opts) {
		Path path;
		MergeOptions(opts, path);
	}

	void ArgParseImpl::MergeOptions(Variant opt, Path &path) {
		switch (opt.GetType()) {
		case Variant::NullType:
			break;
		case Variant::BoolType:
		case Variant::IntegerType:
		case Variant::UnsignedType:
		case Variant::FloatType:
		case Variant::StringType:
			try {
				At(path)->Handle(opt);
			} catch (const std::runtime_error &) {}
			break;
		case Variant::ListType:
			try {
				At(path)->Handle(opt);
			} catch (const std::runtime_error &) {
				unsigned index = 0;
				for (Variant::ListIterator i(opt.ListBegin()), e(opt.ListEnd());
						i != e; ++i, ++index) {
					path.push_back(index);
					MergeOptions(*i, path);
					path.pop_back();
				}
			}
			break;
		case Variant::MapType:
			for (Variant::MapIterator i(opt.MapBegin()), e(opt.MapEnd()); i != e; ++i) {
				path.push_back(i->first);
				MergeOptions(i->second, path);
				path.pop_back();
			}
			break;
		case Variant::BlobType:
			break;
		}
	}

	void ArgParseImpl::GenerateSchema(VariantRef schema) const {
		for (const_opt_iter i(options.begin()), e(options.end()); i != e; ++i) {
			GetOpt(*i)->GenerateSchema(schema);
		}
		for (const_arg_iter i(arguments.begin()), e(arguments.end()); i != e; ++i) {
			GetOpt(*i)->GenerateSchema(schema);
		}
		for (const_group_iter i(groups.begin()), e(groups.end()); i != e; ++i) {
			i->second.GenerateSchema(schema);
		}
	}

	Variant ArgParseImpl::GetDescription() const {
		Variant desc = Variant::MapType;
		desc.Set("title", title);
		desc.Set("description", description);
		desc.Set("epilog", epilog);
		for (const_opt_iter i(options.begin()), e(options.end()); i != e; ++i) {
			const ArgParseOptImpl *opt = GetOpt(*i);
			desc["options"].Set(PathString(opt->path), opt->GetDescription());
		}
		for (const_arg_iter i(arguments.begin()), e(arguments.end()); i != e; ++i) {
			const ArgParseOptImpl *opt = GetOpt(*i);
			desc["arguments"].Append(opt->GetDescription());
		}
		for (const_group_iter i(groups.begin()), e(groups.end()); i != e; ++i) {
			desc["groups"].Set(i->first, i->second.GetDescription());
		}
		return desc;
	}

	// ---------------
	// ArgParseOptImpl
	// ---------------

	ArgParseOptImpl::ArgParseOptImpl(ArgParseImpl *i, int id_, const Path &p,
		   	int sOpt, const std::string &lOpt, const std::string &htxt, ArgParseAction a)
		: type(Variant::NullType),
		path(p),
		id(id_),
		shortOpt(sOpt),
		longOpt(lOpt),
		action(a),
		required(false),
		advanced(false),
		impl(i),
		opt(Variant::MapType),
		result(Variant::NullType)
	{
		if (path.back().IsString()) {
			opt["title"] = path.back().AsString();
		} else {
			opt["title"] = PathString(path);
		}
		opt["description"] = htxt;
	}

	void ArgParseOptImpl::Handle(Variant arg) {
		switch (action) {
		case ARGACTION_STORE:
			CheckRequirements(arg);
			result = arg;
			break;
		case ARGACTION_STORE_CONST:
			result = opt["const"];
			break;
		case ARGACTION_STORE_TRUE:
			if (arg.IsBool()) {
				result = arg;
			} else {
				result = true;
			}
			break;
		case ARGACTION_STORE_FALSE:
			if (arg.IsBool()) {
				result = arg;
			} else {
				result = false;
			}
			break;
		case ARGACTION_APPEND:
			if (arg.IsList()) {
				for (Variant::ListIterator i(arg.ListBegin()), e(arg.ListEnd()); i != e; ++i) {
					CheckRequirements(*i);
					result.Append(*i);
				}
			} else {
				CheckRequirements(arg);
				result.Append(arg);
			}
			break;
		case ARGACTION_COUNT:
			if (arg.IsNumber()) {
				result += arg.AsInt();
			} else {
				result += 1;
			}
			break;
		case ARGACTION_HELP:
			impl->print_help = true;
			break;
		case ARGACTION_ADVHELP:
			impl->print_adv_help = true;
			break;
		case ARGACTION_CONFIG:
			impl->MergeOptions(DeserializeGuessFile(arg.AsString().c_str()));
			break;
		case ARGACTION_CONFIG_JSON:
			impl->MergeOptions(DeserializeFile(arg.AsString().c_str(), SERIALIZE_JSON));
			break;
		case ARGACTION_CONFIG_YAML:
			impl->MergeOptions(DeserializeFile(arg.AsString().c_str(), SERIALIZE_YAML));
			break;
		case ARGACTION_CONFIG_PLIST:
			impl->MergeOptions(DeserializeFile(arg.AsString().c_str(), SERIALIZE_XMLPLIST));
			break;
		case ARGACTION_PRINT_CONFIG:
			impl->print_config = true;
			break;
		default:
			break;
		}
	}

	void ArgParseOptImpl::CheckRequirements(Variant val) {

		switch (val.GetType()) {
		case Variant::StringType:
			if (opt.Contains("minLength") && val.Size() < opt["minLength"]) {
				std::ostringstream oss;
				oss << "Error parsing arguments\n" << OptName(this) << " is too short: " << val.Size()
					<< " chars, minimum " << opt["minLength"].AsUnsigned();
				throw std::runtime_error(oss.str());
			}
			if (opt.Contains("maxLength") && val.Size() > opt["maxLength"]) {
				std::ostringstream oss;
				oss << "Error parsing arguments\n" << OptName(this) << " is too long: " << val.Size()
					<< " chars, maximum " << opt["maxLength"].AsUnsigned();
				throw std::runtime_error(oss.str());
			}
			if (opt.Contains("pattern")) {
				std::string pat = opt["pattern"].AsString();
				regex_t reg;
				memset(&reg, 0, sizeof(regex_t));
				int err;
				if ((err = regcomp(&reg, pat.c_str(), REG_EXTENDED | REG_NOSUB))) {
					std::vector<char> buffer(8192);
					regerror(err, &reg, &buffer[0], buffer.size());
					std::ostringstream oss;
					oss << "Error parsing arguments\n";
					oss << "Failed to compile pattern regex \"" << pat << "\" with error: "
						<< &buffer[0];
					throw std::runtime_error(oss.str());
				}
				if (regexec(&reg, val.AsString().c_str(), 0, 0, 0) != 0) {
					regfree(&reg);
					std::ostringstream oss;
					oss << "Error parsing arguments\n";
					oss << OptName(this) << " failed to match pattern \""
						<< pat << "\"";
					throw std::runtime_error(oss.str());
				}
				regfree(&reg);
			}
			break;
		case Variant::IntegerType: case Variant::UnsignedType: case Variant::FloatType:
			if (opt.Contains("minimum") && val < opt["minimum"]) {
				std::ostringstream oss;
				oss << "Error parsing arguments\n" << OptName(this) << " is too small: ";
				SerializeJSON(oss.rdbuf(), val);
				oss << ", minimum ";
				SerializeJSON(oss.rdbuf(), opt["minimum"]);
				throw std::runtime_error(oss.str());
			}
			if (opt.Contains("maximum") && val > opt["maximum"]) {
				std::ostringstream oss;
				oss << "Error parsing arguments\n" << OptName(this) << " is too large: ";
				SerializeJSON(oss.rdbuf(), val);
				oss << ", maximum ";
				SerializeJSON(oss.rdbuf(), opt["maximum"]);
				throw std::runtime_error(oss.str());
			}
			break;
		default:
			break;
		}

		if (opt.Contains("enum")) {
			Variant choices = opt["enum"];
			bool pass = false;
			for (Variant::ListIterator i(choices.ListBegin()), e(choices.ListEnd()); i != e; ++i)
			{
				if (val == *i) { pass = true; break; }
			}
			if (!pass) {
				std::ostringstream oss;
				oss << "Error parsing arguments\n";
				SerializeJSON(oss.rdbuf(), val);
				oss << "\nis not one of the allowed choices for " << OptName(this) << std::endl;
				for (Variant::ListIterator i(choices.ListBegin()), e(choices.ListEnd()); i != e; ++i)
				{
					oss << " - ";
					SerializeJSON(oss.rdbuf(), *i);
					oss << std::endl;
				}
				throw std::runtime_error(oss.str());
			}
		}
	}


	static void ParseArgIntError(const char *arg) {
		std::ostringstream oss;
		oss << "Error parsing arguments\nUnable to convert \"" << arg << "\" to an integer";
		throw std::runtime_error(oss.str());
	}

	Variant ArgParseOptImpl::ParseArg(const char *arg) {

		class PAction : public ParserActions {
		public:
			virtual void Scalar(ParserImpl *p, double v, const char *anchor, const char *tag)
			{ val = v; }
			virtual void Scalar(ParserImpl *p, const char *str, unsigned length, const char *anchor, const char *tag)
			{ val = std::string(str, length); }
			virtual void Scalar(ParserImpl *p, bool v, const char *anchor, const char *tag)
			{ val = v; }
			virtual void Null(ParserImpl *p, const char *anchor, const char *tag)
			{ val = Variant::NullType; }
			virtual void Scalar(ParserImpl *p, intmax_t v, const char *anchor, const char *tag)
			{ val = v; }
			virtual void Scalar(ParserImpl *p, uintmax_t v, const char *anchor, const char *tag)
			{ val = v; }
			Variant val;
		};

		if (!HasArg()) {
			if (arg) {
				std::ostringstream oss;
				oss << "Error parsing arguments\n"
				   	<< OptName(this) << " does not take an argument, but argument \"" 
					<< arg << "\" given.";
				throw std::runtime_error(oss.str());
			}
			return Variant::NullType;
		}

		// Was type specified?
		unsigned len = strlen(arg);
		switch (type) {
		case Variant::BoolType:
			return Variant(ParseBool(arg));
		case Variant::IntegerType:
		{
			long long val;
			char *end = 0;
			val = strtoll(arg, &end, 10);
			if (val == 0 && end == arg) { ParseArgIntError(arg); }
			if (arg + len != end) { ParseArgIntError(arg); }
			if (!(val == LLONG_MAX && errno == ERANGE)) {
				return Variant(val);
			}
		} // Fall through
		case Variant::UnsignedType:
		{
			char *end = 0;
			unsigned long long val;
			val = strtoull(arg, &end, 10);
			if (val == 0 && end == arg) { ParseArgIntError(arg); }
			if (arg + len != end) { ParseArgIntError(arg); }
			return Variant(val);
		}
		case Variant::FloatType:
		{
			char *end = 0;
			double val = strtod(arg, &end);
			if ((val == 0 && end == arg) || (arg + len != end)) {
				std::ostringstream oss;
				oss << "Error parsing arguments\nUnable to convert \"" << arg << "\" to a number";
				throw std::runtime_error(oss.str());
			}
			return Variant(val);
		}
		case Variant::StringType:
			return Variant(arg);

		case Variant::NullType: case Variant::ListType: case Variant::MapType: case Variant::BlobType: default:
			{
				PAction a;
				GuessScalar(arg, strlen(arg), 0, 0, 0, &a);
				return a.val;
			}
		}
	}

	std::ostream &ArgParseOptImpl::MakeArgString(std::ostream &os) const {
		switch (action) {
		case ARGACTION_STORE:
		case ARGACTION_APPEND:
			if (opt.Contains("enum") && opt["enum"].IsList()) {
				for (Variant::ConstListIterator i(opt["enum"].ListBegin()),
						e(opt["enum"].ListEnd()); i != e; ++i)
				{
					os << i->AsString();
					if (i + 1 != e) {
						os << " | ";
					}
				}
			} else {
				switch (type) {
				case Variant::BoolType: os << "yes|no"; break;
				case Variant::IntegerType: case Variant::UnsignedType:
				case Variant::NullType: case Variant::FloatType: default:
					os << opt.Get("title", "XXX").AsString();
				}
			}
			break;
		case ARGACTION_CONFIG:
		case ARGACTION_CONFIG_JSON:
		case ARGACTION_CONFIG_YAML:
		case ARGACTION_CONFIG_PLIST:
			os << "FILE";
			break;
		case ARGACTION_PRINT_CONFIG:
		default:
			break;
		}
		return os;
	}

	void ArgParseOptImpl::Init(Variant o) {
		if (o.Contains("type")) {
			type = (Variant::Type_t)ParseType(o["type"]);
		}
		o.GetInto(required, "required", required);
		o.GetInto(advanced, "advanced", false);
		if (advanced) { impl->have_advanced = true; }
		opt.Merge(o);
	}

	void ArgParseOptImpl::FinalizeResult() {
		if (opt.Contains("default") && result.IsNull()) {
			bool add_default = true;
			for (std::set<std::string>::iterator i(groups.begin()), e(groups.end());
				i != e; ++i)
			{
				ArgParseGroupImpl *g = impl->GetGroup(*i);
				if (g->mutually_exclusive) {
					for (std::set<ArgParseOptImpl*>::iterator o(g->members.begin()),
							oend(g->members.end()); o != oend; ++o)
					{
						ArgParseOptImpl *opt = *o;
						if (!opt->result.IsNull()) {
							add_default = false;
						}
					}
				}
			}
			if (add_default) {
				switch (action) {
				case ARGACTION_STORE:
				case ARGACTION_STORE_CONST:
				case ARGACTION_STORE_TRUE:
				case ARGACTION_STORE_FALSE:
					result = opt["default"];
					break;
				case ARGACTION_APPEND:
					result.Append(opt["default"]);
				case ARGACTION_COUNT:
				case ARGACTION_HELP:
				case ARGACTION_ADVHELP:
				case ARGACTION_CONFIG:
				case ARGACTION_CONFIG_JSON:
				case ARGACTION_CONFIG_YAML:
				case ARGACTION_CONFIG_PLIST:
				case ARGACTION_PRINT_CONFIG:
				default:
					break;
				}
			}
		}
		if (!result.IsNull()) {
			if (action == ARGACTION_APPEND) {
				if (opt.Contains("maxArgs") && opt["maxArgs"] != 0) {
					if (opt["maxArgs"] < result.Size()) {
						std::ostringstream oss;
						oss << "Error parsing arguments\n";
						oss << "More than maximum arguments specified for " << OptName(this) << ", ";
						oss << result.Size() << " options, maximum: " << opt["maxArgs"].AsUnsigned();
						throw std::runtime_error(oss.str());
					}
				}
				if (opt.Contains("minArgs")) {
					if (opt["minArgs"] > result.Size()) {
						std::ostringstream oss;
						oss << "Error parsing arguments\n";
						oss << "Less than minimum arguments specified for " << OptName(this) << ", ";
						oss << result.Size() << " options, minimum: " << opt["minArgs"].AsUnsigned();
						throw std::runtime_error(oss.str());
					}
				}
			}
		} else if (required) {
			std::ostringstream oss;
			oss << "Error parsing arguments\n";
			oss << "Required " << OptName(this) << " missing.";
			throw std::runtime_error(oss.str());
		}
	}

	void ArgParseOptImpl::FormResult(VariantRef res) {
		if (!HasResult()) { return; }
		if (result.IsNull()) { return; }
		res.SetPath(path, result);
	}

	void ArgParseOptImpl::HelpLine(std::ostream &os, bool is_arg, bool adv, unsigned indent) const {
		if ( !(!advanced || adv) ) { return; }
		os << "\t";
		if (shortOpt > 0) {
			os << "-" << (char)shortOpt;
		}
		if (shortOpt > 0 && !longOpt.empty()) {
			os << ", ";
		}
		if (!longOpt.empty()) {
			os << "--" << longOpt;
		}
		if (HasArg()) {
			os << " " << ArgString(*this);
		}
		if (opt.Contains("env")) {
			if (shortOpt > 0 || !longOpt.empty()) {
				os << "\n\t";
			}
			os << "env: $" << opt["env"].AsString();
			if (HasArg()) {
				os << "=" << ArgString(*this);
			}
		}
		os << "\n\t\t" << opt["description"].AsString();

		if (opt.Contains("default")) {
			Variant def = opt["default"];
			if (!def.IsMap() && !def.IsList()) {
				os << " (default: ";
				SerializeJSON(os.rdbuf(), def);
				os << ")";
			}
		}
		os << "\n";
	}

	Variant ArgParseOptImpl::GetDescription() const {
		Variant desc;
		desc.Merge(opt);
		if (type == Variant::NullType) {
			desc.Set("type", type);
		} else {
			desc.Set("type", TypeString(type));
		}
		if (shortOpt > 0) {
			desc.Set("shortOption", std::string(1, (char)shortOpt));
		} else {
			desc.Erase("shortOption");
		}
		if (!longOpt.empty()) {
			desc.Set("longOption", longOpt);
		} else {
			desc.Erase("longOption");
		}
		desc.Set("action", ArgParseActionToStr(action));
		desc.Set("required", required);
		desc.Set("keypath", PathString(path));
		desc.Set("advanced", advanced);
		return desc;
	}

	void ArgParseOptImpl::GenerateSchema(VariantRef schema) const {
		GenerateSchema(schema, path.begin(), path.end());
	}

	void ArgParseOptImpl::GenerateSchema(VariantRef schema, Path::const_iterator i,
		   	Path::const_iterator e) const {
		if (i == e) {
			schema = GenerateSchema();
		} else {
			if (schema.Get("type", "") == "object") {
				GenerateObjectSchema(schema, i, e);
			} else if (schema.Get("type", "") == "array") {
				GenerateArraySchema(schema, i, e);
			} else if (i->IsString()) {
				GenerateObjectSchema(schema, i, e);
			} else if (i->IsNumber()) {
				GenerateArraySchema(schema, i, e);
			}
		}
	}

	void ArgParseOptImpl::GenerateObjectSchema(VariantRef schema, Path::const_iterator i,
		   	Path::const_iterator e) const
	{
		if (schema.IsNull()) { schema = Variant::MapType; }
		std::string key = i->AsString();
		if (!schema.Contains("type")) { schema["type"] = "object"; }
		if (!schema.Contains("properties")) { schema["properties"] = Variant::MapType; }
		if (required) {
			if (!schema.Contains("required")) { schema["required"] = Variant::ListType; }
			if (std::find(schema["required"].ListBegin(), schema["required"].ListEnd(), Variant(key))
					== schema["required"].ListEnd())
		   	{
				schema["required"].Append(key);
			}
		}
		GenerateSchema(schema["properties"].At(key, Variant::MapType), i+1, e);
	}

	void ArgParseOptImpl::GenerateArraySchema(VariantRef schema, Path::const_iterator i,
		   	Path::const_iterator e) const
   	{
		unsigned index = i->AsUnsigned();
		if (!schema.Contains("type")) { schema["type"] = "array"; }
		if (!schema.Contains("items")) { schema["items"] = Variant::ListType; }
		GenerateSchema(schema["items"].At(index, Variant::MapType), i+1, e);
	}

	Variant ArgParseOptImpl::GenerateSchema() const {
		Variant s = Variant::MapType;
		if (opt.Contains("title")) { s["title"] = opt["title"]; }
		if (opt.Contains("description")) { s["description"] = opt["description"]; }
		switch (action) {
		case ARGACTION_STORE:
			if (type != Variant::NullType) { s["type"] = TypeString(type); }
			if (opt.Contains("minimum")) { s["minimum"] = opt["minimum"]; }
			if (opt.Contains("maximum")) { s["maximum"] = opt["maximum"]; }
			if (opt.Contains("minLength")) { s["minLength"] = opt["minLength"]; }
			if (opt.Contains("maxLength")) { s["maxLength"] = opt["maxLength"]; }
			if (opt.Contains("pattern")) { s["pattern"] = opt["pattern"]; }
			if (opt.Contains("enum")) { s["enum"] = opt["enum"].Copy(); }
			if (opt.Contains("default")) { s["default"] = opt["default"]; }
			break;
		case ARGACTION_STORE_CONST:
			s["enum"].Append(opt.Get("const"));
			break;
		case ARGACTION_STORE_TRUE:
		case ARGACTION_STORE_FALSE:
			s["type"] = "boolean";
			if (opt.Contains("default")) { s["default"] = opt["default"]; }
			break;
		case ARGACTION_APPEND:
			{
				Variant itemschema = Variant::MapType;
				if (type != Variant::NullType) { itemschema["type"] = TypeString(type); }
				if (opt.Contains("minimum")) { itemschema["minimum"] = opt["minimum"]; }
				if (opt.Contains("maximum")) { itemschema["maximum"] = opt["maximum"]; }
				if (opt.Contains("minLength")) { itemschema["minLength"] = opt["minLength"]; }
				if (opt.Contains("maxLength")) { itemschema["maxLength"] = opt["maxLength"]; }
				if (opt.Contains("pattern")) { itemschema["pattern"] = opt["pattern"]; }
				if (opt.Contains("enum")) { itemschema["enum"] = opt["enum"].Copy(); }
				if (opt.Contains("default")) {
					s["default"] = Variant::ListType;
					s["default"].Append(opt["default"]);
				}
				s["items"] = itemschema;
			}
			if (opt.Contains("minArgs") && opt["minArgs"] > 0) { s["minItems"] = opt["minArgs"]; }
			if (opt.Contains("maxArgs") && opt["maxArgs"] > 0) { s["maxItems"] = opt["maxArgs"]; }
			break;
		case ARGACTION_COUNT:
			s["type"] = "integer";
			s["minimum"] = 0;
			break;
		case ARGACTION_HELP:
		case ARGACTION_ADVHELP:
		case ARGACTION_CONFIG:
		case ARGACTION_CONFIG_JSON:
		case ARGACTION_CONFIG_YAML:
		case ARGACTION_CONFIG_PLIST:
		case ARGACTION_UNKNOWN:
		default:
			break;
		}
		return s;
	}

}
