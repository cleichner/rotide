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

using namespace v8;

// Define the accessors and functions to the ro object.
//
// ro =
//      insert_mode         : boolean
//      status              : string
//      CTRL_A .. CTRL_Z    : integers
//      A .. Z              : integers
//
//      test        : function ()
//      bind        : function ([Key], String, function)
//      on_command  : function (function ([Key]))
//      on_command  : function (String, function ([Key]))
//
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
    FUNCTION_MAP(Scripting_engine, on_command),
    { NULL, NULL, NULL }
};

// Status column for startup
// TODO(justinvh): This shouldn't be a constant.
const int STATUS = 55;

} // namespace

// Construct a new scripting instance relative to
// a curses instance.
Scripting_engine::Scripting_engine(Curses* curses)
    : curses(curses)
{
    assert(curses != NULL && "Null instance of curses passed!");

    good = false;
    Curses_color COLOR_FAIL = COLOR(RED, BLACK);

    // Read in the source
    Curses_pos& pos = curses->at(0, 0);
    active_pos = &pos;

    const char* RC_FILE = "runtime/rotide.js";

    // Describe the event
    pos << "Rotide Scripting Engine" << NEXT_LINE;
    pos << HLINE << NEXT_LINE;
    pos  << "Initializing scripting engine" ;

    // Open the file
    std::ifstream rc(RC_FILE, std::ifstream::in);

    // Alert the user if the read goes wrong.
    pos.col = STATUS;
    if (!rc.good()) {
        pos << COLOR_FAIL << "[FAIL]" << RESET << NEXT_LINE
            << RC_FILE << " could not be found!" 
            << NEXT_LINE;
        return;
    }

    pos << "[GOOD]" << NEXT_LINE;

    // Read the file into a stream
    std::stringstream rc_stream;
    rc_stream << rc.rdbuf();
    const std::string& buf = rc_stream.str();

    pos << "Creating environment";
    pos.col = STATUS;

    // Create the execution scope
    HandleScope exec_scope;
    Local<String> script = 
        String::New(buf.c_str(), buf.size());

    // Create the global template and reserve internal fields
    global = ObjectTemplate::New();
    global->SetInternalFieldCount(1);

    // Create the execution context, enter it.
    context = Context::New(NULL, global);
    Context::Scope scope(context);

    // Create the engine object
    tmpl = Persistent<FunctionTemplate>();
    object = Persistent<Object>::New(wrap_class_as_object(&tmpl, this, NULL));

    // Set the internal field to a reference of this
    // Allow the object to reference itself.
    Local<Object> proxy = context->Global();
    Local<Object> proto = proxy->GetPrototype().As<Object>();
    proto->SetPointerInInternalField(0, this);
    proto->Set(String::New("ro"), object);

    // Wrap the core object
    Handle<Object> core = Core::wrap_class_as_object(this);
    object->Set(String::New("core"), core);

    pos << "[GOOD]" << NEXT_LINE << "Compiling";
    pos.col = STATUS;

    // Compile the code
    TryCatch tc;
    Local<Script> compiled = Script::Compile(script);
    if (tc.HasCaught()) {
        Local<Message> message = tc.Message();
        pos << COLOR_FAIL << "[FAIL]" << RESET << NEXT_LINE
            << "<" << RC_FILE << ":" << message->GetLineNumber() << "> "
            << *String::Utf8Value(message->Get());
        return;
    }

    pos << "[GOOD]" << NEXT_LINE << "Running";

    // Run the script
    Local<Value> result = compiled->Run();
    if (tc.HasCaught()) {
        Local<Message> message = tc.Message();
        pos << COLOR_FAIL << "[FAIL]" << RESET << NEXT_LINE
            << "<" << RC_FILE << ":" << message->GetLineNumber() << "> "
            << *String::Utf8Value(message->Get());
        return;
    }

    good = true;
}

