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
