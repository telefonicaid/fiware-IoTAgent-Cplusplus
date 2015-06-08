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
 */

#include <Variant/SchemaLoader.h>
#include <Variant/Extensions.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdexcept>
#include <sstream>
#include <string.h>
#ifdef ENABLE_CURL
#include <curl/curl.h>
#endif

#if 0
#include <iostream>
#define DBPRINT(a) std::cerr << a
#else
#define DBPRINT(a)
#endif

namespace libvariant {

	extern const char JSON_SCHEMA_V4_SCHEMA[];
	class SchemaStaticData {
	public:
		SchemaStaticData() {
			JSON_SCHEMA_SCHEMA = DeserializeJSON(JSON_SCHEMA_V4_SCHEMA);
			SCHEMA_URI = SchemaLoader::ResolveURI(JSON_SCHEMA_SCHEMA["id"].AsString(), "");
			SchemaLoader::NormSchema(JSON_SCHEMA_SCHEMA, SCHEMA_URI);
		}
		Variant JSON_SCHEMA_SCHEMA;
		std::string SCHEMA_URI;
	};
	shared_ptr<SchemaStaticData> schema_static_data;

	SchemaStaticData *GetSchemaStaticData() {
		// This is actually a race condition when multithreading is used, but
		// every thread will write the same data, so this is "OK" as long as
		// the CPU cache coherency is reasonable.
		if (!schema_static_data.get()) {
			shared_ptr<SchemaStaticData> new_schema_static_data;
			new_schema_static_data.reset(new SchemaStaticData);
			schema_static_data = new_schema_static_data;
		}
		return schema_static_data.get();
	}

	SchemaLoader::SchemaLoader()
		: missingHandler(0), 
		missingctx(0),
		base(0)
   	{
		SchemaStaticData *d = GetSchemaStaticData();
		schemas[d->SCHEMA_URI] = d->JSON_SCHEMA_SCHEMA;
	}

	SchemaLoader::SchemaLoader(SchemaLoader *b)
		: missingHandler(0),
		missingctx(0),
		base(b)
	{
		DBPRINT("schema loader\n");
		SchemaStaticData *d = GetSchemaStaticData();
		schemas[d->SCHEMA_URI] = d->JSON_SCHEMA_SCHEMA;
	}

	SchemaLoader::~SchemaLoader() {}

	std::string UnescapeURI(const std::string &uri) {
		std::string ret;
		const char *c = uri.c_str();
		const char *e = c + uri.size();
		while (c != e) {
			switch (*c) {
			case '%':
				if (c + 1 == e || c + 2 == e) {
					ret.append(c);
					c = e;
				} else {
					char hex[3] = { *(c+1), *(c+2), 0 };
					char *endp = 0;
					long hexv = strtol(hex, &endp, 16);
					if (*endp != 0) {
						ret.append(c, 3);
					} else {
						ret.append(1, (char)hexv);
					}
					c += 3;
				}
				break;
			default:
				ret.append(c, 1);
				++c;
				break;
			}
		}
		return ret;
	}

	Variant SchemaLoader::GetSchema(const std::string &uri) {
		Variant::MapIterator iter = schemas.find(uri);
		if (iter != schemas.end()) {
			return iter->second;
		}
		std::string baseuri = ResolveURI(uri, "");
		std::string fragment;
		std::string::size_type hashIndex = uri.find("#");
		if (hashIndex != std::string::npos) {
			fragment = uri.substr(hashIndex + 1);
			baseuri = uri.substr(0, hashIndex);
		}
		iter = schemas.find(baseuri);
		if (iter == schemas.end()) {
			if (base) {
				return base->GetSchema(uri);
			}
			LoaderError err;
			if (!HandleMissing(baseuri, err)) {
				throw std::runtime_error(err.GetErrorMessage());
			}
			iter = schemas.find(baseuri);
		}
		if (iter == schemas.end()) {
			return Variant::NullType;
		}

		Variant schema = iter->second;
		return JSONPointerLookup(UnescapeURI(fragment), schema);
	}

	std::map<std::string, Variant> SchemaLoader::AddSchema(const std::string &uri, Variant schema) {
		std::string luri = ResolveURI(uri, "");
		std::map<std::string, Variant> m;
		m[luri] = schema;
		NormSchema(schema, luri);
		SearchForTrustedSchemas(m, schema, luri);
		schemas.insert(m.begin(), m.end());
		return m;
	}

	void SchemaLoader::ForgetSchema(const std::string &uri) {
		schemas.erase(uri);
	}

