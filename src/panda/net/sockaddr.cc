#include "sockaddr.h"
#include <net/if.h>
#include "pton.impl"
#include <system_error>

#define _NULL_TERMINATE(what, to)                \
    char to[what.length()+1];                    \
    std::memcpy(to, what.data(), what.length()); \
    to[what.length()] = 0;

namespace panda { namespace net {

using std::errc;
using std::error_code;
using std::system_error;
using std::make_error_code;

SockAddr::SockAddr (const sockaddr& _sa) {
    switch (_sa.sa_family) {
        case AF_UNSPEC : sa.sa_family = AF_UNSPEC; break;
        case AF_INET   : sa4 = (const sockaddr_in&)_sa; break;
        case AF_INET6  : sa6 = (const sockaddr_in6&)_sa; break;
        #ifndef _WIN32
        case AF_UNIX   : sau = (const sockaddr_un&)_sa; break;
        #endif
        default        : throw system_error(make_error_code(errc::address_family_not_supported));
    }
}

bool SockAddr::operator== (const SockAddr& oth) const {
    if (family() != oth.family()) return false;
    switch (family()) {
        case AF_UNSPEC : return true;
        case AF_INET   : return !std::memcmp(&sa4, &oth.sa4, sizeof(sa4));
        case AF_INET6  : return !std::memcmp(&sa6, &oth.sa6, sizeof(sa6));
        #ifndef _WIN32
        case AF_UNIX   : return !std::strcmp(sau.sun_path, oth.sau.sun_path);
        #endif
        default        : throw system_error(make_error_code(errc::address_family_not_supported));
    }
}

std::ostream& operator<< (std::ostream& os, const SockAddr& sa) {
    switch (sa.family()) {
        case AF_UNSPEC : os << "<empty>"; break;
        case AF_INET   : os << sa.inet4().ip() << ':' << sa.inet4().port(); break;
        case AF_INET6  :
            os << '[' << sa.inet6().ip();
            if (sa.inet6().scope_id()) os << '%' << sa.inet6().scope_id();
            os << "]:" << sa.inet6().port();
            break;
        #ifndef _WIN32
        case AF_UNIX   : os << sa.unix().path(); break;
        #endif
        default        : throw system_error(make_error_code(errc::address_family_not_supported));
    }
    return os;
}

SockAddr::Inet4::Inet4 (const std::string_view& ip, uint16_t port) {
    _NULL_TERMINATE(ip, ipstr);
    memset(&sa4, 0, sizeof(sa4));
    sa4.sin_family = AF_INET;
    sa4.sin_port = htons(port);
    auto err = inet_pton4(ipstr, (unsigned char*)&(sa4.sin_addr));
    if (err) throw system_error(make_error_code((errc)err));
}

SockAddr::Inet4::Inet4 (const in_addr& addr, uint16_t port) {
    memset(&sa4, 0, sizeof(sa4));
    sa4.sin_family = AF_INET;
    sa4.sin_port = htons(port);
    sa4.sin_addr = addr;
}

string SockAddr::Inet4::ip () const {
    string ret;
    char* buf = ret.reserve(IP4_MAX_ADDRSTRLEN);
    auto err = inet_ntop4((const unsigned char*)&sa4.sin_addr, buf, IP4_MAX_ADDRSTRLEN);
    assert(!err);
    ret.length(std::strlen(buf));
    return ret;
}

SockAddr::Inet6::Inet6 (const std::string_view& ip, uint16_t port, uint32_t scope_id, uint32_t flowinfo) {
    memset(&sa6, 0, sizeof(sa6));
    sa6.sin6_family   = AF_INET6;
    sa6.sin6_port     = htons(port);
    sa6.sin6_flowinfo = htonl(flowinfo);

    int err;
    auto idx = ip.find('%');
    if (idx == std::string_view::npos) {
        sa6.sin6_scope_id = scope_id;
        _NULL_TERMINATE(ip, ipstr);
        err = inet_pton6(ipstr, (unsigned char*)&sa6.sin6_addr);
    } else {
        auto iplen = ip.length();
        char scope[iplen-idx];
        std::memcpy(scope, &ip[idx+1], iplen-idx-1);
        scope[iplen-idx]=0;
        #ifdef _WIN32
        sa6.sin6_scope_id = atoi(scope);
        #else
        sa6.sin6_scope_id = if_nametoindex(scope);
        #endif

        char ipstr[idx+1];
        std::memcpy(ipstr, ip.data(), idx);
        ipstr[idx] = 0;
        err = inet_pton6(ipstr, (unsigned char*)&sa6.sin6_addr);
    }

    if (err) throw system_error(make_error_code((errc)err));
}

SockAddr::Inet6::Inet6 (const in6_addr& addr, uint16_t port, uint32_t scope_id, uint32_t flowinfo) {
    memset(&sa6, 0, sizeof(sa6));
    sa6.sin6_family   = AF_INET6;
    sa6.sin6_port     = htons(port);
    sa6.sin6_addr     = addr;
    sa6.sin6_scope_id = scope_id;
    sa6.sin6_flowinfo = htonl(flowinfo);
}

string SockAddr::Inet6::ip () const {
    string ret;
    char* buf = ret.reserve(IP6_MAX_ADDRSTRLEN);
    auto err = inet_ntop6((const unsigned char*)&sa6.sin6_addr, buf, IP6_MAX_ADDRSTRLEN);
    assert(!err);
    ret.length(std::strlen(buf));
    return ret;
}

#ifndef _WIN32

SockAddr::Unix::Unix (const std::string_view& path) {
    if (path.length() >= sizeof(sau.sun_path)) throw system_error(make_error_code(errc::invalid_argument));
    sau.sun_family = AF_UNIX;
    memcpy(sau.sun_path, path.data(), path.length());
    sau.sun_path[path.length()] = 0;
}

#endif

}}
