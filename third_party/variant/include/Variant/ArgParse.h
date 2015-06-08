/** \file
 * \author John Bridgman
 * \brief 
 */
#ifndef VARIANT_ARGPARSER_H
#define VARIANT_ARGPARSER_H
#pragma once
#include <Variant/Variant.h>
#include <algorithm>
#include <iosfwd>
namespace libvariant {


	enum ArgParseAction {
		ARGACTION_UNKNOWN, // Unknown action (invalid)
		ARGACTION_STORE, // Interpret the option argument and store it at the path
		ARGACTION_STORE_CONST, // Store const into the path
		ARGACTION_STORE_TRUE, // Store true into the path
		ARGACTION_STORE_FALSE, // Store false into the path
		ARGACTION_APPEND, // Append the option argument to the list at path
		ARGACTION_COUNT, // Make the value at path be the count of this options occurrences
		ARGACTION_HELP, // Print out the help text and exit
		ARGACTION_ADVHELP, // Print the advanced help and exit
		ARGACTION_CONFIG, // Consider the option argument a filename to be loaded and parsed as a config
		ARGACTION_CONFIG_JSON, // force parser to use jscn
		ARGACTION_CONFIG_YAML, // force parser to use yaml
		ARGACTION_CONFIG_PLIST, // force parser to use xmlplist
		ARGACTION_PRINT_CONFIG // At the end of parsing print out the resulting config and exit.
	};

	// A constrained list of only the valid types for ArgParse
	enum ArgParseType {
		ARGTYPE_GUESS = Variant::NullType, // No type specification, guess, uses deserialize machinery
		ARGTYPE_INT = Variant::IntegerType, // Error if it cannot be turned into an integer
		ARGTYPE_UINT = Variant::UnsignedType, // Error if it cannot be turned into a unsigned
		ARGTYPE_FLOAT = Variant::FloatType, // Error if it cannot be turned into a float
		ARGTYPE_STRING = Variant::StringType, // store as string
		ARGTYPE_BOOL = Variant::BoolType // Error if it cannot be turned into a boolean type
			// it treats 0 or the strings 'false' and 'no' or any substring ignoring case as false
			// and everything else as true
	};

	//! Valid values are:
	// This is used for converting the enum to a Variant and back for
	// ArgParse::GetDescription and ArgParse::Merge
	// store -- store the value into the key (default)
	// store_const -- store const (see Const) into the key
	// store_true -- store true into the key (i.e. no option argument)
	// store_false -- store false into the key
	// append -- make the value a list of instances of the option
	// count -- count the number of instances of this argument
	// help -- display the help text and exit
	// config -- load a file and merge it into the result (uses GuessFormat)
	// config_json -- same as config, force to use json parser
	// config_yaml -- same as config, force to use yaml parser
	// config_plist -- same as config, force to use xml-plist parser
	// config_print -- when finished parsing, print out the config and exit
	ArgParseAction ArgParseActionFromStr(const std::string &action);
	std::string ArgParseActionToStr(ArgParseAction action);

	class ArgParse;
	struct ArgParseImpl;
	struct ArgParseOptImpl;
	struct ArgParseGroupImpl;

	//! ArgParseOption is a part of class ArgParse.
	// ArgParseOption is the interface that ArgParse provides to the user to be able
	// to conveniently set the options for an option.
	class ArgParseOption {
	public:

		//! Set the title of this option. The title is used
		// as an argument placeholder string.
		ArgParseOption &Title(const std::string &title);

		//! What action to perform when this option is encountered.
		// See ArgParseAction above for a list of actions.
		ArgParseOption &Action(ArgParseAction action);
		DEPRECATED ArgParseOption &Action(const std::string &action);

