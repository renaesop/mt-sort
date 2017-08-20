//
// Created by fed on 2017/8/18.
//

#include <nan.h>

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>


class AsyncWorker {
public:
  AsyncWorker(size_t n = 8): thread_count(n), current_used(0) {};
  std::unique_ptr<std::thread> async(std::function<void ()>&& lamb);
private:
  size_t thread_count;
  size_t current_used;
  std::mutex lock;
};

std::unique_ptr<std::thread> AsyncWorker::async(std::function<void ()>&& lamb) {
  bool shouldNewThread = false;
  {
    std::lock_guard<std::mutex> lock_guard(lock);
    if (thread_count > current_used) {
      ++current_used;
      shouldNewThread = true;
    }
  }
  if (shouldNewThread) {
    return std::make_unique<std::thread>(std::move(lamb));
  }
  lamb();
  return nullptr;
}

const size_t NChangeAlgorithm = 10;

template <typename T>
void SortPick(T obj, size_t start, size_t end) {
  for (auto i = start; i < end; i ++) {
    auto val = obj[i];
    auto j = i;
    for (; j > start; j--) {
      auto temp = obj[j - 1];
      if (temp < val) {
        break;
      }
      obj[j] = temp;
    }
    obj[j] = val;
  }
}

template <typename T>
void SortInternal(AsyncWorker& w, T obj, size_t start, size_t end) {
  if (end - start < NChangeAlgorithm)
    return SortPick(obj, start, end);
  auto pivot = obj[start];
  auto last_one = end;
  auto i = start + 1;
  while (i < last_one) {
    auto val = obj[i];
    if (val < pivot) {
      obj[i - 1] = val;
      ++i;
    } else {
      obj[i] = obj[last_one - 1];
      obj[last_one - 1] = val;
      last_one --;
    }
  }
  obj[last_one - 1] = pivot;
  std::unique_ptr<std::thread> pro(nullptr); 
  if (last_one - start > 512) {
    pro = w.async([=, &w] {
     SortInternal(w, obj, start, last_one - 1);
    });
  } else {
    SortInternal(w, obj, start, last_one - 1);
  }
  SortInternal(w, obj, last_one, end);
  if (pro.get())
    pro->join();
}


void Sort(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Nan::TypedArrayContents<double> array(info[0]);
  AsyncWorker worker(8);
  SortInternal(worker, *array, 0, array.length());
  info.GetReturnValue().Set(info[0]);
}

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("sort").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(Sort)->GetFunction());
}

NODE_MODULE(mt_sort, Init)