#include <iostream>
#include <vector>
#include <functional>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>

using namespace std;

void asyncFindCallback(
    const vector<int>& arr,
    function<bool(int)> predicate,
    function<void(int)> onSuccess,
    function<void()> onFail,
    atomic<bool>& cancelled
) {
    thread([=, &cancelled]() {
        for (int i = 0; i < arr.size(); i++) {

            if (cancelled) {
                cout << "Operation cancelled (callback)\n";
                return;
            }

            this_thread::sleep_for(chrono::milliseconds(300));

            if (predicate(arr[i])) {
                onSuccess(arr[i]);
                return;
            }
        }

        onFail();
        }).detach();
}

class SimplePromise {
private:
    function<void(int)> thenCallback;
    function<void()> catchCallback;

public:
    void then(function<void(int)> cb) {
        thenCallback = cb;
    }

    void onError(function<void()> cb) {
        catchCallback = cb;
    }

    void resolve(int value) {
        if (thenCallback) thenCallback(value);
    }

    void reject() {
        if (catchCallback) catchCallback();
    }
};
