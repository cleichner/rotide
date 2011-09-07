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

#include <rotide/scripting.hpp>
#include <rotide/curses.hpp>
#include <rotide/js/core.hpp>
#include <rotide/v8/type_conversion.hpp>

#include <sstream>
#include <fstream>
#include <iostream>
#include <string>

#include <cassert>
#include <cstring>

#include <linux/input.h>

namespace {
Accessors accessors[] = {
    ACCESSOR_MAP(Scripting_engine, insert_mode),
    ACCESSOR_MAP(Scripting_engine, status),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_A),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_B),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_C),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_D),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_E),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_F),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_G),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_H),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_I),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_J),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_K),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_L),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_M),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_N),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_O),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_P),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_Q),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_R),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_S),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_T),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_U),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_V),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_W),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_X),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_Y),
    ACCESSOR_GETTER_MAP(Scripting_engine, CTRL_Z),
    ACCESSOR_GETTER_MAP(Scripting_engine, I),
    ACCESSOR_GETTER_MAP(Scripting_engine, ESC),
    { NULL, NULL, NULL }
};

Function_mapping functions[] = {
    FUNCTION_MAP(Scripting_engine, test),
    FUNCTION_MAP(Scripting_engine, bind),
    { NULL, NULL, NULL }
};

const int STATUS = 55;

}

Scripting_engine::Scripting_engine(Curses* curses)
    : curses(curses)
{
    good = false;

    // Read in the source
    Curses_pos pos = curses->at(0, 0);
    active_pos = &pos;
    const char* RC_FILE = "runtime/rotide.js";

    pos << "Rotide Scripting Engine" << NEXT_LINE;
    pos << HLINE << NEXT_LINE;
    pos  << "Initializing scripting engine" ;

    std::ifstream rc(RC_FILE, std::ifstream::in);

    pos.col = STATUS;
    if (!rc.good()) {
        pos  << "[FAIL]" << NEXT_LINE
            << RC_FILE << " could not be found!" 
            << NEXT_LINE;
        return;
    }

    pos << "[GOOD]" << NEXT_LINE;

    std::stringstream rc_stream;
    rc_stream << rc.rdbuf();
    const std::string& buf = rc_stream.str();

    pos << "Creating environment";
    pos.col = STATUS;

    // Create the execution scope
    v8::HandleScope exec_scope;
    v8::Handle<v8::String> script = 
        v8::String::New(buf.c_str(), buf.size());

    // Create the global template and reserve internal fields
    global = v8::ObjectTemplate::New();
    global->SetInternalFieldCount(1);

    // Create the execution context, enter it.
    context = v8::Context::New(NULL, global);
    v8::Context::Scope scope(context);

    // Create the engine object
    tmpl = v8::Persistent<v8::FunctionTemplate>();
    object = wrap_class_as_object(&tmpl, this, NULL);

    // Set the internal field to a reference of this
    // Allow the object to reference itself.
    v8::Handle<v8::Object> proxy = context->Global();
    v8::Handle<v8::Object> proto = proxy->GetPrototype().As<v8::Object>();
    proto->SetPointerInInternalField(0, this);
    proto->Set(v8::String::New("ro"), object);

    v8::Handle<v8::Object> core = Core::wrap_class_as_object(this);
    object->Set(v8::String::New("core"), core);

    pos << "[GOOD]" << NEXT_LINE << "Compiling";
    pos.col = STATUS;

    // Compile the code
    v8::TryCatch tc;
    v8::Handle<v8::Script> compiled = v8::Script::Compile(script);
    if (tc.HasCaught()) {
        v8::Handle<v8::Message> message = tc.Message();
        pos  << "[FAIL]" << NEXT_LINE
            << "<" << RC_FILE << ":" << message->GetLineNumber() << "> "
            << *v8::String::Utf8Value(message->Get());
        return;
    }

    pos << "[GOOD]" << NEXT_LINE << "Running";

    // Run the script
    v8::Handle<v8::Value> result = compiled->Run();
    if (tc.HasCaught()) {
        v8::Handle<v8::Message> message = tc.Message();
        pos << "[FAIL]" << NEXT_LINE
            << "<" << RC_FILE << ":" << message->GetLineNumber() << "> "
            << *v8::String::Utf8Value(message->Get());
        return;
    }

    good = true;
}

int is_key_pressed(int fd, int key)
{
        char key_b[KEY_MAX/8 + 1];

        memset(key_b, 0, sizeof(key_b));
        ioctl(fd, EVIOCGKEY(sizeof(key_b)), key_b);

        return !!(key_b[key/8] & (1<<(key % 8)));
}

