#ifndef ROTIDE_SCRIPTING_HPP
#define ROTIDE_SCRIPTING_HPP

#include <v8.h>
#include <rotide/v8/easy.hpp>
#include <string>
#include <vector>
#include <map>

class Curses;
class Curses_pos;
class Key_node;

struct Scripting_attributes {
    bool insert_mode;
};

typedef std::map<int, Key_node> Key_mapping;
typedef std::vector<v8::Persistent<v8::Function> > Function_list;

struct Key_node{
    Key_node() { }
    Function_list functions;
    Key_mapping children;
};

class Key_engine {
public:
    void insert(const std::string& str, const v8::Handle<v8::Function>& fun)
    {
        Key_mapping* mapping = &keys;
        Key_mapping::iterator kit = keys.begin();
        Key_node* node;
        for (std::string::const_iterator cit = str.begin(),
                end = str.end();
                cit != end;
                ++cit)
        {
            Key_mapping::iterator lb = mapping->lower_bound(*cit);
            if (lb != mapping->end() && 
                    !(mapping->key_comp()(*cit, lb->first)))
            {
                mapping = &lb->second.children;
                node = &lb->second;
            } else {
                mapping->insert(lb, 
                        Key_mapping::value_type(*cit, Key_node()));
                node = &(*mapping)[*cit];
                mapping = &node->children;
            }
        }

        node->functions.push_back(v8::Persistent<v8::Function>::New(fun));
    }

    void remove(const std::string& str)
    {
    }

    bool get(const std::string& str, Function_list* funs)
    {
        return false;
    }

    Key_mapping keys;
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
    Key_engine bindings;
private:
    Scripting_attributes attrs;
public:
    DEFINE(Scripting_engine)
    {
        FUNCTION(test);
        FUNCTION(bind);
        ACCESSOR(insert_mode);
    };

    static v8::Handle<v8::Object> wrap_class_as_object(
            v8::Handle<v8::FunctionTemplate>* tmpl,
            Scripting_engine* instance,
            Object_template_extension extensions);
};

#endif // ROTIDE_SCRIPTING_HPP
