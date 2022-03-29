#include "packet.h"

const char* DeviceName(uint8_t id) {
  switch ((DeviceID)id) {
    case DEVICE_CONTROL_CENTER:
      return "Control Center";
    case DEVICE_BROADCAST:
      return "Broadcast";
    case DEVICE_PUMP1:
      return "Pump1";
    case DEVICE_PUMP2:
      return "Pump2";
    case DEVICE_CHLORINATOR:
      return "Chlorinator";
    case DEVICE_WIRED_REMOTE:
      return "Wired Remote";
    case DEVICE_WIRELESS_REMOTE:
      return "Wireless Remote";
    case DEVICE_REMOTE:
      return "Remote";
    case DEVICE_90:
      return "Device 0x90";
  }
  return "Unknown";
}

void Packet::Reset() {
  valid_ = false;
  state_ = preamble_state;
  pos_ = 0;
  buffer_pos_ = 0;
  version_ = 0;
  target_ = 0;
  source_ = 0;
  cid_ = 0;
  command_ = &unknown;
  data_len_ = 0;
  checksum_ = 0;
  memset(buffer_, 0, sizeof(buffer_));
  memset(checksum_data_, 0, sizeof(checksum_data_));
}

bool Packet::Make(uint8_t cid, const char* data) {
  Reset();
  PushHex("ff 00 ff a5 25 10 20");
  Push(cid);
  PushHex(data);
  checksum_data_[0] = (checksum_ / 256) & 0xFF;
  checksum_data_[1] = checksum_ & 0xFF;
  return IsValid();
}

// Read a Frame formatted as an hex string, e.g. "ff 10 02 50 00 00 62 10 03"
bool Packet::ReadFrom(const char *s) {
  Reset();
  return PushHex(s);
}

bool Packet::PushHex(const char *f) {
  for (int i = 0; i < strlen(f); i++) {
    if (f[i] != ' ' && f[i] != '\t') {
      char hi = f[i++];
      char lo = f[i];
      uint8_t b = htoi(hi) * 16 + htoi(lo);
      if (Push(b)) {
        return true;
      }
    }
  }
  return false;
}

bool Packet::Push(uint8_t c) {
  bool complete = false;
  // First, make sure we don't overrun the internal buffer. Drop the buffer and continue if we do.
  if (buffer_pos_ > sizeof(buffer_)) {
    Serial.println("   ***   BUFFER OVERRUN   ***");
    // Can't make this available to the caller; just debug print for now.
    for (unsigned int i = 0; i < sizeof(buffer_); i++) {
      uint8_t c = buffer_[i];
      PrintHex(Serial, c);
    }
    Reset();
  }
  const uint8_t p = buffer_pos_;  // last char in buffer
  buffer_[buffer_pos_++] = c;
  switch (state_) {
    case preamble_state:
      // There's 2 ways we can detect a packet start:
      // 1 - pentair preamble: 0xFF 0x00 0xFF 0xA5
      // 2 - intellichlor preamble: 0x10 0x02
      // Check the we've read at least enough characters for the preamble and
      // see if the 4 last we have match the expected preamble.
      if (p >= 3 && buffer_[p] == 0xA5 && buffer_[p - 1] == 0xFF
          && buffer_[p - 2] == 0x00 && buffer_[p - 3] == 0xFF) {
        // 1 - pentair preamble: 0xFF 0x00 0xFF 0xA5
        nextState(version_state, 0xA5);  // This last preamble byte is part of the checksum
      } else if (p >= 1 && buffer_[p] == 0x02 && buffer_[p - 1] == 0x10) {
        // 2 - intellichlor preamble: 0x10 0x02
        nextState(intellichlor_state, 0x12);  // Start sequence "10 02" is part of the checksum.
        data_ = buffer_ + buffer_pos_;      // Data starts on the next byte.
      }
      // Stay in this state until we find a valid header.
      break;
    case version_state:
      version_ = c;
      nextState(destination_state, c);
      break;
    case destination_state:
      target_ = c;
      nextState(source_state, c);
      break;
    case source_state:
      source_ = c;
      nextState(command_state, c);
      break;
    case command_state:
      cid_ = c;
      nextState(length_state, c);
      break;
    case length_state:
      data_len_ = c;
      if (data_len_ > 0) {
        // keep a pointer to the beginning of the actual data inside the buffer we already have.
        data_ = buffer_ + buffer_pos_;
        nextState(data_state, c);
      } else {
        nextState(checksum_state, c);
      }
      break;
    case data_state:
      pos_++;
      checksum_ += (uint16_t) c;
      if (pos_ >= data_len_) {
        // we've read all the data bytes
        nextState(checksum_state, 0);
      }
      break;
    case checksum_state:
      checksum_data_[pos_++] = c;
      if (pos_ >= 2) {
        complete = true;
        nextState(preamble_state, 0);
        // verify checksum
        uint16_t chk = toUint16(checksum_data_[0], checksum_data_[1]);
        if (checksum_ == chk) {
          valid_ = true;
          ReadCommand();
        } else {
          valid_ = false;
        }
      }
      break;
    case intellichlor_state:
      data_len_++;
      const int p = buffer_pos_ - 1;  // last char in buffer
      if (p >= 1 && buffer_[p] == 0x03 && buffer_[p - 1] == 0x10) {
        // Found the end of the packet.
        complete = true;
        nextState(preamble_state, 0);
        // Data ended 3 bytes ago (1 bytes of checksum + 2 bytes of terminator).
        data_len_ -= 3;
        checksum_data_[0] = 0x00;
        checksum_data_[1] = buffer_[p - 2];
        uint8_t chk = checksum_data_[1];
        // the checksum includes the packet header "10 02" to the end of the data.
        uint8_t checksum = 0x12;
        for (int i = 0; i < data_len_; i++) {
          checksum += data_[i];
        }
        if (checksum == chk) {
          valid_ = true;
          // Make the packet data look like the other packets by setting command/target/source.
          cid_ = COMMAND_CHLORINATOR;
          if (data_[0] == 0x50) {
            source_ = DEVICE_CONTROL_CENTER;
            target_ = DEVICE_CHLORINATOR;
          } else {
            source_ = DEVICE_CHLORINATOR;
            target_ = DEVICE_CONTROL_CENTER;
          }
          ReadCommand();
        } else {
          valid_ = false;
        }
        checksum_ = checksum;
      }
      break;
  }
  return complete;
}

