//
// Created by fed on 2017/8/18.
//

#include <nan.h>

#include <stdio.h>

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>


class AsyncWorker {
public:
    AsyncWorker(size_t m, size_t n) : thread_count(n), current_used(1), friend_array(nullptr) {
      min = static_cast<size_t>(m / (n * 1.33));
    };
    AsyncWorker(size_t m, bool merge, size_t n) : thread_count(n), current_used(1) {
      min = static_cast<size_t>(m / (n * 1.33));
      if (merge) friend_array = new double[m];
      memset(friend_array, m, 0);
    };
    ~AsyncWorker() {
      if (friend_array) delete[] friend_array;
    };

    double* GetFriendArray() {
      return  friend_array;
    };

    std::unique_ptr <std::thread> async(std::function<void()> &&lamb);

    size_t getMin() {
      return min;
    };

private:
    size_t thread_count;
    size_t min;
    size_t current_used;
    std::mutex lock;
    double* friend_array;
};

std::unique_ptr <std::thread> AsyncWorker::async(std::function<void()> &&lamb) {
  bool shouldNewThread = false;
  {
    std::lock_guard <std::mutex> lock_guard(lock);
    if (thread_count > current_used) {
      ++current_used;
      shouldNewThread = true;
    }
  }
  if (shouldNewThread) {
    return std::unique_ptr<std::thread>(new std::thread(std::move(lamb)));
  }
  lamb();
  return nullptr;
}

const size_t NChangeAlgorithm = 10;


template<typename T>
void SortPick(T obj, size_t start, size_t end) {
  for (auto i = start; i < end; i++) {
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

void mergeSort(AsyncWorker &w, double* obj, size_t start, size_t end) {
  if (end - start < NChangeAlgorithm)
    return SortPick(obj, start, end);
  auto mid = (end - start) / 2 + start;
  auto pro = w.async([=, &w] {
      mergeSort(w, obj, start, mid - 1);
  });
  mergeSort(w, obj, mid, end);
  if (pro.get())
    pro->join();

  auto friend_array = w.GetFriendArray();
  auto i = start;
  auto j = mid;
  auto f = start;
  while ((i < mid) && (j < end)) {
    if (obj[i] < obj[j]) {
      friend_array[f++] = obj[i++];
    } else {
      friend_array[f++] = obj[j++];
    }
  }
  while (i < mid) {
    friend_array[f++] = obj[i++];
  }
  while (j < end) {
    friend_array[f++] = obj[j++];
  }
  memmove(obj + start, friend_array, end - start);
}

template<typename T>
void SortInternal(AsyncWorker &w, T obj, size_t start, size_t end) {
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
      last_one--;
    }
  }
  obj[last_one - 1] = pivot;
  std::unique_ptr <std::thread> pro(nullptr);
  if (last_one - start > w.getMin() && last_one - end > w.getMin()) {
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


void Sort(const Nan::FunctionCallbackInfo <v8::Value> &info) {
  Nan::TypedArrayContents<double> array(info[0]);
  AsyncWorker worker(array.length(), true, std::thread::hardware_concurrency());
  mergeSort(worker, *array, 0, array.length());
  info.GetReturnValue().Set(info[0]);
}

void Init(v8::Local <v8::Object> exports) {
  exports->Set(Nan::New("sort").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(Sort)->GetFunction());
}

NODE_MODULE(mt_sort, Init)