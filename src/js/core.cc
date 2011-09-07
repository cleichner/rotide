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
#include <rotide/js/core.hpp>
#include <rotide/curses.hpp>

using namespace v8;

// Define the accessors and functions to the JavaScript core.
//
// ro.core =
//      enable  : function ([programs])
//
namespace {

Accessors accessors[] = {
    { NULL, NULL, NULL }
};

Function_mapping functions[] = {
    FUNCTION_MAP(Core, enable),
    { NULL, NULL, NULL }
};

} // namespace

// Wrap the class as an object so it can be exposed to the JavaScript
Handle<Object> Core::wrap_class_as_object(Scripting_engine* eng)
{
    HandleScope scope;
    Handle<FunctionTemplate> core_tmpl = FunctionTemplate::New();
    generate_fun_tmpl(&core_tmpl, accessors, functions, NULL);
    core_tmpl->SetClassName(String::New("core"));
    Handle<Object> core = core_tmpl->GetFunction()->NewInstance();
    core->SetPointerInInternalField(0, eng);
    return scope.Close(core);
}

// JavaScript method: ro.core.enable()
// Enables units in the engine that are defined as JavaScript modules.
FUNCTION_DEFINE(Core, enable)
{
    Scripting_engine* engine = unwrap<Scripting_engine>(args.Holder());
    Handle<Array> array_val = args[0]->ToObject().As<Array>();

    for (int i = 0, l = array_val->Length(); i < l; i++) {
        engine->load(*String::AsciiValue(array_val->Get(i)));
    }

    return Undefined();
}