		//! Set the type parsing and validation.
		// If set and the argument is not of this type, then it is invalid.
		// Default: ARGTYPE_GUESS
		// If type is ARGTYPE_GUESS then ArgParse attempts to parse the
		// argument with the same machinery that is used by the deserializers
		// to figure out the type of a scalar.
		// ARGTYPE_UINT is the same as ARGTYPE_INT but with
		// Minimum set to 0.
		// See ArgParseType for the list of valid types.
		ArgParseOption &Type(ArgParseType type);
		// Same as above but throws on invalid type
		DEPRECATED ArgParseOption &Type(Variant::Type_t type);
		// String version of the above
		// Valid types are "string", "number", "integer", "boolean", or "null".
		// Default type is "null".
		DEPRECATED ArgParseOption &Type(const std::string &type);

		//! Set the const value for "store_const"
		ArgParseOption &Const(Variant val);

		//! Set the default value used when the option is not set.
		ArgParseOption &Default(Variant val);

		//! Set the environment variable name to check for a value for this option.
		// When the environment variable is present this acts like Default was set
		// to the environment variable's value. I.e., this overrides default,
		// command line arguments override this.
		ArgParseOption &Env(const char *env);

		//! If the resulting value is a number, require to be at least this minimum.
		ArgParseOption &Minimum(Variant val);

		//! If the resulting value is a number, require to be at most this maximum.
		ArgParseOption &Maximum(Variant val);

		//! Add choices.
		// If a choice is added, the value of the option must be one of the choices.
		ArgParseOption &AddChoice(Variant val);

		//! Set choices (old ones forgotten) from a vector.
		ArgParseOption &SetChoices(const std::vector<Variant> &val);

		//! Return the vector of choices set.
		const std::vector<Variant> &GetChoices();

		//! Is this option required?
		ArgParseOption &Required(bool req=true);

		//! Set the help text for this option.
		ArgParseOption &Help(const std::string &helptxt);

		//! If this option is a string, it must match the regex in pattern.
		ArgParseOption &Pattern(const std::string &pattern);

		//! If this option is a string, it must be no longer than maxLength.
		ArgParseOption &MaxLength(unsigned l);

		//! If this option is a string, it must be no shorter than minLength.
		ArgParseOption &MinLength(unsigned l);

		//! If this option can be specified multiple times, it must be
		// specified minArgs times at least.
		// A value of 0 means zero arguments is valid.
		ArgParseOption &MinArgs(unsigned l);

		//! If this option can be specified multiple times, it cannot be
		// specified more than maxArgs.
		// A value of 0 means any number
		ArgParseOption &MaxArgs(unsigned l);

		//! Specify that this argument must be present l times.
		// Sets both minArgs and maxArgs to l
		ArgParseOption &NumArgs(unsigned l);

		//! Adds this option to group gname. If gname does not exist,
		//it will be created with an empty title and description.
		ArgParseOption &Group(const std::string &gname);

		//! Sets this argument to be an advanced option
		// This hides the option when not printing advanced options
		ArgParseOption &Advanced(bool adv = true);

	private:
		ArgParseOption(ArgParseOptImpl *i) : impl(i) {}
		ArgParseOptImpl *impl;
		friend class ArgParse;
	};

	//! Groups are a way to organize the option and to provide mutual exclusion
	//of option.  An argument may be part of more than one group, but may only
	//be part of one group that has a non-empty title and description.  An
	//attempt is made to order the help text so that options within the same
	//group appear together and if a group has a title and/or description the
	//options are placed within a subsection headed with the title and
	//description.
	class OptionGroup {
	public:
		//! Set the title for this group
		OptionGroup &Title(const std::string &t);
		//! Set the description for this group
		OptionGroup &Description(const std::string &d);
		//! Set that the options in this group are mutually exclusive
		OptionGroup &MutuallyExclusive(bool exclusive=true);
		//! Set that at least one option in this group is required.
		//Has no meaning when the group is not mutually exclusive.
		OptionGroup &Required(bool req=true);

		OptionGroup &Add(const std::string &keypath);
		OptionGroup &Add(const Path &keypath);

		//! Sets this option group to be advanced
		// Hide the group when not printing advanced options.
		// This only hides the group when it has a title or description
		OptionGroup &Advanced(bool adv=true);
	private:
		OptionGroup(ArgParseGroupImpl *i) : impl(i) {}
		ArgParseGroupImpl *impl;
		friend class ArgParse;
	};

