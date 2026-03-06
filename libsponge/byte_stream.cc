#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : _buffer(), _capacity(capacity), _bytes_written(0), _bytes_read(0), _input_ended(false), _error(false) {
    // DUMMY_CODE(capacity);
}

size_t ByteStream::write(const string &data) {
    // DUMMY_CODE(data);
    // return {};
    size_t remain_capacity = remaining_capacity();
    size_t write_len = data.size() > remain_capacity ? remain_capacity : data.size();
    for (size_t i = 0; i < write_len; i++) {
        _buffer.push_back(data[i]);
    }
    _bytes_written += write_len;
    return write_len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    // DUMMY_CODE(len);
    // return {};
    size_t peek_len = _buffer.size() > len ? len : _buffer.size();
    return {_buffer.begin(), _buffer.begin() + peek_len};
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    // DUMMY_CODE(len);
    size_t pop_len = _buffer.size() > len ? len : _buffer.size();
    for (size_t i = 0; i < pop_len; i++) {
        _buffer.pop_front();
    }
    _bytes_read += pop_len;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    // DUMMY_CODE(len);
    // return {};
    size_t read_len = _buffer.size() > len ? len : _buffer.size();
    string str = peek_output(read_len);
    pop_output(read_len);
    return str;
}

void ByteStream::end_input() { _input_ended = true; }

bool ByteStream::input_ended() const {
    // return {};
    return _input_ended;
}

size_t ByteStream::buffer_size() const {
    // return {};
    return _buffer.size();
}

bool ByteStream::buffer_empty() const {
    // return {};
    return _buffer.empty();
}

bool ByteStream::eof() const {
    // return false;
    return _input_ended && _buffer.empty();
}

size_t ByteStream::bytes_written() const {
    // return {};
    return _bytes_written;
}

size_t ByteStream::bytes_read() const {
    // return {};
    return _bytes_read;
}

size_t ByteStream::remaining_capacity() const {
    // return {};
    return _capacity - _buffer.size();
}
