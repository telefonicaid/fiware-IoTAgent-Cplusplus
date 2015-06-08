/** \file
 * \author John Bridgman
 * \brief 
 */

#include "EventBuffer.h"
#include <stdlib.h>
#include <ctype.h>
#include <ostream>
#include <limits>

namespace libvariant {

	EmitterParserActions::~EmitterParserActions() {}

	void EventBuffer::Event::Fire(ParserImpl *that, ParserActions *a) const {
		switch (type) {
		case BEGIN_DOCUMENT:
			a->BeginDocument(that);
			break;
		case END_DOCUMENT:
			a->EndDocument(that);
			break;
		case BEGIN_MAP:
			a->BeginMap(that, val.len, 0, 0);
			break;
		case END_MAP:
			a->EndMap(that);
			break;
		case BEGIN_LIST:
			a->BeginList(that, val.len, 0, 0);
			break;
		case END_LIST:
			a->EndList(that);
			break;
		case SCALAR_NULL:
			a->Null(that, 0, 0);
			break;
		case SCALAR_BOOL:
			a->Scalar(that, val.b, 0, 0);
			break;
		case SCALAR_STRING:
			a->Scalar(that, s.c_str(), s.length(), 0, 0);
			break;
		case SCALAR_INT:
			a->Scalar(that, val.i, 0, 0);
			break;
		case SCALAR_UINT:
			a->Scalar(that, val.u, 0, 0);
			break;
		case SCALAR_FLOAT:
			a->Scalar(that, val.d, 0, 0);
			break;
		case SCALAR_BLOB:
			a->Scalar(that, blob->Copy(), 0, 0);
			break;
		default:
			throw std::runtime_error("EventBuffer::Event::Fire Encountered an incorrect event.");
		}
	}

	bool EventBuffer::Event::operator==(const Event &o) const {
		switch (type) {
		case SCALAR_BOOL:
			return  val.b == o.val.b;
		case SCALAR_FLOAT:
			return val.d == o.val.d || (val.d != val.d && o.val.d != o.val.d);
		case SCALAR_STRING:
			return s == o.s;
		case SCALAR_INT:
			return val.i == o.val.i;
		case SCALAR_UINT:
			return val.u == o.val.u;
		default:
			if (type != o.type) { return false; }
			break;
		}
		return true;
	}

	void EventBuffer::Event::Print(std::ostream &os) const {
		switch (type) {
		case BEGIN_DOCUMENT:
			os << "Event(BEGIN_DOCUMENT)";
			break;
		case END_DOCUMENT:
			os << "Event(END_DOCUMENT)";
			break;
		case BEGIN_MAP:
			os << "Event(BEGIN_MAP, " << val.len << ")";
			break;
		case END_MAP:
			os << "Event(END_MAP)";
			break;
		case BEGIN_LIST:
			os << "Event(BEGIN_LIST, " << val.len << ")";
			break;
		case END_LIST:
			os << "Event(END_LIST)";
			break;
		case SCALAR_BOOL:
			os << "Event(SCALAR_BOOL, " << std::boolalpha << val.b << ")";
			break;
		case SCALAR_FLOAT:
			os << "Event(SCALAR_FLOAT, " << val.d << ")";
			break;
		case SCALAR_STRING:
			os << "Event(SCALAR_STRING, " << s << ")";
			break;
		case SCALAR_NULL:
			os << "Event(SCALAR_NULL)";
			break;
		case SCALAR_INT:
			os << "Event(SCALAR_INT, " << val.i << ")";
			break;
		case SCALAR_UINT:
			os << "Event(SCALAR_UINT, " << val.u << ")";
			break;
		case SCALAR_BLOB:
			os << "Event(SCALAR_BLOB)";
			break;
		default:
			os << "Unknown event (" << (int)type << ")";
			break;
		}
	}

	EventBuffer::EventBuffer(const EventBuffer &eb)
		: level(eb.level), events(eb.events)
	{

	}

	EventBuffer::~EventBuffer() {}