	class ArgParse {
	public:
		//! addhelp true causes AddHelp to be called.
		ArgParse(bool addhelp=true);
		ArgParse(const std::string &pname, bool addhelp=true);

		//! Add the option and configure it as an option that takes an argument.
		//The default action will be "store" and type will be unspecified
		//
		// keypath must be a valid path as defined in Path.h
		//
		// For no short option, shortOpt may be 0, and for no long option, longOpt may be
		// the empty string. If both shortOpt is 0 and longOpt is empty then this option
		// is hidden unless Env is also set for the option.
		ArgParseOption AddOption(const std::string &keypath, int shortOpt, const std::string &longOpt="",
				const std::string &helptxt="");

		//! Convenience methods for getting an existing option.
		// ArgParse args(argv[0]);
		// args.AddOption("key1", 'k', "key1", "how many things").Type("integer");
		// args.AddOption("key2", 'K', "key2", "what to name it").Type("string");
		// (... some intermediate code ...)
		// args.At("key1").Default(42);
		// args["key2"].Default("foo");
		ArgParseOption At(const std::string &keypath);
		ArgParseOption operator[](const std::string &keypath) { return At(keypath); }

		//! Add the option and configure it as an option that is default false and
		//if present sets to true
		//i.e., same as:
		//	AddOption(keypath, shortOpt, longOpt, helptxt).Type("boolean")
		//		.Action("store_true").Default(false);
		ArgParseOption AddFlag(const std::string &keypath, int shortOpt, const std::string &longOpt="",
				const std::string &helptxt="");
		
		//! Adds the group name.
		OptionGroup AddGroup(const std::string &name, const std::string &title="", const std::string &description="");
		//! Returns the group name.
		OptionGroup GetGroup(const std::string &name);

		//! Add a positional argument.
		// By default the new positional argument will have NumArgs as 1.
		// Arguments are processed in the order they are added.
		// If maxArgs is 0, then no positional arguments after this one will
		// be processed.
		// Default action is store.
		ArgParseOption AddArgument(const std::string &keypath, const std::string &helptxt="");

		//! Convenience methods for getting an existing argument.
		// ArgParse args(argv[0]);
		// args.AddArgument("arg0", "pick one").AddChoice("AAA").AddChoice("BBB");
		// args.AddArgument("arg1", "scale factor").Type("float");
		// (... some intermediate code ...)
		// args.At(0).AddChoice("CCC");
		// args[1].Default(0.456);
		ArgParseOption At(size_t argumentIndex);
		ArgParseOption operator[](size_t argumentIndex) { return At(argumentIndex); }

		//! When this is on, trying to add a second option with the same short option
		//will cause the second option to act as though it had no short option specified.
		void ConflictResolution(bool resolve=true);

		//! Add a --config and --print-config option
		// --config is set with action ARGACTION_CONFIG and
		// --print-config is set with action ARGACTION_PRINT_CONFIG
		void AddConfigDefaults();

		//! Add -h,--help option with action ARGACTION_HELP
		void AddHelp();

		//! Set the description text that follows the usage message.
		void SetDescription(const std::string &txt);
		//! Set the epilog text at the end of help.
		void SetEpilog(const std::string &txt);
		//! Override the default program name.
		void SetProgramName(const std::string &pname, bool do_basename=true);
		std::string GetProgramName() const;

		void Merge(Variant newopts);
		void Merge(const ArgParse &other);

		//! Validate the argparse description against the internal schema
		void Validate() const;

		//! Parse the arguments with the internal description
		Variant Parse(int argc, char *const *argv);

		//! Print the help message and exit.
		void PrintHelp(bool advanced=false) const;

		//! Return the Variant that describes the arguments 
		Variant GetDescription() const;

		//! Generate a schema for the Variant returned by Parse from the argument description
		Variant GenerateSchema() const;

	private:
		shared_ptr<ArgParseImpl> impl;
	};
}
#endif
