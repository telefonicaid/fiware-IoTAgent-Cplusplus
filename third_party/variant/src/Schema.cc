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
 * \brief 
 *
 * This code is based on the public domain JSONv4 validator tv4.
 * tv4 is available at http://geraintluff.github.com/tv4/
 */

#include <Variant/Schema.h>
#include <Variant/Path.h>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <regex.h>
#include <string.h>
#include <strings.h>
#include <limits>
#include <cmath>
#include <stdlib.h>

#define FLOAT_TOLERANCE 100*std::numeric_limits<long double>::epsilon()
#define RECURSION_LIMIT 100

namespace libvariant {

	struct StopValidation : public std::exception {
		virtual const char *what() const throw() { return "stop validation"; }
	};

	struct SchemaContext {
		SchemaContext(const SchemaContext &o)
		   	: data_path(o.data_path),
			schema_path(o.schema_path),
			depth(o.depth),
			loader(o.loader)
		{}

		SchemaContext(SchemaLoader *l)
			: depth(0),
			loader(l)
		{}

		void AddError(const std::string &m) { AddError(ValidationError(m, data_path, schema_path)); }
		void AddError(const std::string &m, const SchemaContext &o) {
		   	ValidationError err(m, data_path, schema_path);
			err.AddSubError(o.result);
			AddError(err);
	   	}

		void AddError(const ValidationError &err) {
		   	result.AddError(err);
			throw StopValidation();
	   	}

		void AddSchemaError(const std::string &m) { AddSchemaError(SchemaError(m, schema_path)); }
		void AddSchemaError(const SchemaError &err) { result.AddSchemaError(err); }

		void IncrDepth() {
			depth += 1;
			if (depth > RECURSION_LIMIT) {
				std::ostringstream oss;
				oss << "SchemaValidate reached recursion limit.";
				AddSchemaError(oss.str());
				throw StopValidation();
			}
		}

		void DecrDepth() {
			depth -= 1;
		}

		Path data_path;
		Path schema_path;
		unsigned depth;
		SchemaResult result;
		SchemaLoader *loader;
	};

	struct AutoDataPath {
		AutoDataPath(SchemaContext &c, const PathElement &p) : ctx(c) { ctx.data_path.push_back(p); }
		~AutoDataPath() { ctx.data_path.pop_back(); }
		SchemaContext &ctx;
	};

	struct AutoSchemaPath {
		AutoSchemaPath(SchemaContext &c, const PathElement &p) : ctx(c) { ctx.schema_path.push_back(p); }
		~AutoSchemaPath() { ctx.schema_path.pop_back(); }
		SchemaContext &ctx;
	};

	struct AutoDepth {
		AutoDepth(SchemaContext &c) : ctx(c) { ctx.IncrDepth(); }
		~AutoDepth() { ctx.DecrDepth(); }
		SchemaContext &ctx;
	};

	void SchemaValidate(SchemaContext &ctx, Variant schema, Variant data);

	void ValidateNot(SchemaContext &ctx, Variant schema, Variant data) {
		if (!schema.Contains("not")) { return; }
		AutoSchemaPath spath(ctx, "not");
		SchemaContext notctx(ctx);
		try {
			SchemaValidate(notctx, schema["not"], data);
		} catch (const StopValidation &) {
			return;
		}
		if (notctx.result.Error()) { return; }
		ctx.AddError("Data matches schema from \"not\"");
	}

	void ValidateOneOf(SchemaContext &ctx, Variant schema, Variant data) {
		if (!schema.Contains("oneOf")) { return; }
		AutoSchemaPath spath(ctx, "oneOf");
		if (!schema["oneOf"].IsList()) {
			ctx.AddSchemaError("\"oneOf\" must be a list of schemas");
			return;
		}
		ValidationError error("Data does not match any schemas from \"oneOf\"",
			   	ctx.data_path, ctx.schema_path);
		unsigned num_validates = 0;
		unsigned index = 0;
		unsigned validIndex = 0;
		for (Variant::ListIterator i(schema["oneOf"].ListBegin()), e(schema["oneOf"].ListEnd());
				i != e; ++i, ++index)
		{
			SchemaContext subctx(ctx);
			try {
				AutoSchemaPath spath(subctx, index);
				SchemaValidate(subctx, *i, data);
				if (!subctx.result.Error()) { num_validates++; }
			} catch (const StopValidation &) {}
			if (subctx.result.Error()) { error.AddSubError(subctx.result); }
			if (num_validates == 1) {
				validIndex = index;
			} else if (num_validates > 1) {
				std::ostringstream oss;
				oss << "Data is valid against more than one schema from \"oneOf\": indeces "
					<< validIndex << " and " << index;
				ctx.AddError(oss.str());
			}
		}
		if (num_validates == 0) {
			ctx.AddError(error);
		}
	}

