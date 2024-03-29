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

#ifndef HAT_GUI_EASY_HPP
#define HAT_GUI_EASY_HPP

#include <v8.h>
#include <vector>

/*
Generically unwraps a handle to a V8 object of a specified class.
If the object does not have an internal field count then a
nullptr will be returned.
*/
template <class T>
T* unwrap(v8::Handle<v8::Object> object_to_be_unwrapped)
{
    if (!object_to_be_unwrapped->InternalFieldCount()) {
        return NULL;
    }

    v8::Local<v8::External> wrapped = 
        v8::Local<v8::External>::Cast(object_to_be_unwrapped->GetInternalField(0));

    if (wrapped.IsEmpty()) {
        return NULL;
    }

    return static_cast<T*>(wrapped->Value());
}

/*
Performs a unwrap for Diamond-structured classes.
*/
#define diamond_unwrap(T, S) dynamic_cast<T*>(unwrap<Element>(S));

/*
Returns the internal pointer of the current v8 context and casts it to the
appropriate object.

Why can't we just reference Global()?
http://code.google.com/p/v8/source/browse/trunk/test/cctest/test-api.cc#1470
*/
template <class T>
T* unwrap_global_pointer(int index)
{

    v8::Handle<v8::Object> proxy = v8::Context::GetCurrent()->Global();
    v8::Handle<v8::Object> global = proxy->GetPrototype().As<v8::Object>();
    void* p = global->GetPointerFromInternalField(index);
    return reinterpret_cast<T*>(p);
}

/*
These are macros for generating setters and getters for any attribute that
should be translated between the JavaScript and engine.
*/
#define JS_INTERNAL(klass) JavaScript_class_##klass 

#define DEFINE(klass) struct JS_INTERNAL(klass)

/*
Macros for defining a getter relationship between a value controlled by the
class and an accessor from JavaScript.
*/
#define ACCESSOR_GETTER(method) \
    static v8::Handle<v8::Value> javascript_getter_##method(v8::Local<v8::String> name, \
        const v8::AccessorInfo& info)

#define ACCESSOR_GETTER_CALLER(klass, method) \
    klass::JavaScript_class_##klass::javascript_getter_##method

#define ACCESSOR_GETTER_DEFINE(klass, method) \
    v8::Handle<v8::Value> ACCESSOR_GETTER_CALLER(klass,method)(v8::Local<v8::String> name, \
        const v8::AccessorInfo& info)

/*
Macros for defining a setter relationship between a value controlled by the
class and an accessor from JavaScript.
*/
#define ACCESSOR_SETTER(method) \
    static void javascript_setter_##method(v8::Local<v8::String> name, v8::Local<v8::Value> value, \
        const v8::AccessorInfo& info)

#define ACCESSOR_SETTER_CALLER(klass, method) \
    klass::JavaScript_class_##klass::javascript_setter_##method

#define ACCESSOR_SETTER_DEFINE(klass, method) \
    void ACCESSOR_SETTER_CALLER(klass, method)(v8::Local<v8::String> name, \
        v8::Local<v8::Value> value, const v8::AccessorInfo& info)

/*
A wrapper around both the setter and getter for declarations
*/
#define ACCESSOR(method) \
    static v8::Handle<v8::Value> javascript_getter_##method(v8::Local<v8::String> name, \
        const v8::AccessorInfo& info); \
    static void javascript_setter_##method(v8::Local<v8::String> name, v8::Local<v8::Value> value, \
        const v8::AccessorInfo& info);

