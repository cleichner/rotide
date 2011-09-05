#ifndef ROTIDE_SCRIPTING_HPP
#define ROTIDE_SCRIPTING_HPP

#include <v8.h>
#include <rotide/v8/easy.hpp>
#include <string>

class Curses;
class Curses_pos;

struct Scripting_attributes {
    bool insert_mode;
};

class Scripting_engine {
public:
    Scripting_engine(Curses* curses);
    bool load(const std::string& file);
    
    v8::Handle<v8::ObjectTemplate> global;      // Global scope
    v8::Handle<v8::Object> object;               // Engine namespace
    v8::Persistent<v8::FunctionTemplate> tmpl;  // Function template
    v8::Persistent<v8::Context> context;        // Engine context
    Curses* curses;
    Curses_pos* active_pos;
private:
    Scripting_attributes attrs;
public:
    DEFINE(Scripting_engine)
    {
        FUNCTION(test);
        ACCESSOR(insert_mode);
    };

    static v8::Handle<v8::Object> wrap_class_as_object(
            v8::Handle<v8::FunctionTemplate>* tmpl,
            Scripting_engine* instance,
            Object_template_extension extensions);
};

#endif // ROTIDE_SCRIPTING_HPP