	void ValidateAnyOf(SchemaContext &ctx, Variant schema, Variant data) {
		if (!schema.Contains("anyOf")) { return; }
		AutoSchemaPath spath(ctx, "anyOf");
		if (!schema["anyOf"].IsList()) {
			ctx.AddSchemaError("\"anyOf\" must be a list of schemas");
			return;
		}
		ValidationError error("Data does not match any schema from \"anyOf\"",
			   	ctx.data_path, ctx.schema_path);
		unsigned index = 0;
		for (Variant::ListIterator i(schema["anyOf"].ListBegin()), e(schema["anyOf"].ListEnd());
				i != e; ++i, ++index)
		{
			SchemaContext subctx(ctx);
			try {
				AutoSchemaPath spath(subctx, index);
				SchemaValidate(subctx, *i, data);
				if (!subctx.result.Error()) { return; }
			} catch (const StopValidation &) {}
			error.AddSubError(subctx.result);
		}
		ctx.AddError(error);
	}

	void ValidateAllOf(SchemaContext &ctx, Variant schema, Variant data) {
		if (!schema.Contains("allOf")) { return; }
		AutoSchemaPath spath(ctx, "allOf");
		if (!schema["allOf"].IsList()) {
			ctx.AddSchemaError("\"allOf\" must be a list of schemas");
			return;
		}
		unsigned index = 0;
		for (Variant::ListIterator i(schema["allOf"].ListBegin()), e(schema["allOf"].ListEnd());
				i != e; ++i, ++index)
		{
			AutoSchemaPath spath(ctx, index);
			SchemaValidate(ctx, *i, data);
		}
	}

	void ValidateCombinations(SchemaContext &ctx, Variant schema, Variant data) {
		ValidateAllOf(ctx, schema, data);
		ValidateAnyOf(ctx, schema, data);
		ValidateOneOf(ctx, schema, data);
		ValidateNot(ctx, schema, data);
	}

	void ValidateObjectDependenciesHelper(SchemaContext &ctx, const std::string &dep, Variant data) {
		if (!data.Contains(dep)) {
			std::ostringstream oss;
			oss << "Dependency failed - key must exist: " << dep;
			ctx.AddError(oss.str());
		}
	}

	void ValidateObjectDependencies(SchemaContext &ctx, Variant schema, Variant data) {
		if (schema.Contains("dependencies")) {
			Variant deps = schema["dependencies"];
			AutoSchemaPath spath(ctx, "dependencies");
			if (!deps.IsMap()) {
				ctx.AddSchemaError("Schema key \"dependencies\" must be an object.");
				return;
			}
			for (Variant::MapIterator i(deps.MapBegin()), e(deps.MapEnd()); i != e; ++i) {
				if (data.Contains(i->first)) {
					AutoSchemaPath spath(ctx, i->first);
					if (i->second.IsString()) {
						ValidateObjectDependenciesHelper(ctx, i->second.AsString(), data);
					} else if (i->second.IsList()) {
						for (unsigned j = 0; j < i->second.Size(); ++j) {
							AutoSchemaPath spath(ctx, j);
							ValidateObjectDependenciesHelper(ctx, i->second[j].AsString(), data);
						}
					} else if (i->second.IsMap()) {
						SchemaValidate(ctx, i->second, data);
					} else {
						ctx.AddSchemaError("Values for dependencies must be either a key, a list of"
								" keys, or a schema.");
					}
				}
			}
		}
	}

	// Helper class for ValidateObjectProperties
	class PatternProperty {
	public:
		PatternProperty(const std::string &patternkey, Variant schm)
			: key(patternkey), schema(schm)
		{
			memset(&pattern, 0, sizeof(regex_t));
			int err;
			if ((err = regcomp(&pattern, key.c_str(), REG_EXTENDED | REG_NOSUB))) {
				std::vector<char> buffer(8192);
				regerror(err, &pattern, &buffer[0], buffer.size());
				std::ostringstream oss;
				oss << "Failed to compile patternProperty regex \"" << key << "\" with error: "
					<< &buffer[0];
				throw std::runtime_error(oss.str());
			}
		}
		~PatternProperty() {
			regfree(&pattern);
		}
		bool Test(const std::string &k) {
			return regexec(&pattern, k.c_str(), 0, 0, 0) == 0;
		}
		Variant GetSchema() { return schema; }
		std::string GetKey() { return key; }
	private:
		// Can't tell if it is safe to copy regex_t structures...
		PatternProperty(const PatternProperty &);
		std::string key;
		regex_t pattern;
		Variant schema;
	};

