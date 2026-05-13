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
    auto it = _arp_table.find(next_hop_ip);

    if (it != _arp_table.end()) {
        EthernetAddress mac_addr = it->second.eth_addr;

        EthernetFrame frame;
        frame.header().src = _ethernet_address;
        frame.header().dst = mac_addr;
        frame.header().type = EthernetHeader::TYPE_IPv4;

        frame.payload() = dgram.serialize();

        _frames_out.push(frame);
    } else {
        _waiting_dgrams[next_hop_ip].push(dgram);
        auto itr = _arp_timer.find(next_hop_ip);

        if (itr == _arp_timer.end() || itr->second >= 5000) {
            _arp_timer[next_hop_ip] = 0;

            ARPMessage arp_msg;
            arp_msg.opcode = ARPMessage::OPCODE_REQUEST;
            arp_msg.sender_ethernet_address = _ethernet_address;
            arp_msg.sender_ip_address = _ip_address.ipv4_numeric();
            arp_msg.target_ethernet_address = {};
            arp_msg.target_ip_address = next_hop_ip;

            EthernetFrame frame;

            frame.header().src = _ethernet_address;
            frame.header().dst = ETHERNET_BROADCAST;
            frame.header().type = EthernetHeader::TYPE_ARP;

            frame.payload() = arp_msg.serialize();

            _frames_out.push(frame);
        }
    }
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    // DUMMY_CODE(frame);
    if (frame.header().dst != _ethernet_address && frame.header().dst != ETHERNET_BROADCAST) {
        return std::nullopt;
    }

    if (frame.header().type == EthernetHeader::TYPE_IPv4) {
        InternetDatagram dgram;
        // ParseResult result = dgram.parse(frame.payload());
        if (dgram.parse(frame.payload()) == ParseResult::NoError) {
            return dgram;
        }
    }

    else if (frame.header().type == EthernetHeader::TYPE_ARP) {
        ARPMessage arp_msg;
        // ParseResult result = arp_msg.parse(frame.payload());
        if (arp_msg.parse(frame.payload()) == ParseResult::NoError) {
            _arp_table[arp_msg.sender_ip_address] = {arp_msg.sender_ethernet_address, 30000};

            if (arp_msg.opcode == ARPMessage::OPCODE_REQUEST &&
                arp_msg.target_ip_address == _ip_address.ipv4_numeric()) {
                ARPMessage reply;
                reply.opcode = ARPMessage::OPCODE_REPLY;
                reply.sender_ethernet_address = _ethernet_address;
                reply.sender_ip_address = _ip_address.ipv4_numeric();
                reply.target_ethernet_address = arp_msg.sender_ethernet_address;
                reply.target_ip_address = arp_msg.sender_ip_address;

                EthernetFrame reply_frame;
                reply_frame.header().src = _ethernet_address;
                reply_frame.header().dst = arp_msg.sender_ethernet_address;
                reply_frame.header().type = EthernetHeader::TYPE_ARP;
                reply_frame.payload() = reply.serialize();
                _frames_out.push(reply_frame);
            }
            if (_waiting_dgrams.count(arp_msg.sender_ip_address)) {
                while (!_waiting_dgrams[arp_msg.sender_ip_address].empty()) {
                    auto &dgram = _waiting_dgrams[arp_msg.sender_ip_address].front();

                    EthernetFrame out_frame;
                    out_frame.header().src = _ethernet_address;
                    out_frame.header().dst = arp_msg.sender_ethernet_address;
                    out_frame.header().type = EthernetHeader::TYPE_IPv4;
                    out_frame.payload() = dgram.serialize();
                    _frames_out.push(out_frame);

                    _waiting_dgrams[arp_msg.sender_ip_address].pop();
                }
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
    for (auto itr = _arp_table.begin(); itr != _arp_table.end();) {
        if (itr->second.ttl <= ms_since_last_tick) {
            itr = _arp_table.erase(itr);
        } else {
            itr->second.ttl -= ms_since_last_tick;
            itr++;
        }
    }

    for (auto itr = _arp_timer.begin(); itr != _arp_timer.end(); itr++) {
        itr->second += ms_since_last_tick;
    }
}
