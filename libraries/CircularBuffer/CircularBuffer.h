#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <inttypes.h>

template <typename T, uint16_t size>
class CircularBuffer {
public:
    CircularBuffer()
        : _writePtr(_buf), _readPtr(_buf), _tailPtr(_buf + size), _unread(0)
    {}
    
    void push(T value) {
        *_writePtr = value;
        _writePtr += 1;
        _unread += 1;
        if (_writePtr == _tailPtr) {
            _writePtr = _buf;
        }
    }

    T pop() {
        T result = *_readPtr;
        _readPtr += 1;
        _unread -= 1;
        if (_readPtr == _tailPtr) {
            _readPtr = _buf;
        }
        return result;
    }

    T popn(uint16_t n) {
        T result;
        for (uint16_t i = 0; i < n; ++i) {
            result = pop();
        }
        return result;
    }

    T peek(uint16_t index) {
        T *peekPtr = _readPtr;
        peekPtr = _readPtr + index;
        while (peekPtr >= _tailPtr) {
            // Reading beyond tail, wrap around.
            peekPtr -= size;
        }
        return *peekPtr;
    }

    uint16_t unread() const {
        return _unread;
    }

private:
    T _buf[size];
    T *_writePtr;
    T *_readPtr;
    T *_tailPtr;
    uint16_t _unread;
};

#ifdef TESTCASE_CIRCULARBUFFER

#define TEST(condition, msg) success &= (condition); if (!success) { cerr << "FAIL: " << msg; return 1; }

#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
    bool success = true;
    CircularBuffer<char, 5> cb;
    char c;

    TEST(cb.unread() == 0, "unread == 0 after initialization")
    cb.push('A');
    TEST(cb.unread() == 1, "unread == 1 after pushing character")
    c = cb.pop();
    TEST(c == 'A', "c == A after pushing and popping A")
    TEST(cb.unread() == 0, "unread == 0 after popping")
    cb.push('A');
    cb.push('B');
    cb.push('C');
    cb.push('D');
    cb.push('E');
    cb.push('F');
    TEST(cb.unread() == 6, "unread == 6 after pushing 6 characters")
    TEST(cb.peek(0) == 'F', "peek 0 == 'F'")
    TEST(cb.peek(1) == 'B', "peek 1 == 'B'")
    TEST(cb.peek(4) == 'E', "peek 4 == 'E'")
    TEST(cb.peek(5) == 'F', "peek 5 == 'F' (error, reading beyond buffer)")
    c = cb.pop();
    TEST(c == 'F', "c == F after pushing 5 times and popping")
    TEST(cb.peek(4) == 'F', "peek 4 == 'F'")
    c = cb.pop();
    c = cb.pop();
    c = cb.pop();
    c = cb.pop();
    c = cb.pop();
    //cout << "buffer: "; for (int i = 0; i < cb.unread(); ++i) { cout << cb.peek(i); } cout << endl;
    TEST(cb.unread() == 0, "unread == 0 after popping 5 items");
    cout << "Test result: " << (success ? "SUCCESS" : "FAIL") << endl;
    return success ? 0 : 1;
}
#endif

#endif
