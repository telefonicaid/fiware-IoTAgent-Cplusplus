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
#include <Variant/Variant.h>
#include <Variant/Parser.h>
#include <stdexcept>
#include <string>
#include <string.h>
#include <fstream>

#if 0
#include <iostream>
#include <signal.h>
#define ANCHOR (anchor ? anchor : "")
#define TAG (tag ? tag : "")
#define DBTRACE(args) do {\
	for (int i = Level(); i > 0; --i) { std::cerr << ". "; }\
	std::cerr << args << std::endl;\
} while (false)
#else
#define DBTRACE(args)
#endif

namespace libvariant {

	typedef std::map<std::string, Variant> AnchorMap;

	class ParserState {
	public:

		ParserState()
		{}

		void Anchor(const char *anchor, Variant v) {
			if (anchor) { anchors.insert(std::make_pair(std::string(anchor), v)); }
		}

		Variant Anchor(const char *anchor) {
			return anchors.at(anchor);
		}

		AnchorMap anchors;
		Variant result;
	};

	class VariantBaseParserActions : public ParserActions {
	public:
		VariantBaseParserActions(ParserState *s, VariantBaseParserActions *b)
			: done(false), seen_begin_document(false), state(s), base(b)
		{}

		virtual void BeginDocument(ParserImpl *p) {
			DBTRACE("BeginDocument");
			seen_begin_document = true;
		}
		virtual void EndDocument(ParserImpl *p) {
			DBTRACE("EndDocument");
			Finish(p);
		}

		virtual void BeginMap(ParserImpl *p, int length, const char *anchor, const char *tag);
		virtual void BeginList(ParserImpl *p, int length, const char *anchor, const char *tag);

		virtual void Alias(ParserImpl *p, const char *anchor) {
			DBTRACE("Alias (" << ANCHOR << ")");
			SetValue(p, state->Anchor(anchor), 0);
		}
		virtual void Scalar(ParserImpl *p, double v, const char *anchor, const char *tag) {
			DBTRACE("Scalar(" << v << ", " << ANCHOR << ", " << TAG << ")");
			SetValue(p, v, anchor);
		}
		virtual void Scalar(ParserImpl *p, const char *str, unsigned length, const char *anchor, const char *tag) {
			DBTRACE("Scalar(" << std::string(str, length) << ", " << ANCHOR << ", " << TAG << ")");
			SetValue(p, std::string(str, length), anchor);
		}
		virtual void Scalar(ParserImpl *p, bool v, const char *anchor, const char *tag) {
			DBTRACE("Scalar(" << v << ", " << ANCHOR << ", " << TAG << ")");
			SetValue(p, v, anchor);
		}
		virtual void Null(ParserImpl *p, const char *anchor, const char *tag) {
			DBTRACE("Scalar( null, " << ANCHOR << ", " << TAG << ")");
			SetValue(p, Variant::NullType, anchor);
		}
		virtual void Scalar(ParserImpl *p, intmax_t v, const char *anchor, const char *tag) {
			DBTRACE("Scalar(" << v << ", " << ANCHOR << ", " << TAG << ")");
			SetValue(p, v, anchor);
		}
		virtual void Scalar(ParserImpl *p, uintmax_t v, const char *anchor, const char *tag) {
			DBTRACE("Scalar(" << v << ", " << ANCHOR << ", " << TAG << ")");
			SetValue(p, v, anchor);
		}
		virtual void Scalar(ParserImpl *p, BlobPtr b, const char *anchor, const char *tag) {
			DBTRACE("Scalar(blob, " << ANCHOR << ", " << TAG << ")");
			SetValue(p, b, anchor);
		}

		virtual void SetValue(ParserImpl *p, Variant v, const char *anchor) {
			state->Anchor(anchor, v);
			result = v;
			if (anchor) { this->anchor = anchor; }
			if (!seen_begin_document) {
				Finish(p);
			}
		}

		void Finish(ParserImpl *p) {
			if (base) {
				base->SetValue(p, result, (anchor.empty() ? 0 : anchor.c_str()));
			} else {
				state->result = result;
			}
			done = true;
			p->PopAction();
		}

		int Level() {
			if (base) { return base->Level() + 1; }
			else { return 1; }
		}

		Variant result;
		bool done;
		bool seen_begin_document;
		ParserState *state;
		VariantBaseParserActions *base;
		std::string anchor;
	};

	class VariantMapParserActions : public VariantBaseParserActions {
	public:
		VariantMapParserActions(ParserState *s, VariantBaseParserActions *b)
			: VariantBaseParserActions(s, b), expect_key(true)
		{
			result = Variant::MapType;
		}

		virtual void EndMap(ParserImpl *p) {
			DBTRACE("EndMap");
		   	Finish(p);
	   	}

		virtual void SetValue(ParserImpl *p, Variant v, const char *anchor) {
			if (expect_key) {
				key = v.AsString();
				expect_key = false;
			} else {
				state->Anchor(anchor, v);
				result[key] = v;
				expect_key = true;
			}
		}

		bool expect_key;
		std::string key;
	};

	void VariantBaseParserActions::BeginMap(ParserImpl *p, int length, const char *anchor, const char *tag) {
		DBTRACE("BeginMap(" << ANCHOR << ", " << TAG << ")");
		shared_ptr<VariantMapParserActions> actions(new VariantMapParserActions(state, this));
		if (anchor) { actions->anchor = anchor; }
		p->PushAction(actions);
	}