void Scripting_engine::handle_key_combination()
{
    Curses_pos status = curses->status();
    if (curses->last_key >= CTRL_A 
            && curses->last_key <= CTRL_Z
            && curses->last_key != CTRL_J) {
        key_combination.push_back(curses->last_key);
        status << CLEAR;
        for (Key_list::const_iterator cit = key_combination.begin(),
                        end = key_combination.end();
                        cit != end;
                        ++cit)
        {
            status << KEY_STR(*cit);
            if ((cit + 1) != end) 
                status << "-";
        }
    } else {
        const Function_list* list;

        if (!insert_mode()) {
            status << CLEAR << COLOR(WHITE, RED) << BOLD;
        }

        if (curses->last_key == CTRL_J) {
            if (!bindings.get(key_combination, &list)) {
                if (insert_mode()) {
                    return;
                }

                status << "ERROR: ";
                for (Key_list::const_iterator cit = key_combination.begin(),
                        end = key_combination.end();
                        cit != end;
                        ++cit)
                {
                    status << KEY_STR(*cit);
                    if ((cit + 1) != end) 
                        status << "-";
                }
                status << " is not an editor command." << RESET;
                key_combination.clear();
                return;
            }
        } else if (!bindings.get(curses->last_key, &list)) {
            if (insert_mode()) {
                return;
            }

            status 
                << "ERROR: " 
                << KEY_STR(curses->last_key) << " is not an editor command." 
                << RESET;
            key_combination.clear();
            return;
        }

        status << RESET << CLEAR;

        v8::HandleScope handle;
        v8::Context::Scope scope(context);
        v8::TryCatch tc;
        for (Function_list::const_iterator cit = list->begin(),
                end = list->end();
                cit != end;
                ++cit)
        {
            (*cit)->Call(object, 0, NULL);
            if (tc.HasCaught()) {
                Curses_pos pos = curses->at(0, 0);
                v8::Handle<v8::Message> message = tc.Message();
                pos  << "[FAIL]" << message->GetLineNumber() << "> "
                    << *v8::String::Utf8Value(message->Get());
                return;
            }
        }

        key_combination.clear();
    }

}

void Scripting_engine::think()
{
    handle_key_combination();
}

bool Scripting_engine::load(const std::string& file)
{
    std::stringstream str_file;
    str_file << "runtime/" << file;
    std::ifstream rc(str_file.str().c_str(), std::ifstream::in);
    Curses_pos& pos = *active_pos;

    pos << HLINE << NEXT_LINE;
    pos  << "Loading " << str_file.str();

    pos.col = STATUS;
    if (!rc.good()) {
        pos << "[FAIL]" << NEXT_LINE
            << str_file.str() << " could not be found!" 
            << NEXT_LINE;
        return false;
    }

    pos.col = STATUS;
    pos << "[GOOD]" << NEXT_LINE;

    std::stringstream rc_stream;
    rc_stream << rc.rdbuf();
    const std::string& buf = rc_stream.str();

    pos << "Creating environment";
    pos.col = STATUS;

    // Create the execution scope
    v8::HandleScope exec_scope;
    v8::Handle<v8::String> script = 
        v8::String::New(buf.c_str(), buf.size());

    // Create the global template and reserve internal fields
    v8::Context::Scope scope(context);

    // Create the engine object
    pos << "[GOOD]" << NEXT_LINE << "Compiling";
    pos.col = STATUS;

    // Compile the code
    v8::TryCatch tc;
    v8::Handle<v8::Script> compiled = v8::Script::Compile(script);
    if (tc.HasCaught()) {
        v8::Handle<v8::Message> message = tc.Message();
        pos  << "[FAIL]" << NEXT_LINE
            << "<" << str_file.str() << ":" << message->GetLineNumber() << "> "
            << *v8::String::Utf8Value(message->Get());
        return false;
    }

    pos << "[GOOD]" << NEXT_LINE << "Running";
    pos.col = STATUS;

    // Run the script
    v8::Handle<v8::Value> result = compiled->Run();
    if (tc.HasCaught()) {
        v8::Handle<v8::Message> message = tc.Message();
        pos << "[FAIL]" << NEXT_LINE
            << "<" << str_file << ":" << message->GetLineNumber() << "> "
            << *v8::String::Utf8Value(message->Get());
        return false;
    }


    return true;
}

FUNCTION_DEFINE(Scripting_engine, test)
{
    Scripting_engine* self = unwrap<Scripting_engine>(args.Holder());
    v8::Handle<v8::Value> arg_x = args[0];
    v8::Handle<v8::Value> arg_y = args[1];
    v8::Handle<v8::Value> arg_str = args[2];

    int x = arg_x->Int32Value();
    int y = arg_y->Int32Value();
    std::string str = *v8::String::Utf8Value(arg_str->ToString());

    self->curses->at(x,y) << "Says: " << str << NEXT_LINE;
    return v8::Undefined();
}