	bool SchemaLoader::HandleMissing(const std::string &uri, LoaderError &error) {
		if (missingHandler) {
			return missingHandler(this, uri, error, missingctx);
		} else if (base) {
			return base->HandleMissing(uri, error);
		} else {
			error.AddError(uri, "SchemaLoader missing schema");
			return false;
		}
	}

	void SchemaLoader::SetMissingHandler(MissingHandlerType f, void *ctx) {
		missingHandler = f;
		missingctx = ctx;
	}

	// The following two functions are adapted from two functions in tv4 with the following comment:
	// parseURI() and resolveUrl() are from https://gist.github.com/1088850
	//	 -	released as public domain by author ("Yaffle") - see comments on gist

	bool SchemaLoader::ParseURI(URIInfo &info, const std::string &uri) {
		enum {
		HREF=0, PROTOCOL, AUTHORITY, USERHOST, HOST,
		HOSTNAME, PORT, PATHNAME, SEARCH, HASH, MATCHLEN
		};
		static const struct RegexHelper {
		RegexHelper() {
			memset(&regex, 0, sizeof(regex_t));
			int ret = regcomp(&regex,
"^[[:space:]]*([^:\\/?#]+:)?" // PROTOCOL
"(//([^:@]*(:[^:@]*)?@)?([^:/?#]*)(:[0-9]*)?)?" // (//((user(:host)@)(hostname)(:port)))
"([^?#]*)" // PATHNAME
"(\\?[^#]*)?" // SEARCH
"(#[^[:space:]]*)?[[:space:]]*$" // HASH
					, REG_EXTENDED);
			if (ret) {
				std::vector<char> buffer(8192);
				regerror(ret, &regex, &buffer[0], buffer.size());
				std::ostringstream oss;
				oss << "libvariant library initialization failure: Failed to initialize schema uri handling regex with error: "
					<< &buffer[0];
				fputs(oss.str().c_str(), stderr);
				throw std::runtime_error(oss.str());
			}
		}
		~RegexHelper() { regfree(&regex); }
		regex_t regex;
		} regex;
		regmatch_t match[MATCHLEN];
		DBPRINT(std::endl << "Matching against \"" << uri << "\"\n");
		int err = regexec(&regex.regex, uri.c_str(), MATCHLEN, &match[0], 0);
		if (err != 0) { 
			DBPRINT("Match failed\n");
			return false;
	   	}
		for (int i = 0; i < MATCHLEN; ++i) {
			int so, eo, len;
			if (match[i].rm_so >= 0) {
				so = match[i].rm_so;
				eo = match[i].rm_eo;
				len = eo - so;
				DBPRINT("Match ");
				switch (i) {
				case HREF:
					info.href = uri.substr(so, len);
					DBPRINT("href\t");
					break;
				case PROTOCOL:
					info.protocol = uri.substr(so, len);
					DBPRINT("protocol\t");
					break;
				case AUTHORITY:
					info.authority = uri.substr(so, len);
					DBPRINT("authority\t");
					break;
				case PATHNAME:
					info.pathname = uri.substr(so, len);
					DBPRINT("pathname\t");
					break;
				case SEARCH:
					info.search = uri.substr(so, len);
					DBPRINT("search\t");
					break;
				case HASH:
					info.hash = uri.substr(so, len);
					DBPRINT("hash\t");
					break;
				default:
					break;
				}
				DBPRINT(i << " matched \"" << uri.substr(so, len) << "\"" << std::endl);
			} else {
				DBPRINT(i << " did not match" << std::endl);
			}
		}
		return true;
	}

	std::string RemoveDotSegments(const std::string &input) {
		//^(\.\.?(\/|$))+/, '' -- ignore leading ../ removed
		//\/(\.(\/|$))+/g, '/' -- remove /./
		//\/\.\.$/, '/../' -- normalize last /.. to /../
		//\/?[^\/]*/g, function (p) {
		std::vector<std::string> output;
		std::string::size_type i = 0;
		std::string::size_type next = input.find("/");
		bool firstslash = false;
		DBPRINT("Processing \"" << input << "\"\n");
		unsigned lastdot = 0;
		if (next == 0) {
			// First was a slash, make sure to add it back
			firstslash = true;
			DBPRINT("firstslash\n");
		} else if (next >= input.size()) {
			// No slashes
			DBPRINT("no slashes\n");
			if (input == ".") { return ""; }
			else { return input; }
		//} else if (next + 1 == input.size()) {
			// only slash at end
			//if (input == "../" || input == "./") { return ""; }
			//else { return input; }
		} else {
			std::string sub = input.substr(i, next - i);
			if (sub != ".") {
				output.push_back(sub);
				if (sub == "..") { lastdot = output.size(); }
				DBPRINT("leading: \"" << output.back() << "\"\n");
			}
		}
		while (next < input.size()) {
			i = next;
			next = input.find("/", i + 1);
			// Get the string between the slashes
			std::string sub = input.substr(i + 1, next - 1 - i);
			if (sub == ".") {
				lastdot = output.size();
			} else if (sub == "..") {
				DBPRINT("pop\n");
				if (!output.empty()) { output.pop_back(); }
				else { output.push_back(sub); }
				lastdot = output.size();
			} else {
				output.push_back(sub);
				DBPRINT("push back: \"" << output.back() << "\"\n");
			}
		}
		std::ostringstream oss;
		for (std::vector<std::string>::iterator iv(output.begin()), ie(output.end()); iv != ie; ++iv) {
			if (firstslash) {
				oss << "/";
			} else {
				firstslash = true;
			}
			oss << *iv;
		}
		if (output.empty() && firstslash) { oss << "/"; }
		if (lastdot > 0 && lastdot == output.size()) { oss << "/"; }
		DBPRINT("ret: \"" << oss.str() << "\"");
		return oss.str();
	}

