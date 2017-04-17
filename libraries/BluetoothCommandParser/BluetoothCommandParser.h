#ifndef BLUETOOTHCOMMANDPARSER_H
#define BLUETOOTHCOMMANDPARSER_H

#define BLUETOOTH_COMMAND_PARSER_BUF_LENGTH 20

#include <CircularBuffer.h>

class BluetoothCommandParser;

struct BluetoothKeyCommand {
    char key;
    char onoff;
    char extra;
};

struct BluetoothGyroData {
    float x;
    float y;
    float z;
};

#ifdef TESTCASE_BLUETOOTHCOMMANDPARSER
#include <iostream>
using namespace std;
#endif

class BluetoothCommandParser
{
public:
    enum ValidationResult { Valid, Invalid, Wait };
        
    BluetoothCommandParser()
        : _buf()
    {
    }

    void feed(char c) {
        _buf.push(c);
        popUntilWaitOrValid();
    }

    ValidationResult parseKeyCommand() {
        if (_buf.unread() >= 2 && _buf.peek(1) != 'B') {
            return Invalid;
        }
        if (_buf.unread() < 5) {
            return Wait;
        }
#ifdef TESTCASE_BLUETOOTHCOMMANDPARSER
        cout << "buffer: "; for (int i = 0; i < _buf.unread(); ++i) { cout << _buf.peek(i); } cout << endl;
#endif
        _keyCommand.key = _buf.peek(2) - '0';
        _keyCommand.onoff = _buf.peek(3) - '0';
        _keyCommand.extra = _buf.peek(4);
        bool isValid = (_keyCommand.key >= 1 && _keyCommand.key <= 8) &&
            (_keyCommand.onoff >= 0 && _keyCommand.onoff <= 1);
        return isValid ? Valid : Invalid;
    }

    ValidationResult parseGyroData() {
        if (_buf.unread() >= 2 && _buf.peek(1) != 'G') {
            return Invalid;
        }
        if (_buf.unread() < 2 + 3 * sizeof(float)) {
            return Wait;
        }
        char gyroData[3 * sizeof(float)];
        for (int i = 0; i < 3 * sizeof(float); i++) {
            gyroData[i] = _buf.peek(2 + i);
        }
        _gyroData.x = *reinterpret_cast<float *>(gyroData + 0 * sizeof(float));
        _gyroData.y = *reinterpret_cast<float *>(gyroData + 1 * sizeof(float));
        _gyroData.z = *reinterpret_cast<float *>(gyroData + 2 * sizeof(float));
        return Valid;
    }

    void popUntilWaitOrValid() {
        while (_buf.unread() > 0) {
            if (_buf.peek(0) == '!') {
                ValidationResult result;
                result = parseKeyCommand();
                if (result == Wait || result == Valid) {
                    break;
                }
                result = parseGyroData();
                if (result == Wait || result == Valid) {
                    break;
                }
            }
            // If we arrive here, there is no valid command and there is no need to wait for it.
            // Pop first character in the buffer and try again.
            _buf.pop();
        }
    }

    void reset() {
        _buf.popn(_buf.unread());
    }

#ifdef TESTCASE_BLUETOOTHCOMMANDPARSER
    void printBuffer() {
        cout << "buffer (count = " << dec << static_cast<int>(_buf.unread()) << "): ";
        for (int i = 0; i < _buf.unread(); ++i) {
            cout << hex << static_cast<int>(_buf.peek(i)) << " ";
        }
        cout << endl;
    }
#endif

    const BluetoothKeyCommand& keyCommand() const { return _keyCommand; }
    const BluetoothGyroData& gyroData() const { return _gyroData; }

private:
    CircularBuffer<char, BLUETOOTH_COMMAND_PARSER_BUF_LENGTH> _buf;
    BluetoothKeyCommand _keyCommand;
    BluetoothGyroData _gyroData;
};

#ifdef TESTCASE_BLUETOOTHCOMMANDPARSER
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
    BluetoothCommandParser btcmdparser;
    cout << "Testing !B!B11:" << endl;
    btcmdparser.feed('!');
    btcmdparser.feed('B');
    btcmdparser.feed('!');
    btcmdparser.feed('B');
    btcmdparser.feed('1');
    btcmdparser.feed('1');
    btcmdparser.feed(':');
    if (btcmdparser.parseKeyCommand() != BluetoothCommandParser::Valid) {
        cout << "valid: expected valid, but got something else" << endl;
        return 1;
    }
    cout << "key = " << hex << static_cast<int>(btcmdparser.keyCommand().key)
        << ", onoff = " << hex << static_cast<int>(btcmdparser.keyCommand().onoff)
        << ", extra = " << btcmdparser.keyCommand().extra << endl;
    btcmdparser.reset();
    cout << endl << "Testing !B309" << endl;
    btcmdparser.feed('!');
    btcmdparser.feed('B');
    if (btcmdparser.parseKeyCommand() != BluetoothCommandParser::Wait) {
        cout << "valid: expected wait, but failed" << endl;
        return 1;
    }
    btcmdparser.feed('3');
    btcmdparser.feed('0');
    btcmdparser.feed('9');
    if (btcmdparser.parseKeyCommand() == BluetoothCommandParser::Valid) {
        cout << "key = " << hex << static_cast<int>(btcmdparser.keyCommand().key)
            << ", onoff = " << hex << static_cast<int>(btcmdparser.keyCommand().onoff)
            << ", extra = " << btcmdparser.keyCommand().extra << endl;
    } else {
        cout << "valid: false" << endl;
        return 1;
    }
    btcmdparser.reset();
    cout << endl << "Testing !G<x><y><z>" << endl;
    btcmdparser.feed('!');
    btcmdparser.feed('G');
    float x = 10.0;
    float y = 20.0;
    float z = -0.50;
    for (int i = 0; i < sizeof(float); ++i) { btcmdparser.feed(*(reinterpret_cast<char *>(&x) + i)); }
    for (int i = 0; i < sizeof(float); ++i) { btcmdparser.feed(*(reinterpret_cast<char *>(&y) + i)); }
    for (int i = 0; i < sizeof(float); ++i) { btcmdparser.feed(*(reinterpret_cast<char *>(&z) + i)); }
    if (btcmdparser.parseGyroData() == BluetoothCommandParser::Valid) {
        cout << "x = " << btcmdparser.gyroData().x
            << ", y = " << btcmdparser.gyroData().y
            << ", z = " << btcmdparser.gyroData().z << endl;
    } else {
        cout << "valid: false" << endl;
        return 1;
    }
}
#endif

#endif