	void ValidateObjectProperties(SchemaContext &ctx, Variant schema, Variant data) {
		// Precompile because we always match against all of them for every key
		std::vector< shared_ptr<PatternProperty> > patternProperties;
		if (schema.Contains("patternProperties")) {
			AutoSchemaPath spath(ctx, "patternProperties");
			if (schema["patternProperties"].IsMap()) {
				for (Variant::MapIterator i(schema["patternProperties"].MapBegin()),
						e(schema["patternProperties"].MapEnd()); i != e; ++i)
				{
					try {
						patternProperties.push_back(
								shared_ptr<PatternProperty>(
									new PatternProperty(i->first, i->second)
									)
								);
					} catch (const std::runtime_error &e) {
						AutoSchemaPath spath(ctx, i->first);
						ctx.AddSchemaError(e.what());
					}
				}
			} else {
				ctx.AddSchemaError("\"patternProperties\" must be an object");
			}
		}
		Variant properties = schema.Get("properties", Variant::MapType);
		if (!properties.IsMap()) {
			ctx.AddSchemaError("\"properties\" must be an object");
			properties = Variant::MapType;
		}
		for (Variant::MapIterator i(data.MapBegin()), e(data.MapEnd());
				i != e; ++i)
		{
			bool matchFound = false;
			AutoDataPath dpath(ctx, i->first);
			// Validate against property declarations
			if (properties.Contains(i->first)) {
				AutoSchemaPath spath(ctx, "properties");
				AutoSchemaPath spath2(ctx, i->first);
				matchFound = true;
				SchemaValidate(ctx, properties[i->first], i->second);
			}
			// Validate against all matching patternProperties declarations
			for (unsigned j = 0; j < patternProperties.size(); ++j) {
				if (patternProperties[j]->Test(i->first)) {
					AutoSchemaPath spath(ctx, "patternProperties");
					AutoSchemaPath spath2(ctx, patternProperties[j]->GetKey());
					matchFound = true;
					SchemaValidate(ctx, patternProperties[j]->GetSchema(), i->second);
				}
			}
			// If no match was found for either of the previous cases validate
			// against additionalProperties
			if (!matchFound && schema.Contains("additionalProperties")) {
				Variant additionalProps = schema["additionalProperties"];
				AutoSchemaPath spath(ctx, "additionalProperties");
				if (additionalProps.IsBool()) {
					if (!additionalProps.AsBool()) {
						ctx.AddError("Additional properties not allowed.");
					}
				} else {
					SchemaValidate(ctx, additionalProps, i->second);
				}
			}
		}
	}

	void ValidateObjectRequiredProperties(SchemaContext &ctx, Variant schema, Variant data) {
		if (schema.Contains("required")) {
			unsigned index = 0;
			Variant required = schema["required"];
			AutoSchemaPath spath(ctx, "required");
			if (!required.IsList()) {
				ctx.AddSchemaError("\"required\" must be a list of required keys");
				return;
			}
			for (Variant::ListIterator i(required.ListBegin()), e(required.ListEnd());
					i != e; ++i, ++index)
			{
				AutoSchemaPath spath(ctx, index);
				if (i->IsString()) {
					std::string key = i->AsString();
					if (!data.Contains(key)) {
						std::ostringstream oss;
						oss << "Missing required property: " << key;
						ctx.AddError(oss.str());
					}
				} else {
					ctx.AddSchemaError("Elements of the \"required\" list must be strings");
				}
			}
		}
		// TODO: Add v3 compatibility by checking required keys in properties?
	}

	void ValidateObjectMinMaxProperties(SchemaContext &ctx, Variant schema, Variant data) {
		if (schema.Contains("minProperties")) {
			AutoSchemaPath spath(ctx, "minProperties");
			if (schema["minProperties"].IsNumber()) {
				uintmax_t minprops = schema["minProperties"].AsUnsigned();
				if (data.Size() < minprops) {
					std::ostringstream oss;
					oss << "Too few properties defined (" << data.Size() << "), minimum " << minprops;
					ctx.AddError(oss.str());
				}
			} else {
				ctx.AddSchemaError("\"minProperties\" must be a number");
			}
		}
		if (schema.Contains("maxProperties")) {
			AutoSchemaPath spath(ctx, "maxProperties");
			if (schema["maxProperties"].IsNumber()) {
				uintmax_t maxprops = schema["maxProperties"].AsUnsigned();
				if (data.Size() > maxprops) {
					std::ostringstream oss;
					oss << "Too many properties defined (" << data.Size() << "), maximum " << maxprops;
					ctx.AddError(oss.str());
				}
			} else {
				ctx.AddSchemaError("\"minProperties\" must be a number");
			}
		}
	}

