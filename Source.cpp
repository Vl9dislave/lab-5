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
shared_ptr<SimplePromise> asyncFindPromise(
    const vector<int>& arr,
    function<bool(int)> predicate,
    atomic<bool>& cancelled
) {
    auto promise = make_shared<SimplePromise>();

    thread([=, &cancelled]() {
        for (int i = 0; i < arr.size(); i++) {

            if (cancelled) {
                cout << "Operation cancelled (promise)\n";
                return;
            }

            this_thread::sleep_for(chrono::milliseconds(300));

            if (predicate(arr[i])) {
                promise->resolve(arr[i]);
                return;
            }
        }

        promise->reject();
        }).detach();

    return promise;
}

int asyncFindAwait(
    const vector<int>& arr,
    function<bool(int)> predicate,
    atomic<bool>& cancelled
) {
    for (int i = 0; i < arr.size(); i++) {

        if (cancelled) {
            throw runtime_error("Cancelled");
        }

        this_thread::sleep_for(chrono::milliseconds(300));

        if (predicate(arr[i])) {
            return arr[i];
        }
    }

    throw runtime_error("Not found");
}

int main() {
    vector<int> arr = { 1, 3, 5, 8, 10, 13 };
    atomic<bool> cancelled(false);

    cout << "=== Callback version ===\n";

    asyncFindCallback(
        arr,
        [](int x) { return x % 2 == 0; },
        [](int result) {
            cout << "Found (callback): " << result << endl;
        },
        []() {
            cout << "Not found (callback)\n";
        },
        cancelled
    );

    this_thread::sleep_for(chrono::seconds(2));

    cout << "\n=== Promise version ===\n";

    auto promise = asyncFindPromise(
        arr,
        [](int x) { return x > 9; },
        cancelled
    );
    promise->then([](int result) {
        cout << "Found (promise): " << result << endl;
        });

    promise->onError([]() {
        cout << "Not found (promise)\n";
        });

    this_thread::sleep_for(chrono::seconds(2));

    cout << "\n=== Async/Await version ===\n";

    try {
        int result = asyncFindAwait(
            arr,
            [](int x) { return x == 5; },
            cancelled
        );