/*
 *      Author: jm
 */

#ifndef STRING_PRINT_H_
#define STRING_PRINT_H_

#include "Arduino.h"

class StringPrint : public Print {
 private:
  String buffer_;
 public:
  virtual ~StringPrint() {
  }

  virtual size_t write(uint8_t c) override {
    buffer_.concat((char) c);
    return 1;
  }

  const String& ToString() {
    return buffer_;
  }

  const char* c_str() {
    return buffer_.c_str();
  }
};

class HexPrint : public Print {
 private:
  Print* out_;
 public:
  HexPrint(Print* out) {
    out_ = out;
  }

  virtual ~HexPrint() {
  }

  virtual size_t write(uint8_t c) override {
    static char digits[] = "0123456789abcdef";
    out_->print(digits[(c >> 4) & 0x0f]);
    out_->print(digits[c & 0x0f]);
    out_->print(" ");
    return 1;
  }
};

#endif /* STRING_PRINT_H_ */