	void EventBuffer::FillRandom(EventType_t disallowed_events, int max_level, bool allow_nan) {
		bool begin_document = false;
		if (!(BEGIN_DOCUMENT & disallowed_events)) {
			BeginDocument();
			begin_document = true;
		}
		disallowed_events = disallowed_events | BEGIN_DOCUMENT;
		EventType_t next_event = RandomEventType(disallowed_events | SCALAR_EVENTS | END_EVENTS, max_level);
		switch (next_event) {
		case BEGIN_MAP:
			MapRandomEvent(disallowed_events, max_level, allow_nan);
			break;
		case BEGIN_LIST:
			ListRandomEvent(disallowed_events, max_level, allow_nan);
			break;
		default:
			// should not be reachable
			throw std::runtime_error("EventBuffer::FillRandom reached code that should be not reachable");
		}

		if (begin_document) {
			EndDocument();
		}
	}

	EventBuffer::EventType_t EventBuffer::RandomEventType(EventBuffer::EventType_t disallowed, int max_level)
	{
		if (level >= max_level) {
			disallowed = disallowed | BEGIN_EVENTS;
		}
		while (true) {
			int bit = rand() % 32;
			int val = 1<<bit;
			if (!(val & disallowed) && (ALL_EVENTS & val)) {
				return (EventType_t)val;
			}
		}
	}

	static std::string GenerateRandomString() {
		while (true) {
			int len = rand() % 100;
			if (len < 1) { continue; }
			std::string str(len, '\n');
			for (int i = 0; i < len; ++i) {
				char c;
				do { c = rand(); } while (!isprint(c) || isblank(c) || isdigit(c) || c == '~');
				str[i] = c;
			}
			return str;
		}
	}

	static BlobPtr GenerateRandomBlob() {
		while (true) {
			int len = rand() % 100;
			if (len < 1) { continue; }
			char *ptr = (char*)malloc(len);
			for (int i = 0; i < len; ++i) {
				ptr[i] = rand();
			}
			return libvariant::Blob::CreateFree(ptr, len);
		}
	}

	void EventBuffer::BasicRandomEvent(EventType_t disallowed, int max_level, bool allow_nan) {
		EventType_t next_event = RandomEventType(disallowed | END_EVENTS, max_level);
		switch (next_event) {
		case BEGIN_MAP:
			MapRandomEvent(disallowed, max_level, allow_nan);
			break;
		case BEGIN_LIST:
			ListRandomEvent(disallowed, max_level, allow_nan);
			break;
		case SCALAR_NULL:
			EmitNull();
			break;
		case SCALAR_BOOL:
			if (rand() & 1) {
				EmitTrue();
			} else {
				EmitFalse();
			}
			break;
		case SCALAR_STRING:
			{
				std::string rstr = GenerateRandomString();
				Emit(rstr.c_str());
			}
			break;
		case SCALAR_INT:
			Emit(intmax_t(rand() - RAND_MAX/2));
			break;
		case SCALAR_UINT:
			Emit(uintmax_t(rand()));
			break;
		case SCALAR_FLOAT:
			{
				int r1 = rand();
				int type = rand();
				int r2 = rand();
				double val;
				if ((type&0x1) == 0 || !allow_nan) {
					val = double(r1)*double(r2)/double(RAND_MAX);
				} else if ((type&0x2) == 0) {
					val = std::numeric_limits<double>::infinity();
					if ((type&0x4) == 0) {
						val = -val;
					}
				} else if ((type&0x4) == 0) {
					val = std::numeric_limits<double>::quiet_NaN();
				} else {
					val = std::numeric_limits<double>::signaling_NaN();
				}
				Emit(val);
			}
			break;
		case SCALAR_BLOB:
			Emit((ConstBlobPtr)GenerateRandomBlob());
			break;
		default:
			// should not be reachable
			throw std::runtime_error("EventBuffer::BasicRandomEvent reached code that should be not reachable");
		}
	}

	void EventBuffer::MapRandomEvent(EventType_t disallowed, int max_level, bool allow_nan) {
		int map_len = rand()%10;
		BeginMap(map_len);
		for (int i = 0; i < map_len; ++i) {
			std::string key = GenerateRandomString();
			Emit(key.c_str());
			BasicRandomEvent(disallowed, max_level, allow_nan);
		}
		EndMap();
	}

	void EventBuffer::ListRandomEvent(EventType_t disallowed, int max_level, bool allow_nan) {
		int list_len = rand()%10;
		BeginList(list_len);
		for (int i = 0; i < list_len; ++i) {
			BasicRandomEvent(disallowed, max_level, allow_nan);
		}
		EndList();
	}
}
