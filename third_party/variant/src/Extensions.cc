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
 * \author Josh Staley
 * \brief Useful extension functions for Variants
 */

#include <sstream>
#include <stdexcept>
#include <Variant/Variant.h>
#include <Variant/Extensions.h>
#include <Variant/Path.h>
#include <stdlib.h>
#include <ctype.h>
#include <set>

using namespace std;

#if 0
#include <iostream>
#define DBPRINT(x) cout << x
#else
#define DBPRINT(x)
#endif

namespace libvariant {

	bool SimpleList( const Variant & var ) {
		if ( !var.IsList() )
			return false;
		for ( unsigned i = 0; i < var.Size(); ++i )
			if ( var[i].IsMap() | var[i].IsList() )
				return false;
		return true;
	}

	Variant ExtensionParams() {
		Variant v;
		ExtensionParams(v);
		return v;
	}

	void ExtensionParams( VariantRef params ) {
		if ( params.IsNull() ) {
			params = Variant::MapType;
		}

		if ( !params.IsMap() ) {
			throw runtime_error("# ExtensionParams() - params must be a map");
		}

		if ( !(params.Contains("noSimpleLists") && params["noSimpleLists"].IsBool()) ) {
			params["noSimpleLists"] = true;
		}

		if ( !params.Contains("pathPrefix") ) {
			params["pathPrefix"] = "";
		}
		if ( !(params["pathPrefix"].IsString() && params["pathPrefix"].Size() <= 1) ) {
			throw runtime_error("# ExtensionParams() - Invalid pathPrefix");
		}

		if ( !params.Contains("pathDelimiter") ) {
			params["pathDelimiter"] = "/";
		}
		// Must have a delimiter
		if ( !(params["pathDelimiter"].IsString() && params["pathDelimiter"].Size() == 1) ) {
			throw runtime_error("# ExtensionParams() - Invalid pathDelimiter");
		}
	
		if ( !params.Contains("indexPrefix") ) {
			params["indexPrefix"] = "[";
		}
		// Must have a prefix
		if ( !(params["indexPrefix"].IsString() && params["indexPrefix"].Size() == 1) ) {
			throw runtime_error("# ExtensionParams() - Invalid indexPrefix");
		}

		if ( !params.Contains("indexSuffix") ) {
			params["indexSuffix"] = "]";
		}
		if ( !(params["indexSuffix"].IsString() && params["indexSuffix"].Size() <= 1) ) {
			throw runtime_error("# ExtensionParams() - Invalid indexSuffix");
		}

		if ( !params.Contains("listDelimiter") ) {
			params["listDelimiter"] = ",";
		}
		// Must have a delimiter
		if ( !(params["listDelimiter"].IsString() && params["listDelimiter"].Size() == 1)) {
			throw runtime_error("# ExtensionParams() - Invalid listDelimiter");
		}
	}

	Variant FlattenPath( const Variant &var, Variant params ) {
		Variant flat;
		FlattenPathTo( flat, var, params );
		return flat;
	}

	Variant ExpandPath( const Variant &var, Variant params ) {
		Variant expand;
		ExpandPathTo( expand, var, params );
		return expand;
	}

	void FlattenPathTo( VariantRef flat, const Variant &var, Variant params, string prefix ) {
		switch ( var.GetType() ) {
		case Variant::ListType:
			if ( (!params["noSimpleLists"].AsBool()) && SimpleList(var) ) {
				if( prefix.size() ) {
					flat[prefix] = var.Copy();
				} else {
					flat = var.Copy();
				}
			} else {
				for( unsigned i=0; i < var.Size(); ++i ) {
					stringstream tmp;
					if ( prefix.size() ) {
						tmp << prefix;
					}
					tmp << params["indexPrefix"].AsString();
					tmp << i;
					tmp << params["indexSuffix"].AsString();
					FlattenPathTo( flat, var[i], params, tmp.str() );
				}
			}
			break;
		case Variant::MapType:
			for( Variant::ConstMapIterator i(var.MapBegin()), e(var.MapEnd()); i != e; ++i ) {
				stringstream tmp;
				if ( prefix.size() ) {
					tmp << prefix + params["pathDelimiter"].AsString();
				} else {
					tmp << params["pathPrefix"].AsString();
				}
				tmp << i->first;
				FlattenPathTo( flat, i->second, params, tmp.str());
			}
			break;
		default:
			if( prefix.size() ) {
				flat[ prefix ] = var.Copy();
			} else {
				flat = var.Copy();
			}
		}
	}

