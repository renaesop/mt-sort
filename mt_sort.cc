//
// Created by fed on 2017/8/18.
//

#include <nan.h>

#include <iostream>
#include <future>


const uint32_t NChangeAlgorithm = 20;

bool operator< (const v8::Local<v8::Value>& a,  const v8::Local<v8::Value>& b) {
  return a->NumberValue() < b->NumberValue();
}

template <typename T>
void SortPick(T& obj, uint32_t start, uint32_t end) {
  for (auto i = start; i < end; i ++) {
    auto val = obj->Get(i);
    auto j = i;
    for (; j > start; j--) {
      auto temp = obj->Get(j - 1);
      if (temp < val) {
        break;
      }
      obj->Set(j, temp);
    }
    obj->Set(j, val);
  }
}

template <typename T>
void SortInternal(T& obj, uint32_t start, uint32_t end) {
  std::cout << start << end << std::endl;
  if (end - start < NChangeAlgorithm)
    return SortPick(obj, start, end);
  auto pivot = obj->Get(start);
  auto last_one = end;
  auto i = start + 1;
  while (i < last_one) {
    auto val = obj->Get(i);
    if (val < pivot) {
      obj->Set(i - 1, val);
    } else {
      obj->Set(i, obj->Get(last_one - 1));
      obj->Set(last_one - 1, val);
      last_one --;
    }
  }
  obj->Set(last_one - 1, pivot);
  auto pro1 = std::async(std::launch::async, [=, &obj]() {
      return SortInternal(obj, start, last_one - 1);
  });
  auto pro2 = std::async(std::launch::async, [=, &obj]() {
      return SortInternal(obj, last_one, end);
  });
  pro1.get();
  pro2.get();
}

void Sort(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  auto array = v8::Local<v8::Array>::Cast(info[0]);
  SortInternal(array, 0, array->Length());
  info.GetReturnValue().Set(array);
}

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("sort").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(Sort)->GetFunction());
}

NODE_MODULE(mt_sort, Init)