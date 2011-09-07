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

/*
 * TODO(justinvh): DANGER WILL ROBINSON
 * This file is undocumented for now. 
 */

#include <rotide/v8/type_conversion.hpp>
#include <cstring>

bool convert(const v8::Handle<v8::Value>& from, bool* to)
{
  if (from->IsBoolean()) {
    (*to) = from->BooleanValue();
    return true;
  }
  return false;
}

bool convert(const bool& from, v8::Handle<v8::Value>* to)
{
  (*to) = v8::Boolean::New(from);
  return true;
}

bool convert(const v8::Handle<v8::Value>& from, int32_t* to)
{
  if (from->IsInt32()) {
    (*to) = from->Int32Value();
    return true;
  }
  return false;
}

bool convert(const int32_t& from, v8::Handle<v8::Value>* to)
{
  (*to) = v8::Int32::New(from);
  return true;
}

bool convert(const v8::Handle<v8::Value>& from, int64_t* to)
{
  if (from->IsNumber() && !from->IsUndefined()) {
    (*to) = from->IntegerValue();
    return true;
  }
  return false;
}

bool convert(const v8::Handle<v8::Value>& from, double* to)
{
  if (from->IsNumber()) {
    (*to) = from->NumberValue();
    return true;
  }
  return false;
}

bool convert(const double& from, v8::Handle<v8::Value>* to)
{
  (*to) = v8::Number::New(from);
  return true;
}

bool convert(const v8::Handle<v8::Value>& from, float* to)
{
  if (from->IsNumber()) {
    (*to) = (float)from->NumberValue();
    return true;
  }
  return false;
}

bool convert(const float& from, v8::Handle<v8::Value>* to)
{
  (*to) = v8::Number::New(from);
  return true;
}

bool convert(const v8::Handle<v8::Value>& from, char** to)
{
  if (from->IsString()) {
    v8::Local<v8::String> string_val = from->ToString();
    char* s = new char[string_val->Length()];
    strcpy(s, *v8::String::Utf8Value(string_val));
    *to = s;
    return true;
  }
  return false;
}

bool convert(const v8::Handle<v8::Value>& from, std::string* to)
{
  if (from->IsString()) {
    v8::Local<v8::String> string_val = from->ToString();
    (*to) = *v8::String::Utf8Value(string_val);
    return true;
  }
  return false;
}

bool convert(const std::string& from, v8::Handle<v8::Value>* to)
{
  (*to) = v8::String::New(from.c_str(), from.size());
  return true;
}
