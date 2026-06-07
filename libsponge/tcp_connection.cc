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
    _time_since_last_recv = 0;//받고 나서 시간 초기화

    //RST 플래그면 inbound, outbound 에러 설정하고 연결 종료
    if (seg.header().rst) {
        _receiver.stream_out().set_error();
        _sender.stream_in().set_error();
        _active = false;
        return;
    }

    _receiver.segment_received(seg);//RST 아니니까 receiver로 전달

    //상대방이 먼저 FIN을 보내어 수신 스트림 끝났고(input_ended()), sender는 아직 보낼 데이터가 남아있으면(!EOF)
    //Passive Close 상황이므로 ACK 유실될 걱정 안 해도 되므로 _linger_after_streams_finish를 false로 끔.
    if (_receiver.stream_out().input_ended() && !_sender.stream_in().eof()) {
        _linger_after_streams_finish = false;
    }
    //세그먼트 ACK플래그 설정되어 있으면 ACKNO, window_size sender한테 전달
    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
    }
    //fill_window()로 보낼 데이터가 있으면 데이터를 담아서 보내고, 
    //만약 데이터가 없어서 큐가 비어있으면 빈 세그먼트(ACK역할)라도 보냄
    if (seg.length_in_sequence_space() > 0) {
        _sender.fill_window();
        if (_sender.segments_out().empty()) {
            _sender.send_empty_segment();
        }
    }
    //Keep-alive 패킷 응답
    //상대방이 내 컴퓨터가 살아있는지 확인하고 싶을 때 과거의 시퀀스 넘버(ackno-1)를 가진 빈 세그먼트를 찔러봄.
    //이때 무시하지 않고 빈 세그먼트를 보내서 응답
    if (_receiver.ackno().has_value() && (seg.length_in_sequence_space() == 0) &&
        seg.header().seqno == _receiver.ackno().value() - 1) {
        _sender.send_empty_segment();
    }
    //세그먼트 내보냄
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
    size_t written_len = _sender.stream_in().write(data);//sender가 보낼 문자열 길이
    _sender.fill_window();//데이터 담아서
    push_segments_out();//보냄
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
    return written_len;//보낸 문자열 길이
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    // DUMMY_CODE(ms_since_last_tick);

    //sender에게 시간 경과 알림
    _sender.tick(ms_since_last_tick);
    //시간 경과 업데이트
    _time_since_last_recv += ms_since_last_tick;

    //재전송 횟수가 최대 횟수를 초과하면 연결을 종료해야 함.
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        _active = false;//연결 종료
        //inbound, outbound 연결 종료
        _receiver.stream_out().set_error();
        _sender.stream_in().set_error();
        
        //만약 전송 대기 큐가 비어있지 않으면 큐에 남아있는 패킷 전부 터트림
        while (!_sender.segments_out().empty()) {
            _sender.segments_out().pop();
        }
        //RST 설정된 빈 세그먼트 만듦
        _sender.send_empty_segment();
        TCPSegment target = _sender.segments_out().front();
        _sender.segments_out().pop();
        target.header().rst = true;
        //빈 세그먼트 큐에 넣음
        _sender.segments_out().push(target);
        //세그먼트 보냄
        push_segments_out();
        //세그먼트 보내고 종료
        return;
    }
    //조건1: 상대방 FIN 수신하여 inbound 스트림이 완전히 조립되어 끝남
    bool condition1 = _receiver.unassembled_bytes() == 0 && _receiver.stream_out().input_ended();
    //조건2&3: 내 Outbound 끝났고(FIN 전송 완료), 보낸 모든 패킷(FIN 포함)이 상대방으로부터 ACK를 받음.
    //bytes_read()+2는 SYN, FIN 각각 1바이트씩 더하여 전체 시퀀스 길이 구한 것
    bool condition2 =
        (_sender.next_seqno_absolute() == _sender.stream_in().bytes_read() + 2) && (_sender.bytes_in_flight() == 0);
    bool condition3 = (_sender.bytes_in_flight() == 0);

    //세 조건 전부 만족하는 경우에
    //만약 상대가 먼저 끊자고 하면(Passive Close) 바로 연결 종료
    //내가 먼저 끊자고 하면(Active Close) 내 마지막 ACK가 유실됐을까봐 충분히 기다림. _cfg.rt_timeout*10 동안 기다렸다가 종료함.
    if (condition1 && condition2 && condition3) {
        if (_linger_after_streams_finish == false) {
            _active = false;
        } else if (_time_since_last_recv >= 10 * _cfg.rt_timeout) {
            _active = false;
        }
    }
    //세그먼트 내보냄.
    push_segments_out();
    return;
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();//입력 스트림 종료
    _sender.fill_window();//FIN 패킷 큐에 넣기
    push_segments_out();//내보냄
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
            //연결 종료, inbound/outbound 에러 설정
            _active = false;
            _receiver.stream_out().set_error();
            _sender.stream_in().set_error();
            //큐 비우기
            while (!_sender.segments_out().empty()) {
                _sender.segments_out().pop();
            }
            //RST 플래그 설정 패킷 만들고, 큐에 넣어서 보냄
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
//큐에 들어있는 패킷 내보내는 메서드
void TCPConnection::push_segments_out() {
    //큐가 비어있지 않으면 
    while (!_sender.segments_out().empty()) {
        //하나씩 큐에 든 패킷을 꺼냄
        TCPSegment temp_seg = _sender.segments_out().front();
        _sender.segments_out().pop();
        //ACK, ACKNO 설정
        if (_receiver.ackno().has_value()) {
            temp_seg.header().ack = true;
            temp_seg.header().ackno = _receiver.ackno().value();
        }
        //헤더의 window_size 설정
        //최대 window_size 설정 로직
        temp_seg.header().win =
            _receiver.window_size() > static_cast<size_t>(65535) ? static_cast<size_t>(65535) : _receiver.window_size();
        _segments_out.push(temp_seg);
    }
}
