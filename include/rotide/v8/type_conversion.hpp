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

#ifndef ROTIDE_V8_TYPE_CONVERSION_HPP
#define ROTIDE_V8_TYPE_CONVERSION_HPP

#include <v8.h>
#include <string>
#include <iterator>

template <class Type>
bool smart_convert(const v8::Handle<v8::Value>& from, Type* to);

template <class Type>
bool smart_convert(const Type& from, v8::Handle<v8::Value>* to);

template <class Type>
bool smart_get(const v8::Handle<v8::Object>& from, const char* key, Type* to);

bool convert(const v8::Handle<v8::Value>& from, bool* to);
bool convert(const bool& from, v8::Handle<v8::Value>* to);
bool convert(const v8::Handle<v8::Value>& from, int32_t* to);
bool convert(const int32_t& from, v8::Handle<v8::Value>* to);
bool convert(const v8::Handle<v8::Value>& from, double* to);
bool convert(const double& from, v8::Handle<v8::Value>* to);
bool convert(const v8::Handle<v8::Value>& from, float* to);
bool convert(const float& from, v8::Handle<v8::Value>* to);
bool convert(const v8::Handle<v8::Value>& from, char** to);
bool convert(const v8::Handle<v8::Value>& from, std::string* to);
bool convert(const std::string& from, v8::Handle<v8::Value>* to);

template <class Insert_container>
bool convert(const v8::Handle<v8::Value>& from, Insert_container* to)
{
  if (from->IsArray()) {
    v8::Handle<v8::Array> array_val = from->ToObject().As<v8::Array>();
    std::insert_iterator<Insert_container> insert_iter(*to, to->begin());
    for (uint32_t i = 0; i < array_val->Length(); i++) {
      typename Insert_container::value_type local;
      const v8::Handle<v8::Value> val = array_val->Get(i);
      if (!smart_convert(val, &local)) {
        return false;
      }
      insert_iter = local;
    }
    return true;
  }
  return false;
}

template <class Iterable_container>
bool convert(const Iterable_container& from, v8::Handle<v8::Value>* to)
{
  v8::Handle<v8::Array> array_list = v8::Array::New();
  int ith = 0;
  for (typename Iterable_container::const_iterator cit = from.begin();
    cit != from.end();
    ++cit, ith++)
  {
    v8::Handle<v8::Value> val;
    if (!smart_convert(*cit, val)) {
      return false;
    }
    array_list->Set(ith, val);
  }

  return true;
}

template <class Type>
bool smart_convert(const v8::Handle<v8::Value>& from, Type* to)
{
  if (to == NULL) return false;
  return convert(from, to);
}

template <class Type>
bool smart_convert(const Type& from, v8::Handle<v8::Value>* to)
{
  if (to == NULL) return false;
  return convert(from, to);
}

template <class Type>
bool smart_get(const v8::Handle<v8::Object>& from, const char* key, Type* to)
{
  v8::Handle<v8::Value> from_val = from->Get(v8::String::New(key));
  if (from_val->IsUndefined()) {
    return false;
  } else {
    return convert(from_val, to);
  }
}

#endif // ROTIDE_V8_TYPE_CONVERSION_HPP
