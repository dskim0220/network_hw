#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_recv; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    _time_since_last_recv = 0;

    if (seg.header().rst) {
        _receiver.stream_out().set_error();
        _sender.stream_in().set_error();
        _active = false;
        return;
    }

    _receiver.segment_received(seg);

    if (_receiver.stream_out().input_ended() && !_sender.stream_in().eof()) {
        _linger_after_streams_finish = false;
    }

    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
    }

    if (seg.length_in_sequence_space() > 0) {
        _sender.fill_window();
        if (_sender.segments_out().empty()) {
            _sender.send_empty_segment();
        }
    }

    if (_receiver.ackno().has_value() && (seg.length_in_sequence_space() == 0) &&
        seg.header().seqno == _receiver.ackno().value() - 1) {
        _sender.send_empty_segment();
    }

    push_segments_out();
    /*while(!_sender.segments_out().empty()){
         TCPSegment temp_seg = _sender.segments_out().front();
         _sender.segments_out().pop();

         if(_receiver.ackno().has_value()){
             temp_seg.header().ack = true;
             temp_seg.header().ackno = _receiver.ackno().value();
         }

         temp_seg.header().win = _receiver.window_size();
         _segments_out.push(temp_seg);
    }*/
}

bool TCPConnection::active() const {
    // return {};
    return _active;
}

size_t TCPConnection::write(const string &data) {
    // DUMMY_CODE(data);
    // return {};
    size_t written_len = _sender.stream_in().write(data);
    _sender.fill_window();
    push_segments_out();
    /*while(!_sender.segments_out().empty()){
        TCPSegment temp_seg = _sender.segments_out().front();
        _sender.segments_out().pop();

        if(_receiver.ackno().has_value()){
            temp_seg.header().ack = true;
            temp_seg.header().ackno = _receiver.ackno().value();
        }

        temp_seg.header().win = _receiver.window_size();
        _segments_out.push(temp_seg);
    }*/
    return written_len;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    // DUMMY_CODE(ms_since_last_tick);
    _sender.tick(ms_since_last_tick);
    _time_since_last_recv += ms_since_last_tick;

    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        _active = false;
        _receiver.stream_out().set_error();
        _sender.stream_in().set_error();

        while (!_sender.segments_out().empty()) {
            _sender.segments_out().pop();
        }
        _sender.send_empty_segment();
        TCPSegment target = _sender.segments_out().front();
        _sender.segments_out().pop();

        target.header().rst = true;
        _sender.segments_out().push(target);

        push_segments_out();

        return;
    }

    bool condition1 = _receiver.unassembled_bytes() == 0 && _receiver.stream_out().input_ended();
    bool condition2 =
        (_sender.next_seqno_absolute() == _sender.stream_in().bytes_read() + 2) && (_sender.bytes_in_flight() == 0);
    bool condition3 = (_sender.bytes_in_flight() == 0);

    if (condition1 && condition2 && condition3) {
        if (_linger_after_streams_finish == false) {
            _active = false;
        } else if (_time_since_last_recv >= 10 * _cfg.rt_timeout) {
            _active = false;
        }
    }
    push_segments_out();
    return;
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    push_segments_out();
}

void TCPConnection::connect() {
    _sender.fill_window();
    push_segments_out();
    /*while(!_sender.segments_out().empty()){
        TCPSegment temp_seg = _sender.segments_out().front();
        _sender.segments_out().pop();

        if(_receiver.ackno().has_value()){
            temp_seg.header().ack = true;
            temp_seg.header().ackno = _receiver.ackno().value();
        }

        temp_seg.header().win = _receiver.window_size();
        _segments_out.push(temp_seg);
    }*/
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            _active = false;
            _receiver.stream_out().set_error();
            _sender.stream_in().set_error();

            while (!_sender.segments_out().empty()) {
                _sender.segments_out().pop();
            }

            _sender.send_empty_segment();
            TCPSegment temp_seg = _sender.segments_out().front();
            _sender.segments_out().pop();

            temp_seg.header().rst = true;
            _sender.segments_out().push(temp_seg);

            push_segments_out();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

void TCPConnection::push_segments_out() {
    while (!_sender.segments_out().empty()) {
        TCPSegment temp_seg = _sender.segments_out().front();
        _sender.segments_out().pop();

        if (_receiver.ackno().has_value()) {
            temp_seg.header().ack = true;
            temp_seg.header().ackno = _receiver.ackno().value();
        }

        temp_seg.header().win =
            _receiver.window_size() > static_cast<size_t>(65535) ? static_cast<size_t>(65535) : _receiver.window_size();
        _segments_out.push(temp_seg);
    }
}