void Packet::ReadCommand() {
  switch (cid_) {
    case COMMAND_SYSTEM_STATUS:
      command_ = &system_status;
      break;
    case COMMAND_PUMP_STATUS:
      if (DEVICE_CONTROL_CENTER == source_) {
        command_ = &pump_status_request;
      } else {
        command_ = &pump_status_response;
      }
      break;
    case COMMAND_PUMP_CONTROL:
      if (DEVICE_CONTROL_CENTER == source_) {
        command_ = &pump_control_request;
      } else {
        command_ = &pump_control_response;
      }
      break;
    case COMMAND_CHLORINATOR:
      if (data_[0] == 0x50) {
        command_ = &chlorinator_request;
      } else {
        command_ = &chlorinator_response;
      }
      break;
    case COMMAND_SCHEDULE_CONFIG:
      command_ = &schedule_config;
      break;
    case COMMAND_TIME_SETTINGS:
      command_ = &time_settings;
      break;
    case COMMAND_HEAT_STATUS:
      command_ = &heat_status;
      break;
  }

  command_->Read(data_);

  if (COMMAND_SYSTEM_STATUS == cid_) {
    // initialize & maintain the clock from the time received in the command center status sent every 2 seconds.
    if (clock_.minute < system_status.minutes_) {
      clock_.initialize(system_status.hour_, system_status.minutes_);
    }
  }
  if (clock_.initialized) {
    clock_.time();
  }
}

uint8_t htoi(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return 10 + c - 'a';
  }
  if (c >= 'A' && c <= 'F') {
    return 10 + c - 'A';
  }
  return 0;  // not reached
}

uint16_t toUint16(uint8_t hi, uint8_t lo) {
  return (uint16_t) hi * 256 + (uint16_t) lo;
}

#define DEFAULT_PAD 10

void PrintLabel(Print &p, const char *l) {
  PrintLabel(p, l, DEFAULT_PAD);
}

void PrintLabel(Print &p, const char *l, int pad) {
  p.print(l);
  p.print(":");
  for (int i = strlen(l) + 1; i < pad; i++) {
    p.print(" ");
  }
}

void PrintHex(Print &p, uint8_t c) {
  static char digits[] = "0123456789abcdef";
  p.print(digits[(c >> 4) & 0x0f]);
  p.print(digits[c & 0x0f]);
}

