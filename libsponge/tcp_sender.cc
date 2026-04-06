#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _current_rto(0)
    , _elapsed_time(0)
    , _consecutive_retransmissions(0)
    , _last_window_size(1)
    , _bytes_in_flight(0)
    , _last_acked_seqno(0)
    , _is_timer_running(false)
    , _syn_sent(false)
    ,_fin_sent(false){}

uint64_t TCPSender::bytes_in_flight() const { 
    return _bytes_in_flight; 
}

void TCPSender::fill_window() {
    size_t window_size = _last_window_size ? _last_window_size : 1;

    while(window_size > _bytes_in_flight){
        TCPSegment seg;

        if(!_syn_sent){
            seg.header().syn=true;
            _syn_sent=true;
        }

        size_t available = window_size - _bytes_in_flight;
        size_t length = min(TCPConfig::MAX_PAYLOAD_SIZE, available - seg.length_in_sequence_space());

        std::string content = stream_in().read(length);

        seg.payload() = Buffer(std::move(content));

        if(!_fin_sent && stream_in().eof() && available > seg.length_in_sequence_space()){
            seg.header().fin =true;
            _fin_sent = true;
        }

        if(seg.length_in_sequence_space()==0){
            break;
        }

        seg.header().seqno = wrap(_next_seqno, _isn);

        _segments_out.push(seg);
        _outstanding.push(seg);

        _next_seqno += seg.length_in_sequence_space();
        _bytes_in_flight += seg.length_in_sequence_space();

        if(!_is_timer_running){
            _is_timer_running = true;
            _elapsed_time = 0;
        }

    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { 
    //DUMMY_CODE(ackno, window_size);
    uint64_t abs_ackno = unwrap(ackno, _isn, _next_seqno);

    if(abs_ackno > _next_seqno){
        return;
    }

    _last_window_size = window_size;

    if(abs_ackno > _last_acked_seqno){
        _last_acked_seqno = abs_ackno;

        _current_rto = _initial_retransmission_timeout;
        _consecutive_retransmissions = 0;
        _elapsed_time = 0;
    }

    while(!_outstanding.empty()){
        TCPSegment &temp = _outstanding.front();
        uint64_t seg_abs_seqno = unwrap(temp.header().seqno, _isn, _next_seqno);

        if(seg_abs_seqno + temp.length_in_sequence_space() <= abs_ackno){
            _bytes_in_flight -= temp.length_in_sequence_space();
            _outstanding.pop();
        }
        else{
            break;
        }
    }

    if(_outstanding.empty()){
        _is_timer_running = false;
    }

    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
    //DUMMY_CODE(ms_since_last_tick);
    if(!_is_timer_running) return;
    
    _elapsed_time += ms_since_last_tick;

    if(_elapsed_time >= _current_rto){
        if(!_outstanding.empty()){
            _segments_out.push(_outstanding.front());
        }

        if(_last_window_size > 0){
            _consecutive_retransmissions++;
            _current_rto *= 2;
        }

        _elapsed_time = 0;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
}