/*
Map methods to define accessors when constructing an initializer list for
JS_Mapping. See src/element.cpp for an example (accessors[])
*/
#define ACCESSOR_MAP(klass, method) \
    { #method , ACCESSOR_GETTER_CALLER(klass, method), ACCESSOR_SETTER_CALLER(klass, method), false }

#define ACCESSOR_INTERNAL(klass, method) \
    { #method , ACCESSOR_GETTER_CALLER(klass, method), ACCESSOR_SETTER_CALLER(klass, method), true }

#define ACCESSOR_CUSTOM(klass, method, name) \
    { name, ACCESSOR_GETTER_CALLER(klass, method), ACCESSOR_SETTER_CALLER(klass, method), false }

#define ACCESSOR_CUSTOM_INTERNAL(klass, method, name) \
    { name, ACCESSOR_GETTER_CALLER(klass, method), ACCESSOR_SETTER_CALLER(klass, method), true }

#define ACCESSOR_GETTER_MAP(klass, method) \
    { #method , ACCESSOR_GETTER_CALLER(klass, method), NULL, false }

#define ACCESSOR_GETTER_INTERNAL(klass, method) \
    { #method , ACCESSOR_GETTER_CALLER(klass, method), NULL, true}

#define ACCESSOR_SETTER_MAP(klass, method) \
    { #method , NULL, ACCESSOR_SETTER_CALLER(klass, method), false }

#define ACCESSOR_SETTER__INTERNAL(klass, method) \
    { #method , NULL, ACCESSOR_SETTER_CALLER(klass, method), true }

/*
A wrapper around defining functions.
*/
#define FUNCTION_CALLER(klass, method) \
    klass::JavaScript_class_##klass::javascript_function_##method

#define FUNCTION(method) \
    static v8::Handle<v8::Value> javascript_function_##method(const v8::Arguments& args)

#define FUNCTION_DEFINE(klass, method) \
    v8::Handle<v8::Value> FUNCTION_CALLER(klass, method)(const v8::Arguments& args)

/*
Map methods to define functions when constructing an intializer list for
Function_mapping.
*/
#define FUNCTION_MAP(klass, method) \
    { #method , FUNCTION_CALLER(klass, method), false }

#define FUNCTION_MAP_INTERNAL(klass, method) \
    { #method , FUNCTION_CALLER(klass, method), true }

#define JS_CLASS_INVOCATION(klass) \
    { #klass, klass::create, false }

#define JS_CLASS_INVOCATION_CUSTOM(klass, name) \
    { name, klass::create, false }
/*
These typedefs are used for creating the shorthand mapping of the name to
function setters and getters.
*/
typedef v8::Handle<v8::Value> (*JS_getter)(v8::Local<v8::String>, const v8::AccessorInfo&);
typedef void (*JS_setter)(v8::Local<v8::String>, v8::Local<v8::Value>, const v8::AccessorInfo&);
typedef v8::Handle<v8::Value> (*JS_fun)(const v8::Arguments&);
typedef void (Object_template_extension)(v8::Handle<v8::ObjectTemplate>*);


/*
A mapping is a relation between a javascript key and a corresponding setter 
and getter. This is used to define the available accessors. 
See src/element.cpp's accessor initializer list (accessors[])
*/
struct Accessors
{
    const char* name;
    JS_getter getter;
    JS_setter setter;
    bool is_internal;
};

/*
A function mapping is a relationship between a javascript key and a
corresponding function. This is used to define the available functions.
*/
struct Function_mapping
{
    const char* name;
    JS_fun fun;
    bool is_internal;
};


typedef std::pair<Accessors*, Function_mapping*> Mapping_pair;
typedef std::vector<Mapping_pair> Extension_list;
typedef std::vector<v8::Handle<v8::Function> > Function_list;

/*
Accessors provide a method getting and setting variables associated with
the class binding to an object. Functions are methods that can be called
from the JavaScript that have some sort of impact on the engine itself.

An internal method is one that will be in the *.internal.* namespace.
*/

inline
void add_accessors_and_fun_to_fun_tmpl(
    const Accessors* accessors,
    const Function_mapping* funs,
    v8::Handle<v8::FunctionTemplate>* tmpl,
    const bool internal_methods)
{
    v8::Handle<v8::ObjectTemplate> proto = (*tmpl)->PrototypeTemplate();
    v8::Handle<v8::ObjectTemplate> inst = (*tmpl)->InstanceTemplate();
    for (int i = 0; accessors[i].name != NULL; i++) {
        if (internal_methods && !accessors[i].is_internal)
            continue;

        inst->SetAccessor(
            v8::String::NewSymbol(accessors[i].name), 
            accessors[i].getter,
            accessors[i].setter);
    }

    for (int i = 0; funs[i].name != NULL; i++) {
        if (funs[i].is_internal && !internal_methods)
            continue;
        
        proto->Set(
            v8::String::NewSymbol(funs[i].name),
            v8::FunctionTemplate::New(funs[i].fun));
    }
}

/*
Createing a template is essentially creating a new object template that
reserves a field for the to-be-determined class that will be binded in
the internal pointer.
*/
inline
v8::Handle<v8::FunctionTemplate> generate_fun_tmpl(
    v8::Handle<v8::FunctionTemplate>* tmpl,
    const Accessors* accessors, 
    const Function_mapping* funs,
    const Extension_list* extension_list)
{
    v8::Handle<v8::ObjectTemplate> inst = (*tmpl)->InstanceTemplate();
    inst->SetInternalFieldCount(2);

    add_accessors_and_fun_to_fun_tmpl(accessors, funs, tmpl, false);
    add_accessors_and_fun_to_fun_tmpl(accessors, funs, tmpl, true);

    if (extension_list) {
        for (Extension_list::const_iterator ecit = extension_list->begin();
            ecit != extension_list->end();
            ++ecit)
        {
            add_accessors_and_fun_to_fun_tmpl(ecit->first, ecit->second, tmpl, false);
            add_accessors_and_fun_to_fun_tmpl(ecit->first, ecit->second, tmpl, false);
        }
    }

    return *tmpl;
}


#define JS_STR_TO_STL(v8str) *v8::String::Utf8Value(v8str)

#define JS_BA_STR(lhs, rhs, error) \
    if (!rhs->IsString() && !rhs->IsUndefined()) { \
        v8::ThrowException(v8::Exception::TypeError(v8::String::New(error))); \
        return false; \
    } \
    lhs = JS_STR_TO_STL(rhs->ToString());

#define JS_BA_STR_REQUIRED(lhs, rhs, error) \
    if (!rhs->IsString()) { \
        v8::ThrowException(v8::Exception::TypeError(v8::String::New(error))); \
        return false; \
    } \
    lhs = JS_STR_TO_STL(rhs->ToString());

#define JS_BA_INT(lhs, rhs, error) \
    if (!rhs->IsInt32() && !rhs->IsUndefined()) { \
        v8::ThrowException(v8::Exception::TypeError(v8::String::New(error))); \
        return false; \
    } \
    lhs = rhs->Int32Value();


#define JS_BA_INT_REQUIRED(lhs, rhs, error) \
    if (!rhs->IsInt32()) { \
        v8::ThrowException(v8::Exception::TypeError(v8::String::New(error))); \
        return false; \
    } \
    lhs = rhs->Int32Value();

#define JS_BA_FLOAT(lhs, rhs, error) \
    if (!rhs->IsNumber() && !rhs->IsUndefined()) { \
        v8::ThrowException(v8::Exception::TypeError(v8::String::New(error))); \
        return false; \
    } else if (rhs->IsNumber()) { \
        lhs = rhs->NumberValue(); \
    }

#define JS_BA_FLOAT_REQUIRED(lhs, rhs, error) \
    if (!rhs->IsNumber()) { \
        v8::ThrowException(v8::Exception::TypeError(v8::String::New(error))); \
        return false; \
    } \
    lhs = rhs->NumberValue();

#define JS_BA_BOOLEAN(lhs, rhs, error) \
    if (!rhs->IsBoolean() && rhis->IsUndefined()) { \
        v8::ThrowException(v8::Exception::TypeError(v8::String::New(error))); \
        return false; \
    } \
    lhs = rhs->BooleanValue();

#define JS_BA_BOOLEAN_REQUIRED(lhs, rhs, error) \
    if (!rhs->IsBoolean()) { \
        v8::ThrowException(v8::Exception::TypeError(v8::String::New(error))); \
        return false; \
    } \
    lhs = rhs->BooleanValue();

#define JS_BA_FUNCTION(type, fun_list, alt) \
    v8::Local<v8::Object> menu_obj = args[0]->ToObject(); \
    Element* b = unwrap<Element>(args.Holder()); \
    type* e = dynamic_cast<type*>(b); \
    \
    if (!e) { \
        return v8::Exception::Error(v8::String::New(#type " has become detached?")); \
    } \
    \
    if (args.Length() > 0) { \
        v8::Handle<v8::Value> fun_val = args[0]; \
        if (fun_val->IsFunction()) { \
            v8::Handle<v8::Function> fun = v8::Handle<v8::Function>::Cast(fun_val); \
            e->fun_list.push_back(v8::Persistent<v8::Function>::New(fun)); \
            return args.Holder(); \
        } else { \
            return v8::Exception::TypeError(v8::String::New("Expected a function, but got something else.")); \
        } \
    } \
    alt; \
    return args.Holder(); \

#define NULL_EOL { NULL, NULL, NULL }

#endif // HAT_GUI_EASY_HPP