/*
// hack for figuring if a key is pressed based off of the EVIO linux/input.h
int is_key_pressed(int fd, int key)
{
        char key_b[KEY_MAX/8 + 1];

        memset(key_b, 0, sizeof(key_b));
        ioctl(fd, EVIOCGKEY(sizeof(key_b)), key_b);

        return !!(key_b[key/8] & (1<<(key % 8)));
}
*/

bool is_ctrl_key(const int key)
{
    return (key >= CTRL_A && key <= CTRL_Z);
}

// A key combination is any series of keys pressed that are defined to
// have a callback to a JavaScript function.
//
// The basic idea is that as keys are pressed they are put into a vector
// that holds the key series. Depending on the type of press, the commands
// are handled accordingly.
//
// Any CTRL+<A..Z> combination can be appended to one another until either
// CTRL+J or ENTER is hit. This signals the end of a key combination.
//
// TODO(justinvh): Commands should be able to take input, so perhaps being
// aware of the last key and knowing the engine is in a CTRL state will allow
// stuff like: "CTRL+A-CTRL+F Hello World" to be a command.
//
// Any key press is handled independently and is greedy.
//
// EXAMPLE:
//  ro.bind([ro.A, ro.B, ro.C], "ABC", function () { ro.status = "ABCs!"; })
//  /*  This binding below is greedy and consequently will cause the above
//      binding to never be called.  */
//  ro.bind([ro.A], function () { ro.status = "Hello!"; });
//
void Scripting_engine::handle_key_combination()
{
    Key_list::const_iterator kcit, end;
    Curses_pos status = curses->status();
    int key = curses->last_key;
    HandleScope handle;
    Context::Scope scope(context);
    Handle<Array> arguments;

    // If the key pressed is any variation of CTRL+A to CTRL+Z
    // excluding CTRL+J (since ENTER holds the same values traditionally)
    // then append the key to the vector and update the status.
    if (is_ctrl_key(key) && key != CTRL_J) {
        key_combination.push_back(key);
        status << CLEAR;
        for (kcit = key_combination.begin(), end = key_combination.end();
                kcit != end;
                ++kcit)
        {
            status << KEY_STR(*kcit);
            if ((kcit + 1) != end) 
                status << "-";
        }
    // Now were' in the case that we did not press CTRL+A .. CTRL+Z and
    // instead are just typing a single command.
    } else {
        const Function_list* list;

        // If we are in insert mode then we don't want to parse
        // any single key press commands.
        // TODO(justinvh): We can't really do this yet. The distinction
        // between a command mode and an editing mode is not truly defined.
        
        // if (insert_mode()) 
        //  return;
        if (key == CTRL_J && key_combination.empty())
            return;

        if (!insert_mode()) 
            status << CLEAR << COLOR(WHITE, RED) << BOLD;

        // Remember, CTRL+J is the same as ENTER.
        if (key == CTRL_J) {
            // If the current key combination does not produce a binding
            // then we need to alert the user.
            if (!bindings.get(key_combination, &list, &arguments)) {
                if (insert_mode()) return;

                status << "ERROR: \"";
                for (kcit = key_combination.begin(),
                        end = key_combination.end();
                        kcit != end;
                        ++kcit)
                {
                    status << KEY_STR(*kcit, KS_NO_PRETTY_PRINT);
                    if (is_ctrl_key(*kcit) 
                            && (kcit + 1) != end
                            && is_ctrl_key(*(kcit + 1))) {
                        status << "-";
                    }
                }
                status << "\" is not an editor command." << RESET;
                key_history.push_back(key_combination);
                key_combination.clear();
                return;
            }
        } else if (!insert_mode()
                && key_combination.size()
                && is_ctrl_key(*key_combination.begin())) {
            if (key_combination.begin() != key_combination.end()
                    && is_ctrl_key(*(key_combination.end() - 1)) 
                    && !is_ctrl_key(key)) {
                key_combination.push_back((int)' ');
            }

            key_combination.push_back(key);
            status << RESET;
            for (kcit = key_combination.begin(),
                    end = key_combination.end();
                    kcit != end;
                    ++kcit)
            {
                status << KEY_STR(*kcit, KS_NO_PRETTY_PRINT);
                if (is_ctrl_key(*kcit) 
                            && (kcit + 1) != end
                            && is_ctrl_key(*(kcit + 1))) {
                        status << "-";
                }
            }
            return;
        }
        // Now we are just typing a single key. So, parse the command
        // as single key press.
        // TODO(justinvh):  Support multiple keys. A CTRL+<A..Z> mode is
        //                  different then a character mode. 
        else if (!bindings.get(key, &list)) {
            if (insert_mode()) return;

            status 
                << "ERROR: \"" 
                << KEY_STR(key) << "\" is not an editor command." 
                << RESET;
            key_history.push_back(key_combination);
            key_combination.clear();
            return;
        }

        status << RESET << CLEAR;

        // Now a valid function list has been generated. Create the
        // execution context and call the JavaScript function.
        TryCatch tc;
        Handle<Value> values[1] = { arguments };
        for (Function_list::const_iterator cit = list->begin(),
                end = list->end();
                cit != end;
                ++cit)
        {
            // TODO(justinvh):  A buffer object or some sort of exposed
            //                  buffer should be available as an argument.
            
            assert((*cit).IsEmpty() == false && "Lost handle to function!");
            if (arguments.IsEmpty()) {
                (*cit)->Call(object, 0, NULL);
            } else {
                (*cit)->Call(object, 1, values);
            }

            if (tc.HasCaught()) {
                Curses_pos pos = curses->at(0, 0);
                Local<Message> message = tc.Message();
                pos << "[FAIL]" << message->GetLineNumber() << "> "
                    << *String::Utf8Value(message->Get());
                return;
            }
        }

        key_history.push_back(key_combination);
        key_combination.clear();
    }

}

