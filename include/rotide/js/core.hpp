#ifndef ROTIDE_JS_CORE_HPP
#define ROTIDE_JS_CORE_HPP

#include <rotide/v8/easy.hpp>

class Scripting_engine;

class Core {
public:
    static v8::Handle<v8::Object> wrap_class_as_object(Scripting_engine* eng);
public:
    DEFINE(Core)
    {
        FUNCTION(enable);
    };
};

#endif // ROTIDE_JS_CORE_HPP