	class VariantListParserActions : public VariantBaseParserActions {
	public:
		VariantListParserActions(ParserState *s, VariantBaseParserActions *b)
			: VariantBaseParserActions(s, b)
		{
			result = Variant::ListType;
		}

		virtual void EndList(ParserImpl *p) {
		   	DBTRACE("EndList");
		   	Finish(p);
	   	}

		virtual void SetValue(ParserImpl *p, Variant v, const char *anchor) {
			state->Anchor(anchor, v);
			result.Append(v);
		}

	};

	void VariantBaseParserActions::BeginList(ParserImpl *p, int length, const char *anchor, const char *tag) {
		DBTRACE("BeginList(" << ANCHOR << ", " << TAG << ")");
		shared_ptr<VariantListParserActions> actions(new VariantListParserActions(state, this));
		if (anchor) { actions->anchor = anchor; }
		p->PushAction(actions);
	}

	Variant ParseVariant(Parser &p) {
		ParserState state;
		shared_ptr<VariantBaseParserActions> actions(new VariantBaseParserActions(&state, 0));
		p.PushAction(actions);
		while (p.Run() == 0 && !actions->done);
		return state.result;
	}

	Variant Deserialize(const std::string &str, SerializeType type) {
		return Deserialize(str.c_str(), str.length(), type);
	}

	Variant Deserialize(const char *str, SerializeType type) {
		return Deserialize(str, strlen(str), type);
	}

	Variant Deserialize(const void *ptr, unsigned len, SerializeType type) {
		Parser parser = CreateParser(CreateParserInput(ptr, len), type);
		return ParseVariant(parser);
	}

	Variant DeserializeFile(const char *filename, SerializeType type) {
		Parser parser = CreateParser(CreateParserInputFile(filename), type);
		return ParseVariant(parser);
	}

	Variant DeserializeFile(FILE *f, SerializeType type) {
		Parser parser = CreateParser(CreateParserInputFile(f), type);
		return ParseVariant(parser);
	}

	Variant DeserializeFile(std::streambuf *sb, SerializeType type) {
		Parser parser = CreateParser(CreateParserInputFile(sb), type);
		return ParseVariant(parser);
	}

	Variant DeserializeGuess(const std::string &str) {
		return DeserializeGuess(str.c_str(), str.length());
	}
	Variant DeserializeGuess(const char *str) {
		return DeserializeGuess(str, strlen(str));
	}
	Variant DeserializeGuess(const void *ptr, unsigned len) {
		Parser parser = CreateParserGuess(CreateParserInput(ptr, len) );
		return ParseVariant(parser);
	}
	Variant DeserializeGuessFile(const char *filename) {
		Parser parser = CreateParserGuess(CreateParserInputFile(filename));
		return ParseVariant(parser);
	}
	Variant DeserializeGuessFile(FILE *f) {
		Parser parser = CreateParserGuess(CreateParserInputFile(f));
		return ParseVariant(parser);
	}
	Variant DeserializeGuessFile(std::streambuf *sb) {
		Parser parser = CreateParserGuess(CreateParserInputFile(sb));
		return ParseVariant(parser);
	}

	LoadAllIterator DeserializeAll(const std::string &str, SerializeType type) {
		return DeserializeAll(str.c_str(), str.length(), type);
	}

	LoadAllIterator DeserializeAll(const char *str, SerializeType type) {
		return DeserializeAll(str, strlen(str), type);
	}

	LoadAllIterator DeserializeAll(const void *ptr, unsigned len, SerializeType type) {
		Parser parser = CreateParser(CreateParserInput(ptr, len), type);
		return LoadAllIterator(parser);
	}

	LoadAllIterator DeserializeAllFile(const char *filename, SerializeType type) {
		Parser parser = CreateParser(CreateParserInputFile(filename), type);
		return LoadAllIterator(parser);
	}

	LoadAllIterator DeserializeAllFile(FILE *f, SerializeType type) {
		Parser parser = CreateParser(CreateParserInputFile(f), type);
		return LoadAllIterator(parser);
	}

	LoadAllIterator DeserializeAllFile(std::streambuf *sb, SerializeType type) {
		Parser parser = CreateParser(CreateParserInputFile(sb), type);
		return LoadAllIterator(parser);
	}


	class LoadAllIterator::Impl {
	public:
		Impl(Parser p) : parser(p) {}
		Parser parser;
	};

	struct LoadAllIterator::Value {
		Value(Variant v_) :v(v_) {}
		Variant v;
		shared_ptr<Value> next;
	};

	LoadAllIterator::LoadAllIterator()
   	{}

	LoadAllIterator::LoadAllIterator(Parser &parser)
		: impl(new Impl(parser))
   	{
		Next();
	}

	Variant &LoadAllIterator::Get() {
		if (!value) { throw std::out_of_range("LoadAllIterator: Trying to reach past the end."); }
		return value->v;
	}

	void LoadAllIterator::Next() {
		if (!value) {
			if (!impl->parser.Done()) {
				value = shared_ptr<Value>(new Value(ParseVariant(impl->parser)));
			}
		} else if (value->next) {
			value = value->next;
		} else {
			if (!impl->parser.Done()) {
				value->next = shared_ptr<Value>(new Value(ParseVariant(impl->parser)));
				if (impl->parser.Done() && value->next->v.IsNull()) {
					value->next.reset();
				}
			}
			value = value->next;
		}
	}
	bool LoadAllIterator::operator==(const LoadAllIterator &that) const {
		if (value && that.value) {
			return value == that.value;
		}
		return (!value && !that.value);
	}
}