// When the engine thinks it needs to figure out key combinations,
// what to call, and so on. It should just wrap various calls.
void Scripting_engine::think()
{
    handle_key_combination();
}

// Load a runtime/* file.
// These files are going to be JavaScript source files relative
// to the runtime/ directory.
//
// TODO(justinvh):  There needs to be a separation between the basic
//                  runtime files and the local files of the user.
bool Scripting_engine::load(const std::string& file)
{
    // The basic logic to open the file, create a stream, and error
    // to the user if something bad goes wrong in the process.
    std::stringstream str_file;
    str_file << "runtime/" << file;

    // Open file
    std::ifstream rc(str_file.str().c_str(), std::ifstream::in);
    Curses_pos& pos = *active_pos;

    // Alert user that something is going on.
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

    // Read in the stream
    std::stringstream rc_stream;
    rc_stream << rc.rdbuf();
    const std::string& buf = rc_stream.str();

    pos << "Creating environment";
    pos.col = STATUS;

    // Create the execution scope
    HandleScope exec_scope;
    Local<String> script = 
        String::New(buf.c_str(), buf.size());

    // Create the global template and reserve internal fields
    Context::Scope scope(context);

    // Create the engine object
    pos << "[GOOD]" << NEXT_LINE << "Compiling";
    pos.col = STATUS;

    // Compile the code
    TryCatch tc;
    Local<Script> compiled = Script::Compile(script);
    if (tc.HasCaught()) {
        Local<Message> message = tc.Message();
        pos  << "[FAIL]" << NEXT_LINE
            << "<" << str_file.str() << ":" << message->GetLineNumber() << "> "
            << *String::Utf8Value(message->Get());
        return false;
    }

    pos << "[GOOD]" << NEXT_LINE << "Running";
    pos.col = STATUS;

    // Run the script
    Local<Value> result = compiled->Run();
    if (tc.HasCaught()) {
        Local<Message> message = tc.Message();
        pos << "[FAIL]" << NEXT_LINE
            << "<" << str_file << ":" << message->GetLineNumber() << "> "
            << *String::Utf8Value(message->Get());
        return false;
    }


    return true;
}