	void ValidateObject(SchemaContext &ctx, Variant schema, Variant data) {
		if (!data.IsMap() || data.IsNull()) { return; }
		ValidateObjectMinMaxProperties(ctx, schema, data);
		ValidateObjectRequiredProperties(ctx, schema, data);
		ValidateObjectProperties(ctx, schema, data);
		ValidateObjectDependencies(ctx, schema, data);
	}

	void ValidateArrayItems(SchemaContext &ctx, Variant schema, Variant data) {
		if (!schema.Contains("items")) { return; }
		if (!data.IsList()) { return; }
		if (schema["items"].IsList()) {
			Variant items = schema["items"];
			unsigned datalength = data.Size();
			unsigned itemlength = items.Size();
			for (unsigned i(0); i < datalength; ++i) {
				AutoDataPath dpath(ctx, i);
				if (i < itemlength) {
					AutoSchemaPath spath(ctx, "items");
					AutoSchemaPath spath2(ctx, i);
					SchemaValidate(ctx, items[i], data[i]);
				} else if (schema.Contains("additionalItems")) {
					AutoSchemaPath spath(ctx, "additionalItems");
					if (schema["additionalItems"].IsBool()) {
						if (!schema["additionalItems"].AsBool()) {
							ctx.AddError("Additional items not allowed");
						}
					} else {
						SchemaValidate(ctx, schema["additionalItems"], data[i]);
					}
				}
			}
		} else {
			unsigned datalength = data.Size();
			AutoSchemaPath spath(ctx, "items");
			for (unsigned i(0); i < datalength; ++i) {
				AutoDataPath dpath(ctx, i);
				SchemaValidate(ctx, schema["items"], data[i]);
			}
		}
	}

	void ValidateArrayUniqueItems(SchemaContext &ctx, Variant schema, Variant data) {
		if (!schema.Contains("uniqueItems")) { return; }
		AutoSchemaPath spath(ctx, "uniqueItems");
		if (!schema["uniqueItems"].IsBool()) {
			ctx.AddSchemaError("\"uniqueItems\" must be a boolean");
			return;
		}
		if (!schema["uniqueItems"].AsBool()) { return; }
		unsigned length = data.Size();
		for (unsigned i(0); i < length; ++i) {
			for (unsigned j(i+1); j < length; ++j) {
				if (data[i].Comparable(data[j]) && data[i] == data[j]) {
					std::ostringstream oss;
					oss << "Array items are not unique (indices " << i << " and " << j << ")";
					ctx.AddError(oss.str());
				}
			}
		}
	}

	void ValidateArrayLength(SchemaContext &ctx, Variant schema, Variant data) {
		if (schema.Contains("minItems")) {
			AutoSchemaPath spath(ctx, "minItems");
			if (schema["minItems"].IsNumber()) {
				uintmax_t minItems = schema["minItems"].AsUnsigned();
				if (data.Size() < minItems) {
					std::ostringstream oss;
					oss << "Array is to short (" << data.Size() << "), minimum " << minItems;
					ctx.AddError(oss.str());
				}
			} else {
				ctx.AddSchemaError("\"minItems\" must be a number");
			}
		}
		if (schema.Contains("maxItems")) {
			AutoSchemaPath spath(ctx, "maxItems");
			if (schema["maxItems"].IsNumber()) {
				uintmax_t maxItems = schema["maxItems"].AsUnsigned();
				if (data.Size() > maxItems) {
					std::ostringstream oss;
					oss << "Array is to long (" << data.Size() << "), maximum " << maxItems;
					ctx.AddError(oss.str());
				}
			} else {
				ctx.AddSchemaError("\"maxItems\" must be a number");
			}
		}
	}

	void ValidateArray(SchemaContext &ctx, Variant schema, Variant data) {
		if (!data.IsList()) { return; }
		ValidateArrayLength(ctx, schema, data);
		ValidateArrayUniqueItems(ctx, schema, data);
		ValidateArrayItems(ctx, schema, data);
	}

