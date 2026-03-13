#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _unassembled(), _next_idx(0), _eof_idx(0), _unassembled_bytes(0), _eof_found(false), _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    //DUMMY_CODE(data, index, eof);
    size_t _first_unread = _output.bytes_read();
    size_t _first_unacceptable = _first_unread + _capacity;
    
    if(eof){
        _eof_found = true;
        _eof_idx = index+data.size();
    }

    if(index>=_first_unacceptable || (index+data.size()) < _next_idx){
        if(_eof_found && (_next_idx >=_eof_idx)) _output.end_input();
        return;
    }

    size_t len = data.size();
    size_t start, end;
    start = index>=_next_idx ? index : _next_idx;
    end = ((index+len-1) > _first_unacceptable-1) ? _first_unacceptable-1: index+len-1;
    string parsed_string = data.substr(start-index,end-start+1);

    //_unassembled.insert({start,parsed_string});
    for(auto itr=_unassembled.begin();itr != _unassembled.end(); ){
        //string temp;
        size_t seg_start = itr->first;
        size_t seg_end = seg_start + (itr->second).size() - 1;

        if(!(end<seg_start || start>seg_end )){
            if(seg_start < start){
                parsed_string = (itr->second).substr(0,start-seg_start) + parsed_string;
                start = seg_start;
            }

            if(seg_end > end){
                parsed_string = parsed_string + (itr->second).substr(end+1-seg_start,seg_end-end);
                end = seg_end;
            }

            _unassembled_bytes -= (itr->second).size();
            itr = _unassembled.erase(itr);
        }
        else{
            itr++;
        }
        
    }
    _unassembled[start] = parsed_string;
    _unassembled_bytes += parsed_string.size();

    while(!_unassembled.empty()){
        auto itr = _unassembled.begin();
        size_t seg_start = itr->first;
        string &seg_data = itr->second;

        if(seg_start > _next_idx){
            break;
        }

        size_t seg_end = seg_start + seg_data.size() - 1;
        if(seg_end >=_next_idx){
            string temp = seg_data.substr(_next_idx - seg_start);
            _output.write(temp);

            _next_idx = seg_end + 1;
        }

        _unassembled_bytes -= seg_data.size();
        _unassembled.erase(itr);
    }
    if(_eof_found && _next_idx >= _eof_idx){
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { 
    //return {};
    return _unassembled_bytes; 
}

bool StreamReassembler::empty() const { 
    //return {};
    return _unassembled.empty();
}