// Wrap the engine as an object so it can be exposed to the JavaScript
Handle<Object> Scripting_engine::wrap_class_as_object(
        Handle<FunctionTemplate>* function_tmpl,
        Scripting_engine* instance,
        Object_template_extension extensions)
{
    HandleScope scope;
    if (function_tmpl->IsEmpty())
        (*function_tmpl) = FunctionTemplate::New();
    generate_fun_tmpl(function_tmpl, accessors, functions, NULL);
    (*function_tmpl)->SetClassName(String::New("rotide"));
    Local<Function> ctor = (*function_tmpl)->GetFunction();
    Local<Object> obj = ctor->NewInstance();
    obj->SetInternalField(0, External::New(instance));
    return scope.Close(obj);
}

// JavaScript method: ro.test()
// Just tests various interactions with the engine.
FUNCTION_DEFINE(Scripting_engine, test)
{
    Scripting_engine* self = unwrap<Scripting_engine>(args.Holder());
    Local<Value> arg_x = args[0];
    Local<Value> arg_y = args[1];
    Local<Value> arg_str = args[2];

    int x = arg_x->Int32Value();
    int y = arg_y->Int32Value();
    std::string str = *String::Utf8Value(arg_str->ToString());

    self->curses->at(x,y) << "Says: " << str << NEXT_LINE;
    return Undefined();
}

// JavaScript method: ro.bind([Int32], String, Function)
// Binds a key combination list to a function callback.
//
// EXAMPLE:
//  ro.bind([ro.A, ro.B, ro.C], "doABC", function () { ro.status = "ABCs!"; })
FUNCTION_DEFINE(Scripting_engine, bind)
{
    // Unwrap object
    Scripting_engine* self = unwrap<Scripting_engine>(args.Holder());
    std::string cmd;
    Key_list keys;

    // Get keys and function
    Local<Value> key_repr(args[0]);
    Local<Value> cmd_repr(args[1]);
    Local<Value> function_repr(args[2]);

    // Convert the keys to a vector and insert into the local bindings
    if (smart_convert(key_repr, &keys) 
            && smart_convert(cmd_repr, &cmd)
            && function_repr->IsFunction()) {
        Local<Function> function 
            = Local<Function>::Cast(function_repr);
        self->bindings.insert(keys, cmd, function);
        return Undefined();
    } else {
        return Exception::TypeError(
                String::New(
                    "The definition of this method is: \
                    ro.bind([Keys], String, Function). You provided the \
                    wrong types for the arguments to this method."));
    }
}

// JavaScript function: ro.on_command (String, function (Args...))
// JavaScript function: ro.on_command (function (Command, Args...))
// Defines a callback for when a command is entered. A command is any
// successful CTRL+ modifier or explicit command.
FUNCTION_DEFINE(Scripting_engine, on_command)
{
    // Unwrap object
    Scripting_engine* self = unwrap<Scripting_engine>(args.Holder());

    // If the first argument is a function then we are doing the
    // generic case; it becomes the callback for any completed
    // command.
    if (args[0]->IsFunction()) {
        Local<Function> fun_val = Local<Function>::Cast(args[0]);
        Persistent<Function> function = Persistent<Function>::New(fun_val);
        self->attrs.on_command["*"].push_back(function);
        return Undefined();
    } else if (args[0]->IsString() && args[1]->IsFunction()) {
        Local<Value> cmd_repr = args[0];
        std::string cmd;
        Local<Function> fun_val = Local<Function>::Cast(args[1]);
        Persistent<Function> function = Persistent<Function>::New(fun_val);
        if (smart_convert(cmd_repr, &cmd)) {
            self->attrs.on_command[cmd].push_back(function);
            return Undefined();
        }
    }
        
    return Exception::TypeError(
                String::New(
                    "The definition of this method is: \
                    ro.bind([Keys], Function). You provided the wrong types \
                    for the arguments to this method."));

}

