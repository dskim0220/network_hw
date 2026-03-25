#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    // DUMMY_CODE(seg);
    TCPHeader head = seg.header();
    if (!_set_isn) {
        if (!head.syn)
            return;
        _isn = head.seqno;
        _set_isn = true;
    }

    uint64_t chkpoint = _reassembler.stream_out().bytes_written() + 1;
    uint64_t stream_idx = unwrap(head.seqno, _isn, chkpoint) + (head.syn) - 1;

    _reassembler.push_substring(string(seg.payload().str()), stream_idx, head.fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    // return {};
    if (!_set_isn)
        return nullopt;

    uint64_t absolute = _reassembler.stream_out().bytes_written() + 1 + (_reassembler.stream_out().input_ended());
    return wrap(absolute, _isn);
}

size_t TCPReceiver::window_size() const {
    // return {};
    return _reassembler.stream_out().remaining_capacity();
}