FUNCTION_DEFINE(Scripting_engine, bind)
{
    Scripting_engine* self = unwrap<Scripting_engine>(args.Holder());
    Key_list keys;
    v8::Handle<v8::Value> key_repr(args[0]);
    v8::Handle<v8::Value> function_repr(args[1]);
    if (smart_convert(key_repr, &keys) && function_repr->IsFunction()) {
        v8::Handle<v8::Function> function 
            = v8::Handle<v8::Function>::Cast(function_repr);
        self->bindings.insert(keys, function);
        return v8::Undefined();
    } else {
        return v8::Exception::TypeError(
                v8::String::New(
                    "The definition of this method is: \
                    ro.bind([Keys], Function). You provided the wrong types \
                    for the arguments to this method."));
    }
}

ACCESSOR_GETTER_DEFINE(Scripting_engine, insert_mode)
{
    Scripting_engine* self = unwrap<Scripting_engine>(info.Holder());
    v8::Handle<v8::Value> insert_mode_repr;
    if (smart_convert(self->attrs.insert_mode, &insert_mode_repr)) {
        return insert_mode_repr;
    } else {
        return v8::Exception::Error(
                v8::String::New(
                    "Fatal error in converting type of insert_mode"));
    }
}

ACCESSOR_SETTER_DEFINE(Scripting_engine, insert_mode)
{
    Scripting_engine* self = unwrap<Scripting_engine>(info.Holder());
    if (!smart_convert(value, &self->attrs.insert_mode)) {
        v8::Exception::Error(
                v8::String::New(
                    "insert_mode is either true or false."));
    }
}

ACCESSOR_GETTER_DEFINE(Scripting_engine, status)
{
    Scripting_engine* self = unwrap<Scripting_engine>(info.Holder());
    v8::Handle<v8::Value> status_repr;
    if (smart_convert(self->attrs.status, &status_repr)) {
        return status_repr;
    } else {
        return v8::Exception::Error(
                v8::String::New(
                    "Fatal error in converting type of status"));
    }
}

ACCESSOR_SETTER_DEFINE(Scripting_engine, status)
{
    Scripting_engine* self = unwrap<Scripting_engine>(info.Holder());
    if (!smart_convert(value, &self->attrs.status)) {
        v8::Exception::Error(
                v8::String::New(
                    "status is a string"));
    }
    self->curses->status() << self->attrs.status;
}


ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_A) 
{ return v8::Int32::New(CTRL_A); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_B)
{ return v8::Int32::New(CTRL_B); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_C)
{ return v8::Int32::New(CTRL_C); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_D)
{ return v8::Int32::New(CTRL_D); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_E)
{ return v8::Int32::New(CTRL_E); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_F)
{ return v8::Int32::New(CTRL_F); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_G)
{ return v8::Int32::New(CTRL_G); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_H)
{ return v8::Int32::New(CTRL_H); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_I)
{ return v8::Int32::New(CTRL_I); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_J)
{ return v8::Int32::New(CTRL_J); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_K)
{ return v8::Int32::New(CTRL_K); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_L)
{ return v8::Int32::New(CTRL_L); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_M)
{ return v8::Int32::New(CTRL_M); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_N)
{ return v8::Int32::New(CTRL_N); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_O)
{ return v8::Int32::New(CTRL_O); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_P)
{ return v8::Int32::New(CTRL_P); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_Q)
{ return v8::Int32::New(CTRL_Q); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_R)
{ return v8::Int32::New(CTRL_R); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_S)
{ return v8::Int32::New(CTRL_S); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_T)
{ return v8::Int32::New(CTRL_T); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_U)
{ return v8::Int32::New(CTRL_U); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_V)
{ return v8::Int32::New(CTRL_V); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_W)
{ return v8::Int32::New(CTRL_W); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_X)
{ return v8::Int32::New(CTRL_X); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_Y)
{ return v8::Int32::New(CTRL_Y); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_Z)
{ return v8::Int32::New(CTRL_Z); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, I)
{ return v8::Int32::New(105); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, ESC)
{ return v8::Int32::New(27); }

v8::Handle<v8::Object> Scripting_engine::wrap_class_as_object(
        v8::Handle<v8::FunctionTemplate>* function_tmpl,
        Scripting_engine* instance,
        Object_template_extension extensions)
{
    v8::HandleScope scope;
    if (function_tmpl->IsEmpty())
        (*function_tmpl) = v8::FunctionTemplate::New();
    generate_fun_tmpl(function_tmpl, accessors, functions, NULL);
    (*function_tmpl)->SetClassName(v8::String::New("rotide"));
    v8::Handle<v8::Function> ctor = (*function_tmpl)->GetFunction();
    v8::Local<v8::Object> obj = ctor->NewInstance();
    obj->SetInternalField(0, v8::External::New(instance));
    return scope.Close(obj);
}
