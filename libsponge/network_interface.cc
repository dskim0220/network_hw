#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();
    // DUMMY_CODE(dgram, next_hop, next_hop_ip);
    //다음 hop의 MAC을 테이블에서 찾음
    auto it = _arp_table.find(next_hop_ip);
    //만약 테이블에 있으면
    if (it != _arp_table.end()) {
        //다음 hop의 MAC 주소
        EthernetAddress mac_addr = it->second.eth_addr;
        //이더넷 프레임 헤더 설정
        EthernetFrame frame;
        frame.header().src = _ethernet_address;
        frame.header().dst = mac_addr;
        frame.header().type = EthernetHeader::TYPE_IPv4;
        //페이로드 설정    
        frame.payload() = dgram.serialize();
        //프레임 전송
        _frames_out.push(frame);
    } else {//ARP 테이블에 다음 라우터 MAC 없음.
        //다음 hop 전송 대기열에 datagram 추가
        _waiting_dgrams[next_hop_ip].push(dgram);
        //다음 hop 타이머 확인
        auto itr = _arp_timer.find(next_hop_ip);
        //만약 타이머에 기록 안 되어있거나 5000ms 지나면(네트워크 마비 방지)
        if (itr == _arp_timer.end() || itr->second >= 5000) {
            _arp_timer[next_hop_ip] = 0;//타이머 초기화
            //브로드캐스팅 메시지 만듦(ARP request)
            ARPMessage arp_msg;
            arp_msg.opcode = ARPMessage::OPCODE_REQUEST;
            arp_msg.sender_ethernet_address = _ethernet_address;
            arp_msg.sender_ip_address = _ip_address.ipv4_numeric();
            arp_msg.target_ethernet_address = {};//타켓 MAC은 모르니까 비워둠
            arp_msg.target_ip_address = next_hop_ip;

            EthernetFrame frame;

            frame.header().src = _ethernet_address;
            //도착 MAC 주소 FF:FF:FF:FF:FF:FF 설정
            frame.header().dst = ETHERNET_BROADCAST;
            frame.header().type = EthernetHeader::TYPE_ARP;
            //ARP 메시지 이너넷 프레임 payload에 넣음
            frame.payload() = arp_msg.serialize();
            //프레임 내보냄
            _frames_out.push(frame);
        }
    }
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    // DUMMY_CODE(frame);
    //나한테 온 Ethernet frame 아니고, broadcasting도 아니면 그냥 frame 버림
    if (frame.header().dst != _ethernet_address && frame.header().dst != ETHERNET_BROADCAST) {
        return std::nullopt;
    }
    //만약 나한테 온 프레임이면
    if (frame.header().type == EthernetHeader::TYPE_IPv4) {
        InternetDatagram dgram;
        // ParseResult result = dgram.parse(frame.payload());
        //Ethernet payload(IP datagram) 검사해서 에러 없으면 그대로 반환
        if (dgram.parse(frame.payload()) == ParseResult::NoError) {
            return dgram;
        }
    }
    //브로드캐스팅이면
    else if (frame.header().type == EthernetHeader::TYPE_ARP) {
        ARPMessage arp_msg;
        // ParseResult result = arp_msg.parse(frame.payload());
        //만약 payload에 에러 없으면
        if (arp_msg.parse(frame.payload()) == ParseResult::NoError) {
            //ARP Table에 일정시간 동안 기억해둠.(Passive Learning)
            _arp_table[arp_msg.sender_ip_address] = {arp_msg.sender_ethernet_address, 30000};
            //만약 나의 MAC 주소를 묻는 것이면
            if (arp_msg.opcode == ARPMessage::OPCODE_REQUEST &&
                arp_msg.target_ip_address == _ip_address.ipv4_numeric()) {
                ARPMessage reply;//응답 메시지 만듦
                reply.opcode = ARPMessage::OPCODE_REPLY;
                reply.sender_ethernet_address = _ethernet_address;
                reply.sender_ip_address = _ip_address.ipv4_numeric();
                reply.target_ethernet_address = arp_msg.sender_ethernet_address;//내 MAC주소 적어줌
                reply.target_ip_address = arp_msg.sender_ip_address;
                //이더넷 프레임으로 감싸서 전송
                EthernetFrame reply_frame;
                reply_frame.header().src = _ethernet_address;
                reply_frame.header().dst = arp_msg.sender_ethernet_address;
                reply_frame.header().type = EthernetHeader::TYPE_ARP;
                reply_frame.payload() = reply.serialize();
                _frames_out.push(reply_frame);
            }//브로드캐스팅한 곳으로 보내려는 패킷 있으면 전송, ARP message로 MAC 알아냈으니 쌓인 데이터그램 전부 보냄.
            if (_waiting_dgrams.count(arp_msg.sender_ip_address)) {
                while (!_waiting_dgrams[arp_msg.sender_ip_address].empty()) {
                    auto &dgram = _waiting_dgrams[arp_msg.sender_ip_address].front();

                    EthernetFrame out_frame;
                    out_frame.header().src = _ethernet_address;
                    out_frame.header().dst = arp_msg.sender_ethernet_address;
                    out_frame.header().type = EthernetHeader::TYPE_IPv4;
                    out_frame.payload() = dgram.serialize();
                    _frames_out.push(out_frame);
                    //대기큐에서 비움
                    _waiting_dgrams[arp_msg.sender_ip_address].pop();
                }//다 보냈으면 대기열, 타이머에 지움
                _waiting_dgrams.erase(arp_msg.sender_ip_address);
                _arp_timer.erase(arp_msg.sender_ip_address);
            }
        }
    }

    return std::nullopt;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    // DUMMY_CODE(ms_since_last_tick);
    //ARP Table 남은 시간 감소
    for (auto itr = _arp_table.begin(); itr != _arp_table.end();) {
        //만약 남은 시간 이상으로 경과했으면 ARP 테이블에서 지움
        if (itr->second.ttl <= ms_since_last_tick) {
            itr = _arp_table.erase(itr);
        } else {//남은 시간 감소
            itr->second.ttl -= ms_since_last_tick;
            itr++;
        }
    }
    //타이머 시간 증가
    for (auto itr = _arp_timer.begin(); itr != _arp_timer.end(); itr++) {
        itr->second += ms_since_last_tick;
    }
}
