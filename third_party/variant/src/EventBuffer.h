/** \file
 * \author John Bridgman
 * \brief 
 */
#ifndef VARIANT_EVENTBUFFER_H
#define VARIANT_EVENTBUFFER_H
#pragma once
#include <Variant/Emitter.h>
#include <Variant/Parser.h>
#include <deque>
#include <iosfwd>
namespace libvariant {

	class EmitterParserActions : public ParserActions {
	public:
		EmitterParserActions(EmitterImpl *i) : impl(i) {}
		EmitterParserActions(Emitter i) : impl_ptr(i.GetImpl()) { impl = impl_ptr.get(); }
		virtual ~EmitterParserActions();

		virtual void BeginDocument(ParserImpl *p)
		{ impl->BeginDocument(); }
		virtual void EndDocument(ParserImpl *p)
		{ impl->EndDocument(); }
		virtual void BeginMap(ParserImpl *p, int length, const char *anchor, const char *tag)
		{ impl->BeginMap(length); }
		virtual void EndMap(ParserImpl *p)
		{ impl->EndMap(); }
		virtual void BeginList(ParserImpl *p, int length, const char *anchor, const char *tag)
		{ impl->BeginList(length); }
		virtual void EndList(ParserImpl *p)
		{ impl->EndList(); }
		//virtual void Alias(ParserImpl *p, const char *anchor)
		virtual void Scalar(ParserImpl *p, double v, const char *anchor, const char *tag)
		{ impl->Emit(v); }
		virtual void Scalar(ParserImpl *p, const char *str, unsigned length, const char *anchor, const char *tag)
		{
			std::string v(str, length);
		   	impl->Emit(v.c_str());
	   	}
		virtual void Scalar(ParserImpl *p, bool v, const char *anchor, const char *tag) {
		   	if (v) { impl->EmitTrue(); }
			else { impl->EmitFalse(); }
		}
		virtual void Null(ParserImpl *p, const char *anchor, const char *tag)
		{ impl->EmitNull(); }
		virtual void Scalar(ParserImpl *p, intmax_t v, const char *anchor, const char *tag)
		{ impl->Emit(v); }
		virtual void Scalar(ParserImpl *p, uintmax_t v, const char *anchor, const char *tag)
		{ impl->Emit(v); }
		virtual void Scalar(ParserImpl *p, BlobPtr b, const char *anchor, const char *tag)
		{ impl->Emit((ConstBlobPtr)b); }
	private:
		EmitterImpl *impl;
		shared_ptr<EmitterImpl> impl_ptr;
	};

	class EventBuffer : public EmitterImpl, public ParserImpl {
	public:
		enum EventType_t {
			NO_EVENT = 0,
			BEGIN_DOCUMENT = 1,
			END_DOCUMENT = 1<<1,
			BEGIN_MAP = 1<<2,
			END_MAP = 1<<3,
			BEGIN_LIST = 1<<4,
			END_LIST = 1<<5,
			SCALAR_NULL = 1<<6,
			SCALAR_BOOL = 1<<7,
			SCALAR_STRING = 1<<8,
			SCALAR_INT = 1<<9,
			SCALAR_UINT = 1<<10,
			SCALAR_FLOAT = 1<<11,
			SCALAR_BLOB = 1<<12,
			NON_SCALARS = BEGIN_DOCUMENT | END_DOCUMENT | BEGIN_MAP | END_MAP | BEGIN_LIST | END_LIST,
			BEGIN_EVENTS = BEGIN_DOCUMENT | BEGIN_MAP | BEGIN_LIST,
			END_EVENTS = END_DOCUMENT | END_MAP | END_LIST,
			SCALAR_EVENTS = SCALAR_NULL | SCALAR_BOOL | SCALAR_STRING | SCALAR_INT | SCALAR_UINT |
				SCALAR_FLOAT | SCALAR_BLOB,
			ALL_EVENTS = 0x0FFF
		};

		struct Event {
			Event(EventType_t t, Event *p) : type(t), parent(p), key(true) { val.len = 0; }

			Event(Event *p, const char *str) : type(SCALAR_STRING), parent(p), s(str) {}
			Event(Event *p, intmax_t v) : type(SCALAR_INT), parent(p) { val.i = v; }
			Event(Event *p, uintmax_t v) : type(SCALAR_UINT), parent(p) { val.u = v; }
			Event(Event *p, double v) : type(SCALAR_FLOAT), parent(p) { val.d = v; }
			Event(Event *p, bool v) : type(SCALAR_BOOL), parent(p) { val.b = v; }
			Event(Event *p, ConstBlobPtr b) : type(SCALAR_BLOB), parent(p), blob(b) {}

		private:
			void Update() {
				if (type == BEGIN_MAP) {
					if (key) {
						key = false;
					} else {
						key = true;
						val.len++;
					}
				} else if (type == BEGIN_LIST) {
					val.len++;
				}
			}
			