	/// The current version of this funtion is fairly simple and cannot fully
	/// reverse the operation FlattenPathTo does on a variant with the same params
	/// argument. This primarily has to do with lists.
	void ExpandPathTo( VariantRef expand, const Variant &var, Variant params ) {
		if ( !var.IsMap() ) {
			expand = var;
			return;
		}

		for ( Variant::ConstMapIterator i(var.MapBegin()), e(var.MapEnd()); i != e; ++i ) {
			string path = i->first;
			string delimiter = params["pathDelimiter"].AsString();
			if ( delimiter.length() != 1 ) {
				throw runtime_error("# ExpandPathTo() - Delimiter is not a single char");
			}
			for ( unsigned ii = 0; ii < path.length(); ++ii ) {
				if (path[ii] == delimiter[0]) {
					path[ii] = '/';
				}
			}
			expand.SetPath( path, i->second );
		}
	}

	map< string, string > FlattenPathToStrStrMap ( const Variant &var, Variant params ) {
		map< string, string > flatMap;
		Variant flat = FlattenPath( var, params );

		for ( Variant::MapIterator i(flat.MapBegin()), e(flat.MapEnd()); i != e; ++i ) {
			stringstream tmp;

			switch (i->second.GetType()) {
			case Variant::ListType:
				for ( unsigned j = 0; j < i->second.Size(); ++j ) {
					if (j != 0){
						tmp << params["listDelimiter"].AsString();
					}
					tmp << i->second[j].AsString();
				}
				flatMap[i->first] = tmp.str();
				break;
			case Variant::BlobType:
				throw runtime_error("# FlattenToStrStrMap() - BlobType not supported");
			default:
				flatMap[i->first] = i->second.AsString();
			}
		}

		return flatMap;
	}

	Variant RDiff ( const Variant &base, const Variant &compare ) {
		Variant diff(Variant::MapType);
		RDiff( diff, base, compare );
		return diff;
	}

	void RDiff ( VariantRef diff, const Variant &base, const Variant &compare ) {
		switch ( compare.GetType() ) {
		case Variant::MapType:
			DBPRINT("Start of map" << endl);

			// Replace and break
			if ( !base.IsMap() ) {
				diff = compare.Copy();
				break;
			}

			for  ( Variant::ConstMapIterator i(compare.MapBegin()), e(compare.MapEnd()); i != e; ++i ) {
				DBPRINT(i->first << endl);

				// Replace and continue
				if ( !base.Contains(i->first) ) {
					diff[i->first] = i->second.Copy();
					continue;
				}

				// Recurse into map
				if ( base[i->first] != i->second ) {
					diff[i->first] = Variant::MapType;
					DBPRINT("recurse for " << i->first << endl);
					RDiff( diff[i->first], base[i->first], compare[i->first] );
				}

				// If map diff became null or an empty map, remove - TODO: should this be an option?
				if ( diff.Contains(i->first) ) {
					if ( (diff[ i->first ].GetType() == Variant::NullType) ||
						(diff[ i->first ].GetType() == Variant::MapType && diff[ i->first ].Size() == 0) ) {
							diff.Erase(i->first);
						}
				}
			}
			break;
		default:
			if( base != compare ) {
				diff = compare.Copy();
			}
		}
	}

	void RUpdate( VariantRef base, const Variant &update ) {
		switch ( update.GetType() ) {
		case Variant::NullType:
			return;
		case Variant::MapType:
			if ( !base.Exists() || base.IsMap() ) {
				for ( Variant::ConstMapIterator i(update.MapBegin()), e(update.MapEnd()); i != e; ++i ) {
					RUpdate(base.At( i->first, Variant::MapType ), update[ i->first ]);
				}
				break;
			}
			// else we go to default
		default:
			base = update.Copy();
		}
	}