	void ValidateStringFormat(SchemaContext &ctx, Variant schema, Variant data) {
		if (!schema.Contains("format")) { return; }
		AutoSchemaPath spath(ctx, "format");
		if (!schema["format"].IsString()) { return; }
		std::string format = schema["format"].AsString();
		regex_t pattern;
		memset(&pattern, 0, sizeof(regex_t));
		std::string patternstr;
		int err;
		if (format == "regex") {
			// From V3 spec
			err = regcomp(&pattern, data.AsString().c_str(), REG_EXTENDED | REG_NOSUB);
			if (err) {
				std::ostringstream oss;
				oss << "String \"" << data.AsString() << "\" not a valid regular expression.";
				ctx.AddError(oss.str());
			}
			regfree(&pattern);
		} else {
			if (format == "date-time") {
				// From V4 spec
				// RFC 3339, section 5.6
				patternstr = "^[0-9][0-9][0-9][0-9](-[0-1][0-9](-[0-3][0-9]"
					"(T[0-9][0-9](:[0-9][0-9](:[0-9][0-9])?)?)?)?)?Z?$";
			} else if (format == "date") {
				// From V3 spec: should be format of YYYY-MM-DD
				patternstr = "^[0-9][0-9][0-9][0-9]-[0-1][0-9]-[0-3][0-9]$";
			} else if (format == "time") {
				// From V3 spec: should be format of hh:mm:ss
				patternstr = "^[0-2][0-9]:[0-5][0-9]:[0-5][0-9]$";
			} else if (format == "email") {
				// RFC 5322, section 3.4.1
				// TODO
			} else if (format == "hostname") {
				// RFC 1034, section 3.1
				// TODO
			} else if (format == "ipv4") {
				// RFC 2676, section 3.2
				// TODO
			} else if (format == "ipv6") {
				// RFC 2373, section 2.2
				// TODO
			} else if (format == "uri") {
				// RFC 3986
				// TODO
			}

			if (!patternstr.empty()) {
				if ((err = regcomp(&pattern, patternstr.c_str(), REG_EXTENDED | REG_NOSUB))) {
					std::vector<char> buffer(8192);
					regerror(err, &pattern, &buffer[0], buffer.size());
					std::ostringstream oss;
					oss << "Failed to compile format regex \"" << patternstr << "\" with error: "
						<< &buffer[0];
					ctx.AddSchemaError(oss.str());
					return;
				}
				try {
					if (regexec(&pattern, data.AsString().c_str(), 0, 0, 0)) {
						ctx.AddError("String does not match format " + format);
					}
				} catch (...) {
					regfree(&pattern);
					throw;
				}
				regfree(&pattern);
			}
		}
	}

	void ValidateStringPattern(SchemaContext &ctx, Variant schema, Variant data) {
		if (!schema.Contains("pattern")) { return; }
		AutoSchemaPath spath(ctx, "pattern");
		if (!schema["pattern"].IsString()) {
			ctx.AddSchemaError("\"pattern\" must be a string");
			return;
		}
		regex_t pattern;
		memset(&pattern, 0, sizeof(regex_t));
		std::string patternstr = schema["pattern"].AsString();
		int err;
		if ((err = regcomp(&pattern, patternstr.c_str(), REG_EXTENDED | REG_NOSUB))) {
			std::vector<char> buffer(8192);
			regerror(err, &pattern, &buffer[0], buffer.size());
			std::ostringstream oss;
			oss << "Failed to compile pattern regex \"" << patternstr << "\" with error: "
				<< &buffer[0];
			ctx.AddSchemaError(oss.str());
			return;
		}
		try {
			if (regexec(&pattern, data.AsString().c_str(), 0, 0, 0)) {
				std::ostringstream oss;
				oss << "String does not match pattern \""
					<< patternstr << "\"";
				ctx.AddError(oss.str());
			}
		} catch (...) {
			regfree(&pattern);
			throw;
		}
		regfree(&pattern);
	}

	void ValidateStringLength(SchemaContext &ctx, Variant schema, Variant data) {
		if (schema.Contains("minLength")) {
			AutoSchemaPath spath(ctx, "minLength");
			if (schema["minLength"].IsNumber()) {
				// TODO: Get encoded length rather than byte length?
				if (data.Size() < schema["minLength"].AsUnsigned()) {
					std::ostringstream oss;
					oss << "String is too short (" << data.Size() << " chars), minimum "
						<< schema["minLength"].AsUnsigned();
					ctx.AddError(oss.str());
				}
			} else {
				ctx.AddSchemaError("\"minLength\" must be a number");
			}
		}
		if (schema.Contains("maxLength")) {
			AutoSchemaPath spath(ctx, "maxLength");
			if (schema["maxLength"].IsNumber()) {
				// TODO: Get encoded length rather than byte length?
				if (data.Size() > schema["maxLength"].AsUnsigned()) {
					std::ostringstream oss;
					oss << "String is too long (" << data.Size() << " chars), maximum "
						<< schema["maxLength"].AsUnsigned();
					ctx.AddError(oss.str());
				}
			} else {
				ctx.AddSchemaError("\"maxLength\" must be a number");
			}
		}
	}

