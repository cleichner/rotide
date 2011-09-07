//
// Copyright 2011 Justin Bruce Van Horne <justinvh@gmail.com>
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

typedef std::map<int, Key_node> Key_mapping;
typedef std::vector<v8::Persistent<v8::Function> > Function_list;
typedef std::map<std::string, Function_list> Function_map;
typedef std::map<std::string, v8::Persistent<v8::Function> > Command_mapping; 
typedef std::vector<int> Key_list;
typedef std::vector<Key_list> Key_history;

class Scripting_attributes {
public:
    Scripting_attributes()
        : insert_mode(false), status("-- WAITING --") { }
    bool insert_mode;
    std::string status;
    Function_map on_command;
};

struct Key_node{
    Key_node() { }
    Function_list functions;
    Key_mapping children;
};

class Key_engine {
public:
    void insert(const Key_list& key_list, 
            const std::string& cmd,
            const v8::Handle<v8::Function>& fun)
    {
        Key_mapping* mapping = &keys;
        Key_mapping::iterator kit = keys.begin();
        Key_node* node;
        for (Key_list::const_iterator cit = key_list.begin(),
                end = key_list.end();
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

        v8::Persistent<v8::Function> persistent_fun = v8::Persistent<v8::Function>::New(fun);
        cmds[cmd] = persistent_fun;
        node->functions.push_back(persistent_fun);
    }

    bool get(const std::string& cmd, v8::Handle<v8::Function>* fun)
    {
        Command_mapping::iterator cmit = cmds.find(cmd);
        if (cmit != cmds.end()) {
            fun = &cmit->second;
            return true;
        }

        return false;
    }

    bool get(const int key, const Function_list** funs)
    {
        Key_mapping::iterator kit = keys.find(key);
        if (kit != keys.end()) {
            *funs = &kit->second.functions;
            return true;
        }
        return false;
    }

    bool get(const Key_list& key_list, const Function_list** funs)
    {
        Key_mapping* mapping = &keys;
        Key_mapping::iterator kit;
        for (Key_list::const_iterator cit = key_list.begin(),
                end = key_list.end();
                cit != end;
                ++cit)
        {
            kit = mapping->find(*cit);
            if (kit != mapping->end()) {
                if ((cit + 1) == key_list.end()) {
                    *funs = &kit->second.functions;
                    return true;
                } else {
                    mapping = &kit->second.children;
                }
            } else {
                return false;
            }
        }

        return false;
    }

    Command_mapping cmds;
    Key_mapping keys;
};

class Scripting_engine {
public:
    Scripting_engine(Curses* curses);
    bool load(const std::string& file);
    void think();
    
    v8::Handle<v8::ObjectTemplate> global;      // Global scope
    v8::Handle<v8::Object> object;               // Engine namespace
    v8::Persistent<v8::FunctionTemplate> tmpl;  // Function template
    v8::Persistent<v8::Context> context;        // Engine context
    Curses* curses;
    Curses_pos* active_pos;
    Key_engine bindings;
    bool good;
    bool insert_mode() { return attrs.insert_mode; }
    const std::string& status() { return attrs.status; }
private:
    void handle_key_combination();
    Scripting_attributes attrs;
    Key_list key_combination;
    Key_history key_history;
public:
    DEFINE(Scripting_engine)
    {
        FUNCTION(test);
        FUNCTION(bind);
        FUNCTION(on_command);
        ACCESSOR(insert_mode);
        ACCESSOR(status);

        // The fun stuff
        ACCESSOR_GETTER(CTRL_A);
        ACCESSOR_GETTER(CTRL_B);
        ACCESSOR_GETTER(CTRL_C);
        ACCESSOR_GETTER(CTRL_D);
        ACCESSOR_GETTER(CTRL_E);
        ACCESSOR_GETTER(CTRL_F);
        ACCESSOR_GETTER(CTRL_G);
        ACCESSOR_GETTER(CTRL_H);
        ACCESSOR_GETTER(CTRL_I);
        ACCESSOR_GETTER(CTRL_J);
        ACCESSOR_GETTER(CTRL_K);
        ACCESSOR_GETTER(CTRL_L);
        ACCESSOR_GETTER(CTRL_M);
        ACCESSOR_GETTER(CTRL_N);
        ACCESSOR_GETTER(CTRL_O);
        ACCESSOR_GETTER(CTRL_P);
        ACCESSOR_GETTER(CTRL_Q);
        ACCESSOR_GETTER(CTRL_R);
        ACCESSOR_GETTER(CTRL_S);
        ACCESSOR_GETTER(CTRL_T);
        ACCESSOR_GETTER(CTRL_U);
        ACCESSOR_GETTER(CTRL_V);
        ACCESSOR_GETTER(CTRL_W);
        ACCESSOR_GETTER(CTRL_X);
        ACCESSOR_GETTER(CTRL_Y);
        ACCESSOR_GETTER(CTRL_Z);
        ACCESSOR_GETTER(I);
        ACCESSOR_GETTER(ESC);
    };

    static v8::Handle<v8::Object> wrap_class_as_object(
            v8::Handle<v8::FunctionTemplate>* tmpl,
            Scripting_engine* instance,
            Object_template_extension extensions);
};

#endif // ROTIDE_SCRIPTING_HPP
