#include <rotide/scripting.hpp>
#include <rotide/curses.hpp>

#include <sstream>
#include <fstream>
#include <cassert>
#include <iostream>

Scripting_engine::Scripting_engine(Curses* curses)
    : curses(curses)
{
    // Read in the source
    Curses_pos pos = curses->at(0, 0);
    Curses_color COLOR_GREEN = COLOR(GREEN, BLACK);
    Curses_color COLOR_RED = COLOR(RED, BLACK);
    const char* RC_FILE = "rotide/rotide.js";
    const int STATUS = 55;

    pos << "Rotide Scripting Engine" << NEXT_LINE;
    pos << "-------------------------------------------------------------"
        << NEXT_LINE << COLOR_GREEN 
        << "Initializing scripting engine." ;

    std::ifstream rc(RC_FILE, std::ifstream::in);

    pos.col = STATUS;
    if (!rc.good()) {
        pos << COLOR_RED << "[FAIL]" << NEXT_LINE
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

    // Set the internal field to a reference of this
    // Allow the object to reference itself.
    v8::Handle<v8::Object> proxy = context->Global();
    v8::Handle<v8::Object> proto = proxy->GetPrototype().As<v8::Object>();
    proto->SetPointerInInternalField(0, this);
    //proto->Set(v8::String::New("ro"), object);

    pos << "[GOOD]" << NEXT_LINE << "Compiling";
    pos.col = STATUS;

    // Compile the code
    v8::TryCatch tc;
    v8::Handle<v8::Script> compiled = v8::Script::Compile(script);
    if (tc.HasCaught()) {
        v8::Handle<v8::Message> message = tc.Message();
        pos << COLOR_RED << "[FAIL]" << NEXT_LINE
            << "<" << RC_FILE << ":" << message->GetLineNumber() << "> "
            << *v8::String::Utf8Value(message->Get());
        return;
    }

    pos << "[GOOD]" << NEXT_LINE << "Running";
    pos.col = STATUS;

    // Run the script
    v8::Handle<v8::Value> result = compiled->Run();
    if (tc.HasCaught()) {
        v8::Handle<v8::Message> message = tc.Message();
        pos << COLOR_RED << "[FAIL]" << NEXT_LINE
            << "<" << RC_FILE << ":" << message->GetLineNumber() << "> "
            << *v8::String::Utf8Value(message->Get());
        return;
    }

    pos << "[GOOD]" << NEXT_LINE;
}