	void ValidateString(SchemaContext &ctx, Variant schema, Variant data) {
		if (!data.IsString()) { return; }
		ValidateStringLength(ctx, schema, data);
		ValidateStringPattern(ctx, schema, data);
		ValidateStringFormat(ctx, schema, data);
	}

	void ValidateMinMax(SchemaContext &ctx, Variant schema, Variant data) {
		if (schema.Contains("minimum")) {
			if (schema["minimum"].IsNumber()) {
				if (data < schema["minimum"]) {
					AutoSchemaPath spath(ctx, "minimum");
					std::ostringstream oss;
					oss << "Value " << data.AsString()
						<< " is less than minimum " << schema["minimum"].AsString();
					ctx.AddError(oss.str());
				}
				if (schema.Contains("exclusiveMinimum") && schema["exclusiveMinimum"].AsBool()
						&& data == schema["minimum"]) {
					AutoSchemaPath spath(ctx, "exclusiveMinimum");
					std::ostringstream oss;
					oss << "Value " << data.AsString() << " is equal to exclusive minimum "
						<< schema["minimum"].AsString();
					ctx.AddError(oss.str());
				}
			} else {
				AutoSchemaPath spath(ctx, "minimum");
				ctx.AddSchemaError("\"minimum\" must be a number");
			}
		}
		if (schema.Contains("maximum")) {
			if (schema["maximum"].IsNumber()) {
				if (data > schema["maximum"]) {
					AutoSchemaPath spath(ctx, "maximum");
					std::ostringstream oss;
					oss << "Value " << data.AsString()
						<< " is greater than maximum " << schema["maximum"].AsString();
					ctx.AddError(oss.str());
				}
				if (schema.Contains("exclusiveMaximum") && schema["exclusiveMaximum"].AsBool()
						&& data == schema["maximum"]) {
					AutoSchemaPath spath(ctx, "exclusiveMaximum");
					std::ostringstream oss;
					oss << "Value " << data.AsString()
						<< " is equal to exclusive maximum " << schema["maximum"].AsString();
					ctx.AddError(oss.str());
				}
			} else {
				AutoSchemaPath spath(ctx, "maximum");
				ctx.AddSchemaError("\"maximum\" must be a number");
			}
		}
	}

	void ValidateMultipleOf(SchemaContext &ctx, Variant schema, Variant data) {
		std::string multkey;
		if (schema.Contains("multipleOf")) {
			multkey = "multipleOf";
		} else if (schema.Contains("divisibleBy")) {
			multkey = "divisibleBy";
		} else {
			return;
		}
		AutoSchemaPath spath(ctx, multkey);
		Variant multipleof = schema[multkey];
		if (!multipleof.IsNumber() || multipleof < 0) {
			std::ostringstream oss;
			oss << "\"" << multkey << "\" must be a positive number";
			ctx.AddSchemaError(oss.str());
			return;
		}
		Variant mod = data % multipleof;
		if (data.IsFloat() || multipleof.IsFloat()) {
			// We have an issue here of the fact that many numeric values in base 10 are
			// not exactly represented in binary floating point.
			// So, we need to check if it is within epsilon of 0
			long double m = mod.AsLongDouble();
			long double divisor = multipleof.AsLongDouble();
			long double tolerance = FLOAT_TOLERANCE;

			if (m > tolerance && std::abs(m - divisor) > tolerance) {
				std::ostringstream oss;
				oss << "Value " << data.AsString() << " is not a multiple of " << multipleof.AsString();
				//oss << "\nmodulus: " << m << "\n|modulus-divisor|: " << std::abs(m - divisor)
				//	<< "\ntolerance " << tolerance;
				ctx.AddError(oss.str());
			}
		} else if (mod != 0) {
			std::ostringstream oss;
			oss << "Value " << data.AsString() << " is not a multiple of " << multipleof.AsString();
			ctx.AddError(oss.str());
		}
	}

	void ValidateNumeric(SchemaContext &ctx, Variant schema, Variant data) {
		if (!data.IsNumber()) { return; }
		ValidateMultipleOf(ctx, schema, data);
		ValidateMinMax(ctx, schema, data);
	}

