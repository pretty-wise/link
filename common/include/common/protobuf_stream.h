/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include <istream>
#include <streambuf>

struct Protobuf_streambuf : std::streambuf {
  Protobuf_streambuf(void *buffer, streamsize nbytes) {
    this->setg(static_cast<char *>(buffer), static_cast<char *>(buffer),
               static_cast<char *>(buffer) + nbytes);
  }
};

struct ProtobufStream : public Protobuf_streambuf, std::istream {
  ProtobufStream(void *buffer, streamsize nbytes)
      : Protobuf_streambuf(buffer, nbytes), std::istream(this) {}
};
