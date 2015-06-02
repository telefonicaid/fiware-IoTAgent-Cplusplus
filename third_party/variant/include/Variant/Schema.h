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
 * \brief A schema validator.
 *
 * This code is based on the public domain JSONv4 validator tv4.
 * tv4 is available at http://geraintluff.github.com/tv4/
 */
#ifndef VARIANT_SCHEMA_H
#define VARIANT_SCHEMA_H
#pragma once
#include <Variant/Variant.h>
#include <map>
#include <iosfwd>
#include <deque>
#include <sstream>
namespace libvariant {

	class SchemaResult;

	class ValidationError {
	public:
		ValidationError(const std::string &m, const Path &dpath,
				const Path &spath);

		void AddSubError(const SchemaResult &err);

		const std::string &GetMessage() const { return message; }
		const Path &GetDataPath() const { return data_path; }
		std::string GetDataPathStr() const;
		const Path &GetSchemaPath() const { return schema_path; }
		std::string GetSchemaPathStr() const;
		const std::vector<SchemaResult> &GetSubErrors() const { return sub_errors; }

		std::string PrettyPrintMessage() const;
		void PrettyPrintMessage(std::ostream &os, unsigned indent=0) const;
	private:
		std::string message;
		Path data_path;
		Path schema_path;
		std::vector<SchemaResult> sub_errors;
	};

	class SchemaError {
	public:
		SchemaError(const std::string &m, const Path &spath);

		const std::string &GetMessage() const { return message; }
		const Path &GetSchemaPath() const { return schema_path; }
		std::string GetSchemaPathStr() const;

		std::string PrettyPrintMessage() const;
		void PrettyPrintMessage(std::ostream &os, unsigned indent=0) const;
	private:
		std::string message;
		Path schema_path;
	};

	class SchemaResult {
	public:
		SchemaResult();

		void AddError(const ValidationError &err);
		void AddSchemaError(const SchemaError &err);

		bool Error() const { return !errors.empty() || !schema_errors.empty(); }
		bool ValidationErrors() const { return !errors.empty(); }
		bool SchemaErrors() const { return !schema_errors.empty(); }

		const std::vector<ValidationError> &GetErrors() const { return errors; }
		const std::vector<SchemaError> &GetSchemaErrors() const { return schema_errors; }

		std::string PrettyPrintMessage() const;
		void PrettyPrintMessage(std::ostream &os, unsigned indent=0) const;
	private:
		std::vector<ValidationError> errors;
		std::vector<SchemaError> schema_errors;
	};

	inline std::ostream &operator<<(std::ostream &os, const SchemaResult &sr) {
		sr.PrettyPrintMessage(os);
		return os;
	}

	class SchemaLoader {
	public:
		class LoaderError {
		public:
			LoaderError();
			~LoaderError();
			void AddError(const std::string &uri, const std::string &msg);
			std::string GetErrorMessage() const { return erross.str(); }
		private:
			std::ostringstream erross;
		};

		typedef bool (*MissingHandlerType)(SchemaLoader*,const std::string&,LoaderError &,void*);
		SchemaLoader();
		SchemaLoader(SchemaLoader *b);
		virtual ~SchemaLoader();
		// Get the schema at the uri specified.
		// Internal lookup first, then call HandleMissing if the uri is not internally available.
		// If the schema then is still not available return Variant::NullType.
		virtual Variant GetSchema(const std::string &uri);
		// Add a schema to the loader at the uri specified.
		// returns the schema(s) added with the corresponding uri(s)
		virtual std::map<std::string, Variant> AddSchema(const std::string &uri, Variant schema);
		// Forget a schema with the specified uri.
		virtual void ForgetSchema(const std::string &uri);
		// Called when a schema is not in the loader.
		// Default implementation is to call the missing handler 
		// or add a not found error to the LoaderError and return false
		// Return true on success, otherwise false, any errors are filled into the LoaderError
		virtual bool HandleMissing(const std::string &uri, LoaderError &error);
		void SetMissingHandler(MissingHandlerType, void *ctx);

		static void NormSchema(Variant &schema, const std::string &uri);
		//!
		// A realpath function for schema URIs.
		// href is the "current working directory" and base is
		// possibly a relative path.
		static std::string ResolveURI(const std::string &base, const std::string &href);

		struct URIInfo {
			std::string href; // EX: the whole uri if matched
			std::string protocol; // EX: http://
			std::string authority; // user:host@hostname:port
			std::string pathname; // The path segment
			std::string search; // ?q=234
			std::string hash; // #tag
		};

		//! Break a uri into its components and store them into the URIInfo struct
		//! returns true if the parsing was successful
		static bool ParseURI(URIInfo &info, const std::string &uri);
	protected:

		static void SearchForTrustedSchemas(std::map<std::string, Variant> &m, Variant schema, const std::string &uri);
		MissingHandlerType missingHandler;
		void *missingctx;
		std::map<std::string, Variant> schemas;
		SchemaLoader *base;
	};

	//! Validate data against a schema.
	//!
	//! The schema parameter is either a schema, a string uri to a schema, or null.
	//! If it is a schema then data is validated against it.
	//! If it is a string then it calls loader->GetSchema()  and validates data against the result.
	//! If schema is null, then look for a key "$schema" in data, set schema to it and proceed as above.
	SchemaResult SchemaValidate(Variant schema, Variant data, SchemaLoader *loader=0);

	//! Add the defaults specified in schema to data if they do not exist.
	//! Ignores default in some combinations and types that could match multiple entries (not,
	//! additionalProperties, patternProperties, etc.)
	//! schema is processed the same as for SchemaValidate
	void AddSchemaDefaults(Variant schema, VariantRef data, SchemaLoader *loader=0);
}
#endif