	void ValidateEnum(SchemaContext &ctx, Variant schema, Variant data) {
		if (!schema.Contains("enum")) { return; }
		Variant enm = schema["enum"];
		AutoSchemaPath spath(ctx, "enum");
		if (!enm.IsList()) {
			ctx.AddSchemaError("\"enum\" must be an array");
			return;
		}
		for (Variant::ListIterator i(enm.ListBegin()), e(enm.ListEnd()); i!=e; ++i) {
			if (i->Comparable(data) && *i == data) { return; }
		}
		std::ostringstream oss;
		oss << "No enum match for: ";
		SerializeJSON(oss.rdbuf(), data);
		ctx.AddError(oss.str());
	}

	void ValidateType(SchemaContext &ctx, Variant schema, Variant data) {
		if (!schema.Contains("type")) { return; }
		AutoSchemaPath spath(ctx, "type");
		std::vector<Variant> allowed_types;
		if (schema["type"].IsList()) {
			allowed_types = schema["type"].AsList();
		} else {
			// TODO: v3 compatibility
			// if (schema["type"] == "any") { return; }
			allowed_types.push_back(schema["type"]);
		}
		std::ostringstream oss;
		unsigned index = 0;
		for (Variant::ListIterator i(allowed_types.begin()), e(allowed_types.end());
				i != e; ++i, ++index) {
			if (*i == "array") {
				if (data.IsList()) { return; }
			} else if (*i == "boolean") {
				if (data.IsBool()) { return; }
			} else if (*i == "integer") {
				if (data.IsInt() || data.IsUnsigned()) { return; }
				if (data.IsFloat() && data % 1 == 0) { return; }
			} else if (*i == "null") {
				if (data.IsNull()) { return; }
			} else if (*i == "number") {
				if (data.IsNumber()) { return; }
			} else if (*i == "object") {
				if (data.IsMap()) { return; }
			} else if (*i == "string") {
				if (data.IsString()) { return; }
			} else {
				AutoSchemaPath spath(ctx, index);
				std::ostringstream oss;
				oss << "Invalid schema type: \"" << i->AsString() << "\"";
				ctx.AddSchemaError(oss.str());
			}
		}
		oss << "invalid data type: \"" << data.GetType() << "\" expected";
		if (allowed_types.size() == 1) {
			oss << ": \"" << allowed_types.front().AsString() << "\"";
		} else {
			oss << " one of: [";
			for (Variant::ListIterator i(allowed_types.begin()), e(allowed_types.end());
					i != e; ++i) {
				oss << "\"" <<  i->AsString() << "\"";
				if (i+1 != e) { oss << ", "; }
			}
			oss << "]";
		}
		ctx.AddError(oss.str());
	}

	void ValidateBasic(SchemaContext &ctx, Variant schema, Variant data) {
		ValidateType(ctx, schema, data);
		ValidateEnum(ctx, schema, data);
	}

	void SchemaValidate(SchemaContext &ctx, Variant schema, Variant data) {
		AutoDepth depth(ctx);
		// Empty schema always validates
		if (!schema.IsMap()) { return; }
		if (schema.Contains("$ref") && schema["$ref"].IsString()) {
			SchemaValidate(ctx, ctx.loader->GetSchema(schema["$ref"].AsString()), data);
		}
		ValidateBasic(ctx, schema, data);
		ValidateNumeric(ctx, schema, data);
		ValidateString(ctx, schema, data);
		ValidateArray(ctx, schema, data);
		ValidateObject(ctx, schema, data);
		ValidateCombinations(ctx, schema, data);
	}

	SchemaResult SchemaValidate(Variant schema, Variant data, SchemaLoader *loader)
	{
		SchemaLoader lloader(loader);
		if (schema.IsNull() && data.IsMap() && data.Contains("$schema")) {
			schema["$ref"] = data["$schema"];
		}
		if (schema.IsString()) {
			schema = lloader.GetSchema(schema.AsString());
		}
		lloader.AddSchema("", schema);
		SchemaContext ctx(&lloader);
		try {
			SchemaValidate(ctx, schema, data);
		} catch (const StopValidation &) {}
		return ctx.result;
	}

	void AddSchemaDefaultsImpl(Variant schema, VariantRef data, SchemaLoader *loader);

	void AddSchemaListDefaultsImpl(VariantRef slist, VariantRef data, SchemaLoader *loader) {
		if (slist.Exists() && slist.IsList()) {
			for (Variant::ListIterator i(slist.ListBegin()), e(slist.ListEnd()); i != e; ++i) {
				AddSchemaDefaultsImpl(*i, data, loader);
			}
		}
	}

