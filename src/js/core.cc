#include <rotide/scripting.hpp>
#include <rotide/js/core.hpp>

namespace {
Accessors accessors[] = {
    { NULL, NULL, NULL }
};

Function_mapping functions[] = {
    FUNCTION_MAP(Core, enable),
    { NULL, NULL, NULL }
};
}

v8::Handle<v8::Object> Core::wrap_class_as_object(Scripting_engine* eng)
{
    v8::HandleScope scope;
    v8::Handle<v8::FunctionTemplate> core_tmpl = v8::FunctionTemplate::New();
    generate_fun_tmpl(&core_tmpl, accessors, functions, NULL);
    core_tmpl->SetClassName(v8::String::New("core"));
    v8::Handle<v8::Object> core = core_tmpl->GetFunction()->NewInstance();
    return scope.Close(core);
}

FUNCTION_DEFINE(Core, enable)
{
    return v8::Undefined();
}
