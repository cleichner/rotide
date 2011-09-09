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
#include <sstream>
#include <string>
#include <vector>
#include <map>

bool is_ctrl_key(const int key);

class Curses;
class Curses_pos;
class Key_node;

typedef std::map<int, Key_node> Key_mapping;
typedef std::map<std::string, Function_list> Function_map;
typedef std::map<std::string, Function_list> Command_mapping; 
typedef std::vector<int> Key_list;
typedef std::vector<Key_list> Key_history;
typedef std::vector<std::string> Argument_list;

class Scripting_attributes {
public:
    Scripting_attributes()
        : insert_mode(false), cmd_mode(false), status("-- WAITING --") { }
    bool insert_mode, cmd_mode;
    std::string status;
    int status_x;
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
        cmds[cmd].push_back(persistent_fun);
        node->functions.push_back(persistent_fun);
    }

    void insert(const std::string& cmd, const v8::Handle<v8::Function>& fun)
    {
        v8::Persistent<v8::Function> persistent_fun = v8::Persistent<v8::Function>::New(fun);
        cmds[cmd].push_back(persistent_fun);
    }

    bool get(const std::string& cmd_str, std::string* cmd_val, const Function_list** fun, v8::Handle<v8::Array>* arguments)
    {
        size_t offset = cmd_str.find_first_of(" ");
        offset = (offset == std::string::npos) ? cmd_str.size() : offset;
        const std::string& cmd = cmd_str.substr(0, offset);

        if (cmd_val)
            *cmd_val = cmd;

        Command_mapping::iterator cmit = cmds.find(cmd);

        if (cmit != cmds.end()) {
            *fun = &cmit->second;

            if ((offset + 1) < cmd_str.size()) {
                const std::string& arg_str = cmd_str.substr(offset + 1);
                std::string::const_iterator cit = arg_str.begin();
                std::string::const_iterator end = arg_str.end();
                set_arguments<std::string>(cit, end, arguments);
            }

            return true;
        }

        cmit = cmds.find("*");
        if (cmit != cmds.end()) {
            *fun = &cmit->second;
            if ((offset + 1) < cmd_str.size()) {
                const std::string& arg_str = cmd_str.substr(offset + 1);
                std::string::const_iterator cit = arg_str.begin();
                std::string::const_iterator end = arg_str.end();
                set_arguments<std::string>(cit, end, arguments);
            }
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

    template<class T>
    void set_arguments(
            typename T::const_iterator& beg, 
            typename T::const_iterator& end, 
            v8::Handle<v8::Array>* arguments)
    {
        bool in_quote = false;
        char c = *beg, p = '\0';
        std::stringstream buf;
        Argument_list arg_list;
        for (typename T::const_iterator ait = beg;
                ait != end;
                ++ait)
        {
            c = *ait;
            bool end_next = (ait + 1) == end;
            bool is_quote = (c == '"' && p != '\\');

            // Switch our quote states
            if (is_quote) 
                in_quote = !in_quote;

            // If the last character is next then make sure to
            // include it (unless it is a quote)
            if (end_next && !is_quote)
                buf << c;

            // If we're not in a quote and we're in a separator
            // or we're near the end then we can append the
            // string to the stream.
            if ((!in_quote && c == ' ') || end_next) {
                p = c;
                arg_list.push_back(buf.str());
                buf.str("");
                continue;
            }

            p = c;

            if (is_quote || p == '\\')
                continue;

            buf << c;
        }

        *arguments = v8::Array::New(arg_list.size());

        int i = 0;
        for (Argument_list::const_iterator argit = arg_list.begin(),
                argend = arg_list.end();
                argit != argend;
                ++argit, i++)
        {
            // TODO(justinvh): type-based arguments?
            const char* s = (*argit).c_str();
            (*arguments)->Set(i, v8::String::New(s, (*argit).size()));
        }

    }

    bool get(const Key_list& key_list, 
            const Function_list** funs, 
            v8::Handle<v8::Array>* arguments)
    {
        Key_mapping* mapping = &keys;
        Key_mapping::iterator kit = mapping->end();

        bool is_cmd = is_ctrl_key(*key_list.begin());
        for (Key_list::const_iterator cit = key_list.begin(),
                end = key_list.end();
                cit != end;
                ++cit)
        {
            if (kit != mapping->end() 
                    && is_cmd 
                    && (*cit) == int(' ') 
                    && (cit + 1) != end) 
            {
                if (kit->second.functions.size()) {
                    ++cit;
                    set_arguments<Key_list>(cit, end, arguments);
                    *funs = &kit->second.functions;
                    return true;
                }

                return false;
            }

            kit = mapping->find(*cit);
            if (kit != mapping->end()) {
                if ((cit + 1) == key_list.end()) {
                    if (kit->second.functions.size()) {
                        *funs = &kit->second.functions;
                        return true;
                    } else {
                        return false;
                    }
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
    v8::Persistent<v8::Object> object;          // Engine namespace
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
        FUNCTION(command);
        ACCESSOR(insert_mode);
        ACCESSOR(cmd_mode);
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
        ACCESSOR_GETTER(COLON);
    };

    static v8::Handle<v8::Object> wrap_class_as_object(
            v8::Handle<v8::FunctionTemplate>* tmpl,
            Scripting_engine* instance,
            Object_template_extension extensions);
};

#endif // ROTIDE_SCRIPTING_HPP
