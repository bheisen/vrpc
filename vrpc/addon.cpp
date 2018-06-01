/*
__/\\\________/\\\____/\\\\\\\\\______/\\\\\\\\\\\\\_________/\\\\\\\\\_
__\/\\\_______\/\\\__/\\\///////\\\___\/\\\/////////\\\____/\\\////////__
 __\//\\\______/\\\__\/\\\_____\/\\\___\/\\\_______\/\\\__/\\\/___________
  ___\//\\\____/\\\___\/\\\\\\\\\\\/____\/\\\\\\\\\\\\\/__/\\\_____________
   ____\//\\\__/\\\____\/\\\//////\\\____\/\\\/////////___\/\\\_____________
    _____\//\\\/\\\_____\/\\\____\//\\\___\/\\\____________\//\\\____________
     ______\//\\\\\______\/\\\_____\//\\\__\/\\\_____________\///\\\__________
      _______\//\\\_______\/\\\______\//\\\_\/\\\_______________\////\\\\\\\\\_
       ________\///________\///________\///__\///___________________\/////////__


Enables node.js to run vrpc as native addon.
Author: Dr. Burkhard C. Heisen (https://github.com/bheisen/vrpc)


Licensed under the MIT License <http://opensource.org/licenses/MIT>.
Copyright (c) 2018 Dr. Burkhard C. Heisen <burkhard.heisen@xsmail.com>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <node.h>
#include "vrpc.hpp"
#include "json.hpp"

#ifdef VRPC_COMPILE_AS_ADDON
  #include VRPC_COMPILE_AS_ADDON
#endif

namespace vrpc_bindings {

  using v8::Exception;
  using v8::Function;
  using v8::FunctionCallbackInfo;
  using v8::HandleScope;
  using v8::Isolate;
  using v8::Local;
  using v8::Object;
  using v8::Persistent;
  using v8::String;
  using v8::Value;

  void callCpp(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    // Check the number of arguments passed
    if (args.Length() < 1) {
      // Throw an Error that is passed back to JavaScript
      isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(
            isolate,
            "Wrong number of arguments, expecting exactly one")
          )
      );
      return;
    }

    // Check the argument type
    if (!args[0]->IsString()) {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong argument type, expecting string"))
      );
      return;
    }

    // Perform the operation
    Local<String> v8String = args[0]->ToString();
    std::vector<char> utf8Buffer(v8String->Utf8Length() + 1, '0');
    args[0]->ToString()->WriteUtf8(&utf8Buffer[0]);
    std::string value;
    try {
      value = vrpc::LocalFactory::call(std::string(&utf8Buffer[0]));
    } catch (const std::exception& e) {
      isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, e.what()))
      );
      return;
    }
    Local<String> localString = String::NewFromUtf8(isolate, value.c_str());

    // Set the return value (using the passed in
    // FunctionCallbackInfo<Value>&)
    args.GetReturnValue().Set(localString);
  }

  void loadBindings(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    // Check the number of arguments passed
    if (args.Length() < 1) {
      // Throw an Error that is passed back to JavaScript
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(
          isolate,
          "Wrong number of arguments, expecting exactly one")
        )
      );
      return;
    }

    // Check the argument type
    if (!args[0]->IsString()) {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong argument type, expecting string"))
      );
      return;
    }

    // Perform the operation
    Local<String> v8String = args[0]->ToString();
    std::vector<char> utf8Buffer(v8String->Utf8Length() + 1, '0');
    args[0]->ToString()->WriteUtf8(&utf8Buffer[0]);
    try {
      vrpc::LocalFactory::load_bindings(std::string(&utf8Buffer[0]));
    } catch (const std::exception& e) {
      isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, e.what())));
      return;
    }
  }

  void getMemberFunctions(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    // Check the number of arguments passed
    if (args.Length() < 1) {
      // Throw an Error that is passed back to JavaScript
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(
          isolate,
          "Wrong number of arguments, expecting exactly one")
        )
      );
      return;
    }

    // Check the argument type
    if (!args[0]->IsString()) {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong argument type, expecting string"))
      );
      return;
    }

    // Perform the operation
    Local<String> v8String = args[0]->ToString();
    std::vector<char> utf8Buffer(v8String->Utf8Length() + 1, '0');
    args[0]->ToString()->WriteUtf8(&utf8Buffer[0]);
    std::string value;
    try {
      auto functions = vrpc::LocalFactory::get_member_functions(
        std::string(&utf8Buffer[0])
      );
      nlohmann::json j;
      j["functions"] = functions;
      value = j.dump();
    } catch (const std::exception& e) {
      isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, e.what())));
      return;
    }
    Local<String> localString = String::NewFromUtf8(isolate, value.c_str());
    args.GetReturnValue().Set(localString);
  }

  void getStaticFunctions(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    // Check the number of arguments passed
    if (args.Length() < 1) {
      // Throw an Error that is passed back to JavaScript
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(
          isolate,
          "Wrong number of arguments, expecting exactly one")
        )
      );
      return;
    }

    // Check the argument type
    if (!args[0]->IsString()) {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong argument type, expecting string"))
      );
      return;
    }

    // Perform the operation
    Local<String> v8String = args[0]->ToString();
    std::vector<char> utf8Buffer(v8String->Utf8Length() + 1, '0');
    args[0]->ToString()->WriteUtf8(&utf8Buffer[0]);
    std::string value;
    try {
      auto functions = vrpc::LocalFactory::get_static_functions(
        std::string(&utf8Buffer[0])
      );
      nlohmann::json j;
      j["functions"] = functions;
      value = j.dump();
    } catch (const std::exception& e) {
        isolate->ThrowException(Exception::Error(
          String::NewFromUtf8(isolate, e.what()))
        );
        return;
    }
    Local<String> localString = String::NewFromUtf8(isolate, value.c_str());
    args.GetReturnValue().Set(localString);
  }

  Persistent<Function> callback_handler;

  void cppCallbackHandler(Isolate* isolate, const nlohmann::json& json) {
    _VRPC_DEBUG << "will call back with " << json << std::endl;
    HandleScope handleScope(isolate);
    Local<Function> cb = Local<Function>::New(isolate, callback_handler);
    const unsigned argc = 1;
    Local<Value> argv[argc] = {
      String::NewFromUtf8(isolate, json.dump().c_str())
    };
    cb->Call(Null(isolate), argc, argv);
  }

  void onCallback(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    callback_handler.Reset(isolate, Local<Function>::Cast(args[0]));
    vrpc::Callback::register_callback_handler(
      std::bind(cppCallbackHandler, isolate, std::placeholders::_1)
    );
  }

  void Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "loadBindings", loadBindings);
    NODE_SET_METHOD(exports, "getMemberFunctions", getMemberFunctions);
    NODE_SET_METHOD(exports, "getStaticFunctions", getStaticFunctions);
    NODE_SET_METHOD(exports, "callCpp", callCpp);
    NODE_SET_METHOD(exports, "onCallback", onCallback);
  }

  NODE_MODULE(vrpc, Init)
}