	void AddSchemaDefaultsImpl(Variant schema, VariantRef data, SchemaLoader *loader) {
		if (!schema.IsMap()) { return; }
		if (schema.Contains("$ref") && schema["$ref"].IsString()) {
			AddSchemaDefaultsImpl(loader->GetSchema(schema["$ref"].AsString()), data, loader);
		}
		if (schema.Contains("default") && (!data.Exists() || data.IsNull())) {
			data = schema["default"].Copy();
			return;
		}
		if (schema.Contains("properties") && (!data.Exists() || data.IsMap())) {
			for (Variant::MapIterator i(schema["properties"].MapBegin()), e(schema["properties"].MapEnd());
					i != e; ++i)
			{
				AddSchemaDefaultsImpl(i->second, data.At(i->first), loader);
			}
		}
		if (schema.Contains("items") && (!data.Exists() || data.IsList())) {
			if (schema["items"].IsList()) {
				for (unsigned i = 0; i < schema["items"].Size(); ++i)
				{
					AddSchemaDefaultsImpl(schema["items"][i], data[i], loader);
				}
			} else {
				AddSchemaDefaultsImpl(schema["items"], data[0], loader);
			}
		}
		AddSchemaListDefaultsImpl(schema["allOf"], data, loader);
		AddSchemaListDefaultsImpl(schema["oneOf"], data, loader);
		AddSchemaListDefaultsImpl(schema["anyOf"], data, loader);
	}

	void AddSchemaDefaults(Variant schema, VariantRef data, SchemaLoader *loader) {
		SchemaLoader lloader(loader);
		if (schema.IsNull() && data.IsMap() && data.Contains("$schema")) {
			schema["$ref"] = data["$schema"];
		}
		if (schema.IsString()) {
			schema = lloader.GetSchema(schema.AsString());
		}
		lloader.AddSchema("", schema);
		AddSchemaDefaultsImpl(schema, data, &lloader);
	}

	void InsertIndent(std::ostream &os, unsigned indent) {
		for (unsigned i = 0; i < indent; ++i) { os << "\t"; }
	}

	ValidationError::ValidationError(const std::string &m, const Path &dpath,
				const Path &spath)
		: message(m),
		data_path(dpath),
		schema_path(spath)
	{}

	void ValidationError::AddSubError(const SchemaResult &err) {
		sub_errors.push_back(err);
	}

	std::string ValidationError::GetDataPathStr() const {
		return PathString(data_path);
	}

	std::string ValidationError::GetSchemaPathStr() const {
		return PathString(schema_path);
	}

	std::string ValidationError::PrettyPrintMessage() const {
		std::ostringstream oss;
		PrettyPrintMessage(oss);
		return oss.str();
	}

	void ValidationError::PrettyPrintMessage(std::ostream &os, unsigned indent) const {
		InsertIndent(os, indent);
		os << "validation error: " << message << "\n";
		InsertIndent(os, indent);
		os << "data path: " << GetDataPathStr() << "\n";
		InsertIndent(os, indent);
		os << "schema path: " << GetSchemaPathStr() << "\n";
		if (!sub_errors.empty()) {
			InsertIndent(os, indent);
			os << "sub errors:\n";
			for (unsigned i = 0; i < sub_errors.size(); ++i) {
				sub_errors[i].PrettyPrintMessage(os, indent+1);
			}
		}
	}

	SchemaError::SchemaError(const std::string &m, const Path &spath)
		: message(m),
		schema_path(spath)
   	{
	}

	std::string SchemaError::GetSchemaPathStr() const {
		return PathString(schema_path);
	}

	std::string SchemaError::PrettyPrintMessage() const {
		std::ostringstream oss;
		PrettyPrintMessage(oss);
		return oss.str();
	}

	void SchemaError::PrettyPrintMessage(std::ostream &os, unsigned indent) const {
		InsertIndent(os, indent);
		os << "schema error: " << message << "\n";
		InsertIndent(os, indent);
		os << "schema path: " << GetSchemaPathStr() << "\n";
	}

	SchemaResult::SchemaResult()
	{
	}

	void SchemaResult::AddError(const ValidationError &err) {
		errors.push_back(err);
	}

	void SchemaResult::AddSchemaError(const SchemaError &err) {
		schema_errors.push_back(err);
	}

	std::string SchemaResult::PrettyPrintMessage() const {
		std::ostringstream oss;
		PrettyPrintMessage(oss);
		return oss.str();
	}

	void SchemaResult::PrettyPrintMessage(std::ostream &os, unsigned indent) const {
		if (!errors.empty()) {
			for (unsigned i = 0; i < errors.size(); ++i) {
				errors[i].PrettyPrintMessage(os, indent);
			}
		}
		if (!schema_errors.empty()) {
			for (unsigned i = 0; i < schema_errors.size(); ++i) {
				schema_errors[i].PrettyPrintMessage(os, indent);
			}
		}
	}

}