			Event *Parent() {
				if (type == BEGIN_MAP) { return this; }
				if (type == BEGIN_LIST) { return this; }
				return parent;
			}
		public:
			void Fire(ParserImpl *that, ParserActions *a) const;
			bool operator==(const Event &o) const;
			void Print(std::ostream &os) const;
		private:
			friend class EventBuffer;
			EventType_t type;
			Event *parent;
			union {
				bool b;
				double d;
				intmax_t i;
				uintmax_t u;
				uintmax_t len;
			} val;
			bool key;
			std::string s;
			libvariant::ConstBlobPtr blob;
		};

	private:
		Event *Parent() {
			if (events.empty()) { return 0; }
			else { return events.back().Parent(); }
		}

		Event *ParentUpdate() {
			Event *p = Parent();
			if (p) { p->Update(); }
			return p;
		}

		Event *GrandParent() {
			Event *p = Parent();
			if (p) { return p->parent; }
			else { return 0; }
		}
	public:

		EventBuffer() : level(0) {}
		EventBuffer(const EventBuffer &eb);
		virtual ~EventBuffer();

		virtual void BeginDocument() {
			events.push_back(Event(BEGIN_DOCUMENT, Parent()));
			level++;
		}

		virtual void EndDocument() {
			events.push_back(Event(END_DOCUMENT, GrandParent()));
			level--;
		}

		virtual void BeginMap(int) {
			events.push_back(Event(BEGIN_MAP, ParentUpdate()));
			level++;
		}

		virtual void EndMap() {
			events.push_back(Event(END_MAP, GrandParent()));
			level--;
		}

		virtual void BeginList(int) {
			events.push_back(Event(BEGIN_LIST, ParentUpdate()));
			level++;
		}

		virtual void EndList() {
			events.push_back(Event(END_LIST, GrandParent()));
			level--;
		}

		virtual void EmitNull() { events.push_back(Event(SCALAR_NULL, ParentUpdate())); }
		virtual void EmitTrue() { events.push_back(Event(ParentUpdate(), true)); }
		virtual void EmitFalse() { events.push_back(Event(ParentUpdate(), false)); }
		virtual void Emit(const char *v) { events.push_back(Event(ParentUpdate(), v)); }
		virtual void Emit(intmax_t v) { events.push_back(Event(ParentUpdate(), v)); }
		virtual void Emit(uintmax_t v) { events.push_back(Event(ParentUpdate(), v)); }
		virtual void Emit(double v) { events.push_back(Event(ParentUpdate(), v)); }
		virtual void Emit(ConstBlobPtr b) { events.push_back(Event(ParentUpdate(), b)); }

		virtual void Flush() {}
		virtual void Close() {}

		virtual Variant GetParam(const std::string &key) { return Variant::NullType; }
		virtual Variant GetParams() { return Variant::NullType; }
		virtual void SetParam(const std::string &key, Variant value) {}

		void Playback(Emitter e) {
			Playback(e.GetImpl().get());
		}

		void Playback(EmitterImpl *e) {
			shared_ptr<ParserActions> action(new EmitterParserActions(e));
			PushAction(action);
			while (!Done()) { Run(); }
			PopAction();
		}

		void Fill(Parser p) {
			shared_ptr<ParserActions> action(new EmitterParserActions(this));
			p.PushAction(action);
			unsigned start_events = events.size();
			while (!p.Done() && (level > 0 || start_events == events.size()) && p.Run() == 0);
			p.PopAction();
		}

		void FillRandom(EventType_t disallowed_events, int max_level, bool allow_nan);

		virtual int Run() {
			while (!events.empty() && !action_stack.empty()) {
				ParserActions *a = TopAction();
				events.front().Fire(this, a);
				events.pop_front();
			}
			return 0;
		}

		virtual bool Done() const { return events.empty(); }
		virtual bool Error() const { return false; }
		virtual bool Ok() const { return !events.empty(); }
		virtual std::string ErrorStr() const { return ""; }
		virtual void Reset() { events.clear(); }

		bool Equal(const EventBuffer &o) const {
			return events == o.events;
		}

		unsigned NumEvents() const { return events.size(); }
		const Event &GetEvent(int i) const { return events[i]; }
		int GetLevel() const { return level; }

	private:
		EventType_t RandomEventType(EventType_t disallowed, int max_level);
		void BasicRandomEvent(EventType_t disallowed, int max_level, bool allow_nan);
		void MapRandomEvent(EventType_t disallowed, int max_level, bool allow_nan);
		void ListRandomEvent(EventType_t disallowed, int max_level, bool allow_nan);

		int level;
		std::deque<Event> events;
	};

	inline EventBuffer::EventType_t operator|(EventBuffer::EventType_t lhs, EventBuffer::EventType_t rhs) {
		return (EventBuffer::EventType_t)((int)lhs | (int)rhs);
	}

	inline EventBuffer::EventType_t operator&(EventBuffer::EventType_t lhs, EventBuffer::EventType_t rhs) {
		return (EventBuffer::EventType_t)((int)lhs & (int)rhs);
	}

	inline std::ostream &operator<<(std::ostream &os, const EventBuffer::Event &e) {
		e.Print(os);
		return os;
	}

}
#endif