void PrintTime(Print &p, uint8_t h, uint8_t m) {
  p.print(h);
  p.print((m < 10) ? ":0" : ":");
  p.print(m);
}

void printPad(Print &p, int n) {
  if (n < 10)
    p.print("0");
  p.print(n);
}

void printPad(Print &p, long n) {
  if (n < 100)
    p.print("0");
  if (n < 10)
    p.print("0");
  p.print(n);
}

void PrintDate(Print &p, byte day, byte month, byte year) {
  printPad(p, month);
  p.print("/");
  printPad(p, day);
  p.print("/20");
  printPad(p, year);
}

void PrintClock(Print &p, Clock &c) {
  printPad(p, c.hour);
  p.print(":");
  printPad(p, c.minute);
  p.print(":");
  printPad(p, c.second);
  p.print(".");
  printPad(p, c.millisecond);
}

void Packet::PrintHeader(Print &p) {
  PrintLabel(p, "millis");
  Serial.println(millis());

  PrintLabel(p, "version");
  PrintHex(p, version_);
  p.println();

  PrintLabel(p, "source");
  PrintHex(p, source_);
  p.print(" (");
  p.print(DeviceName(source_));
  p.print(")");
  p.println();

  PrintLabel(p, "target");
  PrintHex(p, target_);
  p.print(" (");
  p.print(DeviceName(target_));
  p.print(")");
  p.println();

  PrintLabel(p, "command");
  PrintHex(p, cid_);
  p.print(" (");
  p.print(command_->Name());
  p.print(")");
  p.println();
}

void Packet::PrintTo(Print &p) {
  PrintHeader(p);
  command_->PrintTo(p);
  p.print("- Packet Data [");
  p.print(data_len_);
  p.println("]");
  if (data_len_ > 0) {
    p.print("\t\t");
    for (int i = 0; i < data_len_; i++) {
      p.print("\t");
      p.print(i);
    }
    p.println();

    p.print("data(0x):\t");
    for (int i = 0; i < data_len_; i++) {
      PrintHex(p, data_[i]);
      p.print("\t");
    }
    p.println();

    p.print("data(10):");
    for (int i = 0; i < data_len_; i++) {
      p.print("\t");
      p.print(data_[i]);
    }
    p.println();
  }
// print single line data header with clock,src,tgt,command, so data dumps can be grep-ed for specific commands.
  if (clock_.initialized) {
    PrintClock(p, clock_);
    p.print(" ");
    PrintHex(p, source_);
    PrintHex(p, target_);
    PrintHex(p, cid_);
    p.print(":: ");
    for (int i = 0; i < data_len_; i++) {
      PrintHex(p, data_[i]);
      p.print(" ");
    }
    p.println();
  }

  p.print("checkdat:");
  for (int i = 0; i < 2; i++) {
    p.print("\t");
    PrintHex(p, checksum_data_[i]);
  }
  p.println();
  p.print("checksum:");
  uint8_t hi = (checksum_ / 256) & 0xFF;
  uint8_t lo = checksum_ & 0xFF;
  p.print("\t");
  PrintHex(p, hi);
  p.print("\t");
  PrintHex(p, lo);
  p.println();

  p.print("bufferPos:\t");
  p.println(buffer_pos_);

  p.print("buffer:\t");
  for (int i = 0; i < buffer_pos_; i++) {
    p.print("\t");
    PrintHex(p, buffer_[i]);
  }
  p.println();
  p.println("---------------------------------");
}

void Packet::WriteTo(Print &p) {
  if (version_ == 0) {
    // chlorinator protocol
    const uint8_t prefix[] = {0x10, 0x02};
    const uint8_t suffix[] = {0x10, 0x03};
    p.write(prefix, sizeof(prefix));
    for(int i = 0; i < data_len_; i++) {
      p.write(data_[i]);
    }
    p.write(checksum_data_[1]);
    p.write(suffix, sizeof(suffix));
  } else {
    // intellitouch protocol
    const uint8_t prefix[] = {0xff, 0x00, 0xff, 0xa5};
    p.write(prefix, sizeof(prefix));
    p.write((version_));
    p.write((target_));
    p.write((source_));
    p.write((cid_));
    p.write((data_len_));
    for(int i = 0; i < data_len_; i++) {
      p.write(data_[i]);
    }
    p.write(checksum_data_[0]);
    p.write(checksum_data_[1]);
  }
}
