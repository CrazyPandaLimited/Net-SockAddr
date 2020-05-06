#include "sockaddr.h"
#include <ostream>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <net/if.h>
#endif

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

// this function is fix for clang build, see MEIACORE-723 for details
// htonl is statement in clang, but not exression, it cannot be used dirrectly in var initializers
template <typename T>
static in_addr to_inaddr(T src) {
    auto ia = htonl(src);
    return reinterpret_cast<const in_addr&>(ia);
}

const in_addr SockAddr::Inet4::addr_any       = to_inaddr(INADDR_ANY);
const in_addr SockAddr::Inet4::addr_loopback  = to_inaddr(INADDR_LOOPBACK);
const in_addr SockAddr::Inet4::addr_broadcast = to_inaddr(INADDR_BROADCAST);
const in_addr SockAddr::Inet4::addr_none      = to_inaddr(INADDR_NONE);

const in6_addr SockAddr::Inet6::addr_any      = IN6ADDR_ANY_INIT;
const in6_addr SockAddr::Inet6::addr_loopback = IN6ADDR_LOOPBACK_INIT;

const SockAddr::Inet4 SockAddr::Inet4::sa_any     (SockAddr::Inet4::addr_any, 0);
const SockAddr::Inet4 SockAddr::Inet4::sa_loopback(SockAddr::Inet4::addr_loopback, 0);
const SockAddr::Inet6 SockAddr::Inet6::sa_any     (SockAddr::Inet6::addr_any, 0);
const SockAddr::Inet6 SockAddr::Inet6::sa_loopback(SockAddr::Inet6::addr_loopback, 0);

static system_error _not_supported () { return system_error(make_error_code(errc::address_family_not_supported)); }

bool SockAddr::is_valid(const sockaddr *sa, size_t length) noexcept {
    if (length < sizeof (sa_family_t)) return false;
    #ifndef _WIN32
    if (length > sizeof (sockaddr_un)) return false;
    #else
    if (length > sizeof (sockaddr_in6)) return false;
    #endif

    switch (sa->sa_family) {
        #ifndef _WIN32
        case AF_UNIX   :
        #endif
        case AF_INET   : /* no reasonable assumption about valid bytes */
        case AF_INET6  : /* no reasonable assumption about valid bytes */
        case AF_UNSPEC : return true;
        default        : return false;
    }
}

void SockAddr::validate(const sockaddr* sa, size_t length) {
    if (!is_valid(sa, length)) throw system_error(make_error_code(errc::bad_address));
}


SockAddr::SockAddr (const sockaddr* _sa, size_t length) {
    validate(_sa, length);
    switch (_sa->sa_family) {
        case AF_UNSPEC : sa.sa_family = AF_UNSPEC; break;
        case AF_INET   : sa4 = *(const sockaddr_in*)_sa; break;
        case AF_INET6  : sa6 = *(const sockaddr_in6*)_sa; break;
        #ifndef _WIN32
        case AF_UNIX   : memcpy(&this->sa, _sa, length); assure_correct_unix(length); break;
        #endif
    }
}

void SockAddr::assure_correct_unix(size_t length) noexcept {
    switch (sa.sa_family) {
#ifndef _WIN32
    case AF_UNIX: {
        // write null-byte by force
        if (length == 2) { length = 3; }
        ((char*)&sa)[length - 1] = 0;
        break;
    }
#endif
    case AF_INET:  if (length < sizeof(sockaddr_in))  memset(((char*)&sa4) + length, 0, sizeof(sockaddr_in)  - length); break;
    case AF_INET6: if (length < sizeof(sockaddr_in6)) memset(((char*)&sa6) + length, 0, sizeof(sockaddr_in6) - length); break;
    }
}

SockAddr& SockAddr::operator=(const SockAddr& oth) {
    if (this != &oth) {
        memcpy(&sa, &oth.sa, oth.length());
    }
    return *this;
}


bool SockAddr::operator== (const SockAddr& oth) const {
    if (family() != oth.family()) return false;
    switch (family()) {
        case AF_UNSPEC : return true;
        case AF_INET   : return !std::memcmp(&sa4, &oth.sa4, sizeof(sockaddr_in));
        case AF_INET6  : return !std::memcmp(&sa6, &oth.sa6, sizeof(sockaddr_in6));
        #ifndef _WIN32
        case AF_UNIX   : {
            auto l1 = length();
            auto l2 = oth.length();
            if (l1 != l2) return false;
            return !std::memcmp(&sau, &oth.sau, l1);
        }
        #endif
        default        : throw _not_supported();
    }
}

string SockAddr::ip () const {
    switch (sa.sa_family) {
        case AF_UNSPEC: return {};
        case AF_INET:   return inet4().ip();
        case AF_INET6:  return inet6().ip();
        default: throw _not_supported();
    }
}

uint16_t SockAddr::port () const {
    switch (sa.sa_family) {
        case AF_UNSPEC: return 0;
        case AF_INET:   return inet4().port();
        case AF_INET6:  return inet6().port();
        default: throw _not_supported();
    }
}

size_t SockAddr::length () const {
    switch (sa.sa_family) {
        case AF_UNSPEC: return sizeof(sa_family_t);
        case AF_INET:   return sizeof(sockaddr_in);
        case AF_INET6:  return sizeof(sockaddr_in6);
        #ifndef _WIN32
        case AF_UNIX:   return sizeof(sa_family_t) + strlen(sau.sun_path) + 1; /* null-byte */
        #endif
        default: throw _not_supported();
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
        default        : throw _not_supported();
    }
    return os;
}

SockAddr::Inet4::Inet4 (const string_view& ip, uint16_t port) {
    _NULL_TERMINATE(ip, ipstr);
    memset(&sa4, 0, sizeof(sockaddr_in));
    sa4.sin_family = AF_INET;
    sa4.sin_port = htons(port);
    auto err = inet_pton4(ipstr, (unsigned char*)&(sa4.sin_addr));
    if (err) throw system_error(make_error_code((errc)err));
}

SockAddr::Inet4::Inet4 (const in_addr& addr, uint16_t port) {
    memset(&sa4, 0, sizeof(sockaddr_in));
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

SockAddr::Inet6::Inet6 (const string_view& ip, uint16_t port, uint32_t scope_id, uint32_t flowinfo) {
    memset(&sa6, 0, sizeof(sockaddr_in6));
    sa6.sin6_family   = AF_INET6;
    sa6.sin6_port     = htons(port);
    sa6.sin6_flowinfo = htonl(flowinfo);

    int err;
    auto idx = ip.find('%');
    if (idx == string_view::npos) {
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
    memset(&sa6, 0, sizeof(sockaddr_in6));
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

SockAddr::Unix::Unix (const string_view& path) {
    if (path.length() >= sizeof(sau.sun_path)) throw system_error(make_error_code(errc::invalid_argument));
    sau.sun_family = AF_UNIX;
    memcpy(sau.sun_path, path.data(), path.length());
    sau.sun_path[path.length()] = 0;
}

#endif

}}