	std::string SchemaLoader::ResolveURI(const std::string &base, const std::string &href) {
		URIInfo b, h;
		bool base_parsed = ParseURI(b, base);
		bool href_parsed = ParseURI(h, href);
		std::string ret = "";
		if (!base_parsed || !href_parsed) {
			return ret;
		}

		ret += (h.protocol != "" ? h.protocol : b.protocol);
		ret += (h.protocol != "" || h.authority != "" ? h.authority : b.authority);
		if (h.protocol != "" || h.authority != "" || h.pathname[0] == '/') {
			ret += RemoveDotSegments(h.pathname);
		} else {
			if (h.pathname == "") {
				ret += RemoveDotSegments(b.pathname);
			} else {
				ret += RemoveDotSegments(
						(b.authority != "" && b.pathname == "" ? "/" : "")
						+ b.pathname.substr(0, b.pathname.rfind("/") + 1)
						+ h.pathname
						);
			}
		}
		if (h.protocol != "" || h.authority != "" || h.pathname != "" || h.search != "") {
			ret += h.search;
		} else {
			ret += b.search;
		}
		ret += h.hash;
		return ret;
	}

	void SchemaLoader::NormSchema(Variant &schema, const std::string &uri) {
		std::string baseuri = uri;
		if (schema.IsMap()) {
			if (schema.Contains("id") && schema["id"].IsString()) {
				baseuri = ResolveURI(baseuri, schema["id"].AsString());
				schema["id"] = baseuri;
			}
			if (schema.Contains("$ref") && schema["$ref"].IsString()) {
				schema["$ref"] = ResolveURI(baseuri, schema["$ref"].AsString());
			} else {
				for (Variant::MapIterator i(schema.MapBegin()), e(schema.MapEnd()); i != e; ++i) {
					if (i->first != "enum") {
						NormSchema(i->second, baseuri);
					}
				}
			}
		} else if (schema.IsList()) {
			for (Variant::ListIterator i(schema.ListBegin()), e(schema.ListEnd()); i != e; ++i) {
				NormSchema(*i, baseuri);
			}
		}
	}

	void SchemaLoader::SearchForTrustedSchemas(std::map<std::string, Variant> &m,
			Variant schema, const std::string &uri)
	{
		if (schema.IsMap()) {
			if (schema.Contains("id") && schema["id"].IsString()) {
				std::string id = schema["id"].AsString();
				if (id.substr(0, uri.length()) == uri) {
					std::string remainder = id.substr(uri.length());
					if ((uri.length() > 0 && uri[uri.length()-1] == '/')
							|| remainder[0] == '#'
							|| remainder[0] == '?')
					{
						if (m.find(id) == m.end()) {
							m[id] = schema;
						}
					}
				}
			}

			for (Variant::MapIterator i(schema.MapBegin()), e(schema.MapEnd()); i != e; ++i) {
				if (i->first != "enum" && i->second.IsMap()) {
					SearchForTrustedSchemas(m, i->second, uri);
				}
			}
		}
	}

	SchemaLoader::LoaderError::LoaderError() {
		erross << "SchemaLoader error:\n";
	}

	SchemaLoader::LoaderError::~LoaderError() {}

	void SchemaLoader::LoaderError::AddError(const std::string &uri, const std::string &msg) {
		erross << "\t" << uri << ": " << msg << "\n";
	}

	void AdvSchemaLoader::AddPath(const std::string &path) {
		paths.push_back(path);
	}