	std::string JSONPointerUnescape(const std::string &fragment) {
		std::string ret;
		const char *c = fragment.c_str();
		const char *e = c + fragment.size();
		while (c != e) {
			switch (*c) {
			case '~':
				++c;
				if (c != e) {
					if (*c == '0') {
						ret.append("~");
					} else if (*c == '1') {
						ret.append("/");
					} else {
						ret.append("~");
						ret.append(c, 1);
					}
					++c;
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

	enum Operations_e {
		OP_ADD,
		OP_REMOVE,
		OP_TEST,
		OP_GET
	};

	struct Operation_t {
		Operation_t() {}
		Operation_t(Operations_e o, const std::string &p)
		   	: op(o), success(false), guess(false), replaced(false), pointer(p) {}

		Operations_e op;
		Path path; // path that modification /starts at/
		Variant value;
		Variant old_value;
		bool success;
		bool guess;
		bool replaced;
		std::string pointer;
	};

	static bool IsIndex(const std::string &str, uintmax_t &index) {
		const char *b = str.c_str();
		char *e = 0;
		unsigned long long v = strtoull(b, &e, 10);
		if (e == b) { return false; }
		while (e != 0 && *e != '\0' && isspace(*e)) { ++e; }
		if (e == 0 || *e != '\0') { return false; }
		index = v;
		return true;
	}

	VariantRef JSONPointerOp(const std::string &pointer, VariantRef v, Operation_t &op) {
		if (pointer.empty()) {
			switch (op.op) {
			case OP_ADD:
				if (v.Exists()) {
					op.replaced = true;
					op.old_value = v;
				}
				v = op.value;
				op.success = true;
				return v;
			case OP_REMOVE:
				op.value = v;
				v = Variant::NullType;
				op.success = true;
				return v;
			case OP_TEST:
				op.success = (v == op.value);
				return v;
			case OP_GET:
				op.value = v;
				op.success = true;
				return v;
			}
		}
		if (pointer[0] != '/') {
			throw std::runtime_error("JSON Pointer is required to start with '/'");
		}
		std::string::size_type nindex =  pointer.find('/', 1);
		std::string component = JSONPointerUnescape(pointer.substr(1, nindex - 1));
		bool is_last = (nindex >= pointer.size());
		if (!v.Exists()) {
			if (op.guess) {
				// Best guess
				uintmax_t index = 0;
				if (IsIndex(component, index)) {
					return JSONPointerOp(pointer.substr(nindex), v.At(index), op);
				} else {
					return JSONPointerOp(pointer.substr(nindex), v.At(component), op);
				}
			} else {
				throw std::runtime_error("Unable to resolve the JSON Pointer.");
			}
		} else if (v.IsMap()) {
			op.path.push_back(component);
			if (is_last) {
				switch (op.op) {
				case OP_ADD:
					if (v.Contains(component)) {
						op.replaced = true;
						op.old_value = v.Get(component);
					}
					v.Set(component, op.value);
					op.success = true;
					return v.At(component);
				case OP_REMOVE:
					op.value = v.At(component);
					v.Erase(component);
					op.success = true;
					return v.At(component);
				case OP_TEST:
					{
						VariantRef ret = v.At(component);
						op.success = (ret == op.value);
						return ret;
					}
				case OP_GET:
					{
						VariantRef ret = v.At(component);
						op.value = ret;
						op.success = true;
						return ret;
					}
				}
			} else {
				return JSONPointerOp(pointer.substr(nindex), v.At(component), op);
			}
		} else if (v.IsList()) {
			if (is_last && component == "-") {
				op.path.push_back(v.Size());
				if (op.op == OP_ADD) {
					v.Append(op.value);
					op.success = true;
					return v.At(v.Size());
				}
				throw std::runtime_error("Attempting to reference past the end of an array");
			}
			uintmax_t index = 0;
			if (!IsIndex(component, index)) {
				throw std::runtime_error("Attempting to index an array with a string");
			}
			op.path.push_back(index);
			if (is_last) {
				switch (op.op) {
				case OP_ADD:
					if (v.Contains(index)) {
						op.old_value = v.Get(index);
						op.replaced = true;
					}
					v.AsList().insert(v.ListBegin()+index, op.value);
					op.success = true;
					return v.At(index);
				case OP_REMOVE:
					op.value = v.At(component);
					v.Erase(index);
					op.success = true;
					return v.At(index);
				case OP_TEST:
					{
						VariantRef ret = v.At(index);
						op.success = (ret == op.value);
						return ret;
					}
				case OP_GET:
					{
						VariantRef ret = v.At(index);
						op.value = ret;
						op.success = true;
						return ret;
					}
				}
			} else {
				return JSONPointerOp(pointer.substr(nindex), v.At(index), op);
			}
		}
		std::ostringstream oss;
		oss << "The JSON Pointer does not match the document, "
			<< v.GetType() << " is not a indexable type";
		throw std::runtime_error(oss.str());
	}

	void JSONPointerAdd(const std::string &pointer, VariantRef v, Variant data) {
		Operation_t op(OP_ADD, pointer);
		op.value = data;
		try {
			JSONPointerOp(op.pointer, v, op);
		} catch (const std::exception &e) {
			std::ostringstream oss;
			oss << "Error adding a value with the JSON Pointer \""
				<< op.pointer << "\" at path \""
				<< PathString(op.path) << "\": " << e.what();
			throw std::runtime_error(oss.str());
		}
	}

	void JSONPointerRemove(const std::string &pointer, VariantRef v) {
		Operation_t op(OP_REMOVE, pointer);
		try {
			JSONPointerOp(op.pointer, v, op);
		} catch (const std::exception &e) {
			std::ostringstream oss;
			oss << "Error removing a value with the JSON Pointer \""
				<< op.pointer << "\" at path \""
				<< PathString(op.path) << "\": " << e.what();
			throw std::runtime_error(oss.str());
		}
	}

	void JSONPointerReplace(const std::string &pointer, VariantRef v, Variant data) {
		Operation_t op(OP_GET, pointer);
		try {
			if (JSONPointerOp(op.pointer, v, op).Exists()) {
				v.SetPath(op.path, data);
			} else {
				throw std::runtime_error("The target does not exist");
			}
		} catch (const std::exception &e) {
			std::ostringstream oss;
			oss << "Error replacing a value with the JSON Pointer \""
				<< op.pointer << "\" at path \""
				<< PathString(op.path) << "\": " << e.what();
			throw std::runtime_error(oss.str());
		}
	}

	void JSONPointerMove(const std::string &src, const std::string &dst, VariantRef v) {
		Operation_t op(OP_REMOVE, src);
		try {
			JSONPointerOp(op.pointer, v, op);
			Variant data = op.value;
			Path path = op.path;
			try {
				op = Operation_t(OP_ADD, dst);
				op.value = data;
				JSONPointerOp(op.pointer, v, op);
			} catch (const std::exception &e) {
				v.SetPath(path, data);
				throw e;
			}
		} catch (const std::exception &e) {
			std::ostringstream oss;
			oss << "Error moving a value with the JSON Pointer \""
				<< op.pointer << "\" at path \""
				<< PathString(op.path) << "\": " << e.what();
			throw std::runtime_error(oss.str());
		}
	}

	void JSONPointerCopy(const std::string &src, const std::string &dst, VariantRef v) {
		Operation_t op(OP_GET, src);
		try {
			VariantRef s = JSONPointerOp(op.pointer, v, op);
			if (s.Exists()) {
				op = Operation_t(OP_ADD, dst);
				op.value = s.Copy();
				JSONPointerOp(op.pointer, v, op);
			} else {
				throw std::runtime_error("The src does not exist");
			}
		} catch (const std::exception &e) {
			std::ostringstream oss;
			oss << "Error copying a value with the JSON Pointer \""
				<< op.pointer << "\" at path \""
				<< PathString(op.path) << "\": " << e.what();
			throw std::runtime_error(oss.str());
		}
	}

	VariantRef JSONPointerLookup(const std::string &pointer, VariantRef v) {
		Operation_t op(OP_GET, pointer);
		try {
			return JSONPointerOp(op.pointer, v, op);
		} catch (const std::exception &e) {
			std::ostringstream oss;
			oss << "Error looking up a value with the JSON Pointer \""
				<< op.pointer << "\" at path \""
				<< PathString(op.path) << "\": " << e.what();
			throw std::runtime_error(oss.str());
		}
	}

	std::string FormJSONPointer(const Path &path) {
		std::ostringstream ret;
		for (Path::const_iterator i(path.begin()), e(path.end()); i != e; ++i) {
			if (i->IsNumber()) {
				ret << "/" << i->AsUnsigned();
			} else {
				ret << "/" << i->AsString();
			}
		}
		return ret.str();
	}

	struct Patch_t {
		Patch_t(Operations_e o, Path p, Variant v) : op(o), path(p), value(v) {}
		Patch_t(Operations_e o, Path p) : op(o), path(p) {}
		Operations_e op;
		Path path;
		Variant value;
	};

	void JSONPatchUndo(VariantRef v, std::deque<Patch_t> *undo) {
		while (!undo->empty()) {
			const Patch_t &p = undo->back();
			switch (p.op) {
			case OP_ADD:
				{
					Path path = p.path;
					if (path.size() > 0) {
						PathElement elem = path.back();
						path.pop_back();
						VariantRef r = v.AtPath(path);
						if (r.IsList()) {
							r.AsList().insert(r.ListBegin()+elem.AsUnsigned(), p.value);
						} else {
							r.Set(elem.AsString(), p.value);
						}
					} else {
						v = p.value;
					}
				}
				break;
			case OP_REMOVE:
				v.ErasePath(p.path);
				break;
			case OP_TEST:
			case OP_GET:
				throw std::runtime_error("Unable to perform operation here");
			}
			undo->pop_back();
		}
	}


	bool JSONPatchOp(VariantRef v, const Variant &diff, std::deque<Patch_t> *undo) {
		std::string oper = diff.Get("op").AsString();
		Operation_t op;
		try {
			if (oper == "add") {
				op = Operation_t(OP_ADD, diff.Get("path").AsString());
				op.value = diff.Get("value");
				JSONPointerOp(op.pointer, v, op);
				if (op.replaced) {
					undo->push_back(Patch_t(OP_ADD, op.path, op.old_value));
				} else {
					undo->push_back(Patch_t(OP_REMOVE, op.path));
				}
				return op.success;
			} else if (oper == "remove") {
				op = Operation_t(OP_REMOVE, diff.Get("path").AsString());
				JSONPointerOp(op.pointer, v, op);
				undo->push_back(Patch_t(OP_ADD, op.path, op.value));
				return op.success;
			} else if (oper == "replace") {
				op = Operation_t(OP_GET, diff.Get("path").AsString());
				VariantRef r = JSONPointerOp(op.pointer, v, op);
				if (r.Exists()) {
					r = diff.Get("value");
				} else {
					return false;
				}
				undo->push_back(Patch_t(OP_ADD, op.path, op.value));
				return op.success;
			} else if (oper == "move") {
				op = Operation_t(OP_REMOVE, diff.Get("from").AsString());
				JSONPointerOp(op.pointer, v, op);
				Variant data = op.value;
				undo->push_back(Patch_t(OP_ADD, op.path, data));
				op = Operation_t(OP_ADD, diff.Get("path").AsString());
				op.value = data;
				JSONPointerOp(op.pointer, v, op);
				if (op.replaced) {
					undo->push_back(Patch_t(OP_ADD, op.path, op.old_value));
				} else {
					undo->push_back(Patch_t(OP_REMOVE, op.path));
				}
				return op.success;
			} else if (oper == "copy") {
				op = Operation_t(OP_GET, diff.Get("from").AsString());
				JSONPointerOp(op.pointer, v, op);
				Variant data = op.value;
				op = Operation_t(OP_ADD, diff.Get("path").AsString());
				op.value = data;
				JSONPointerOp(op.pointer, v, op);
				if (op.replaced) {
					undo->push_back(Patch_t(OP_ADD, op.path, op.old_value));
				} else {
					undo->push_back(Patch_t(OP_REMOVE, op.path));
				}
				return op.success;
			} else if (oper == "test") {
				op = Operation_t(OP_TEST, diff.Get("path").AsString());
				op.value = diff.Get("value");
				JSONPointerOp(op.pointer, v, op);
				return op.success;
			}
		} catch (const std::exception &e) {
			std::ostringstream oss;
			oss << "Error executing patch op \"" << oper << "\" for pointer: " << op.pointer
				<< " stopped at: " << PathString(op.path) << "\n\twhat:\n" << e.what();
			throw std::runtime_error(oss.str());
		}
		throw std::runtime_error("Unknown JSON Patch operation: " + oper);
	}

	std::pair<bool, std::string> JSONPatch(VariantRef v, const Variant &diff) {
		std::deque<Patch_t> undo;
		try {
			bool success = true;
			if (diff.IsList()) {
				Variant::ConstListIterator i(diff.ListBegin()), e(diff.ListEnd());
				for (;i != e && success; ++i) {
					success = JSONPatchOp(v, *i, &undo);
				}
			} else {
				success = JSONPatchOp(v, diff, &undo);
			}
			std::string msg;
			if (!success) {
				JSONPatchUndo(v, &undo);
				msg = "Failure";
			}
			return std::make_pair(success, msg);
		} catch (const std::exception &e) {
			JSONPatchUndo(v, &undo);
			return std::make_pair(false, std::string(e.what()));
		}
	}

	void JSONDiffRef(VariantRef src, VariantRef dst, VariantRef diff, Path &path) {
		if (!dst.Exists()) {
			diff.Append(
					Variant()
					.Set("op", "remove")
					.Set("path", FormJSONPointer(path))
					);
			return;
		}
		if (!src.Exists()) {
			diff.Append(
					Variant()
					.Set("op", "add")
					.Set("path", FormJSONPointer(path))
					.Set("value", dst)
					);
			return;
		}
		if (src.GetType() != dst.GetType()) {
			diff.Append(
					Variant()
					.Set("op", "replace")
					.Set("path", FormJSONPointer(path))
					.Set("value", dst)
					);
			return;
		}
		if (src != dst) {
			switch (src.GetType()) {
			case Variant::ListType:
				{
					uintmax_t len = std::max<uintmax_t>(src.Size(), dst.Size());
					for (uintmax_t i = 0; i < len; ++i) {
						path.push_back(PathElement(i));
							JSONDiffRef(src.At(i), dst.At(i), diff, path);
						path.pop_back();
					}
				}
				break;
			case Variant::MapType:
				{
					std::set<std::string> keys;
					Variant::ConstMapIterator i, e;
					for (i = dst.MapBegin(), e = dst.MapEnd(); i != e; ++i) {
						keys.insert(i->first);
					}
					for (i = src.MapBegin(), e = src.MapEnd(); i != e; ++i) {
						keys.insert(i->first);
					}
					for (std::set<std::string>::iterator i(keys.begin()), e(keys.end()); i != e; ++i) {
						path.push_back(PathElement(*i));
						JSONDiffRef(src.At(*i), dst.At(*i), diff, path);
						path.pop_back();
					}
				}
				break;
			default:
				diff.Append(
						Variant()
						.Set("op", "replace")
						.Set("path", FormJSONPointer(path))
						.Set("value", dst)
						);
				break;
			}
		}
	}

	Variant JSONDiff(const Variant &src, const Variant &dst) {
		Path path;
		Variant diff = Variant::ListType;
		JSONDiffRef(src, dst, diff, path);
		return diff;
	}
}
