#include <rotide/scripting.hpp>
#include <rotide/curses.hpp>
#include <rotide/js/core.hpp>
#include <rotide/v8/type_conversion.hpp>

#include <sstream>
#include <fstream>
#include <cassert>
#include <iostream>
#include <string>

namespace {
Accessors accessors[] = {
    ACCESSOR_MAP(Scripting_engine, insert_mode),
    { NULL, NULL, NULL }
};

Function_mapping functions[] = {
    FUNCTION_MAP(Scripting_engine, test),
    { NULL, NULL, NULL }
};

const int STATUS = 55;

}

Scripting_engine::Scripting_engine(Curses* curses)
    : curses(curses)
{
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
            << "<" << str_file << ":" << message->GetLineNumber() << "> "
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
}


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