	bool AdvSchemaLoader::HandleMissing(const std::string &uri, LoaderError &error) {
		URIInfo info;
		if (uri.empty()) {
			return SchemaLoader::HandleMissing(uri, error);
		} else if (ParseURI(info, uri)) {
			if (info.href[0] == '/' || info.href[0] == '.') {
				// Assume file system path.
				if (!LoadFromFile(info, error)) {
					return SchemaLoader::HandleMissing(uri, error);
				}
				return true;
			} else if (info.protocol.empty()) {
				if (!LoadFromFile(info, error) && !LoadFromCurl(info, error)) {
					return SchemaLoader::HandleMissing(uri, error);
				}
				return true;
			} else if (info.protocol == "file:") {
				if (!LoadFromFile(info, error)) {
					return SchemaLoader::HandleMissing(uri, error);
				}
				return true;
			} else {
				if (!LoadFromCurl(info, error)) {
					return SchemaLoader::HandleMissing(uri, error);
				}
				return true;
			}
		}
		return SchemaLoader::HandleMissing(uri, error);
	}

	bool AdvSchemaLoader::LoadFromFile(URIInfo &info, LoaderError &error) {
		// Normalize the path..
		std::string path;
		if (!info.authority.empty()) {
			if 	(
					!info.protocol.empty()
				   	&& info.authority.size() >= 2
					&& info.authority[0] == '/'
				   	&& info.authority[1] == '/'
				)
		   	{
				path = info.authority.substr(2);
			} else {
				path = info.authority;
			}
		}
		path += info.pathname;
		if (path[0] == '/') {
			// Absolute path
			try {
				AddSchema(info.href, DeserializeGuessFile(path.c_str()));
				return true;
			} catch (const std::exception &e) {
				error.AddError(path.c_str(), e.what());
				return false;
			}
		}
		// Not absolute
		for (std::vector<std::string>::iterator i(paths.begin()), e(paths.end()); i != e; ++i) {
			std::string uri = *i + "/" + path;
			try {
				AddSchema(info.href, DeserializeGuessFile(uri.c_str()));
				return true;
			} catch (const std::exception &e) {
				error.AddError(uri, e.what());
			}
		}
		return false;
	}

#ifdef ENABLE_CURL

	size_t CurlContextWrite(char *ptr, size_t size, size_t nmemb, void *userdata);

	class CurlContext {
	public:

		CurlContext(SchemaLoader::URIInfo &info)
			: self(curl_easy_init())
	   	{
			if (!self) {
				std::ostringstream oss;
				oss << "Unable to initialize curl to fetch uri: " << info.href;
				throw std::runtime_error(oss.str());
			}
			CURLcode err = curl_easy_setopt(self, CURLOPT_NOSIGNAL, 1);
			if (err != CURLE_OK) { Error(err); }
			err = curl_easy_setopt(self, CURLOPT_WRITEFUNCTION, &CurlContextWrite);
			if (err != CURLE_OK) { Error(err); }
			err = curl_easy_setopt(self, CURLOPT_WRITEDATA, (void*)this);
			if (err != CURLE_OK) { Error(err); }
			uri = info.protocol + info.authority + info.pathname + info.search;
			err = curl_easy_setopt(self, CURLOPT_URL, uri.c_str());
			if (err != CURLE_OK) { Error(err); }
		}

		~CurlContext() {
			curl_easy_cleanup(self);
		}

		size_t Write(char *ptr, size_t len) {
			data_buff.insert(data_buff.end(), ptr, ptr + len);
			return len;
		}

		void Perform() {
			CURLcode err = curl_easy_perform(self);
			if (err != CURLE_OK) { Error(err); }
		}

		void Error(CURLcode err) {
			std::ostringstream oss;
			oss << "Error attempting to fetch uri: " << uri << "\n";
			oss << curl_easy_strerror(err);
			throw std::runtime_error(oss.str());
		}

		CURL *self;
		std::string uri;
		std::vector<char> data_buff;
		std::vector<char> err_buff;
	};

	size_t CurlContextWrite(char *ptr, size_t size, size_t nmemb, void *userdata) {
		return ((CurlContext*)userdata)->Write(ptr, size*nmemb);
	}

	bool AdvSchemaLoader::LoadFromCurl(URIInfo &info, LoaderError &error) {
		try {
			CurlContext ctx(info);
			ctx.Perform();
			AddSchema(ctx.uri, DeserializeGuess(&ctx.data_buff[0], ctx.data_buff.size()));
			return true;
		} catch (const std::exception &e) {
			error.AddError(info.href, e.what());
			return false;
		}
	}

#else
	bool AdvSchemaLoader::LoadFromCurl(URIInfo &info, LoaderError &error) {
		error.AddError(info.href, "Curl support not compiled in to load urls.");
		return false;
	}
#endif
}