// JavaScript getter: ro.insert_mode : boolean
// If true the editor is in insert mode. Otherwise it is in a command mode.
// Returns the value of insert_mode
ACCESSOR_GETTER_DEFINE(Scripting_engine, insert_mode)
{
    HandleScope scope;
    Scripting_engine* self = unwrap<Scripting_engine>(info.Holder());
    Local<Value> insert_mode_repr;
    if (smart_convert(self->attrs.insert_mode, &insert_mode_repr)) {
        return insert_mode_repr;
    } else {
        return Exception::Error(
                String::New(
                    "Fatal error in converting type of insert_mode"));
    }
}
// JavaScript getter: ro.insert_mode : boolean
// If true the editor is in insert mode. Otherwise it is in a command mode.
// Sets the value of insert_mode
ACCESSOR_SETTER_DEFINE(Scripting_engine, insert_mode)
{
    Scripting_engine* self = unwrap<Scripting_engine>(info.Holder());
    if (!smart_convert(value, &self->attrs.insert_mode)) {
        Exception::Error(
                String::New(
                    "insert_mode is either true or false."));
    }
}

// JavaScript getter: ro.status : string
// Updates the status bar of the editor.
//
// TODO(justinvh): There needs to be a more succint way of doing this.
// Giving full access to the status bar can cause a lot of problems where
// multiple scripts want to take the status bar. Maybe doing a stack of status
// messages that can be history-ized for viewing is more appropriate.
ACCESSOR_GETTER_DEFINE(Scripting_engine, status)
{
    HandleScope scope;
    Scripting_engine* self = unwrap<Scripting_engine>(info.Holder());
    Local<Value> status_repr;
    if (smart_convert(self->attrs.status, &status_repr)) {
        return status_repr;
    } else {
        return Exception::Error(
                String::New(
                    "Fatal error in converting type of status"));
    }
}

// JavaScript getter: ro.status : string
// Updates the status bar of the editor.
//
// TODO(justinvh): There needs to be a more succint way of doing this.
// Giving full access to the status bar can cause a lot of problems where
// multiple scripts want to take the status bar. Maybe doing a stack of status
// messages that can be history-ized for viewing is more appropriate.

ACCESSOR_SETTER_DEFINE(Scripting_engine, status)
{
    Scripting_engine* self = unwrap<Scripting_engine>(info.Holder());
    if (!smart_convert(value, &self->attrs.status)) {
        Exception::Error(
                String::New(
                    "status is a string"));
    }
    self->curses->status() << self->attrs.status;
}


// JavaScript getters: ro.CTRL_<A..Z>: Int32
// Returns the corresponding value for a CTRL+<A..Z> press.

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_A) 
{ return Int32::New(CTRL_A); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_B)
{ return Int32::New(CTRL_B); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_C)
{ return Int32::New(CTRL_C); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_D)
{ return Int32::New(CTRL_D); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_E)
{ return Int32::New(CTRL_E); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_F)
{ return Int32::New(CTRL_F); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_G)
{ return Int32::New(CTRL_G); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_H)
{ return Int32::New(CTRL_H); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_I)
{ return Int32::New(CTRL_I); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_J)
{ return Int32::New(CTRL_J); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_K)
{ return Int32::New(CTRL_K); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_L)
{ return Int32::New(CTRL_L); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_M)
{ return Int32::New(CTRL_M); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_N)
{ return Int32::New(CTRL_N); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_O)
{ return Int32::New(CTRL_O); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_P)
{ return Int32::New(CTRL_P); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_Q)
{ return Int32::New(CTRL_Q); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_R)
{ return Int32::New(CTRL_R); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_S)
{ return Int32::New(CTRL_S); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_T)
{ return Int32::New(CTRL_T); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_U)
{ return Int32::New(CTRL_U); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_V)
{ return Int32::New(CTRL_V); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_W)
{ return Int32::New(CTRL_W); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_X)
{ return Int32::New(CTRL_X); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_Y)
{ return Int32::New(CTRL_Y); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, CTRL_Z)
{ return Int32::New(CTRL_Z); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, I)
{ return Int32::New(105); }

ACCESSOR_GETTER_DEFINE(Scripting_engine, ESC)
{ return Int32::New(27); }
