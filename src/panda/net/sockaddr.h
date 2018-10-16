#pragma once
#include <panda/string.h>
#include <panda/string_view.h>
#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <netinet/in.h>
    #include <sys/un.h>
#endif

namespace panda { namespace net {

static const int IP4_MAX_ADDRSTRLEN = 16;
static const int IP6_MAX_ADDRSTRLEN = 46;

struct SockAddr {
    struct Inet4;
    struct Inet6;

    SockAddr () { sa.sa_family = AF_UNSPEC; }

    SockAddr (const sockaddr& sa);
    SockAddr (const sockaddr_in&  sa) : sa4(sa) {}
    SockAddr (const sockaddr_in6& sa) : sa6(sa) {}

    SockAddr (const SockAddr& oth) : SockAddr(oth.sa) {}

    sa_family_t family () const { return sa.sa_family; }

    bool is_inet4 () const { return family() == AF_INET; }
    bool is_inet6 () const { return family() == AF_INET6; }

    Inet4& inet4 () const { return *((Inet4*)this); }
    Inet6& inet6 () const { return *((Inet6*)this); }

    const sockaddr& get () const { return sa; }

    bool operator== (const SockAddr& oth) const;
    bool operator!= (const SockAddr& oth) const { return !operator==(oth); }

    #ifndef _WIN32

    struct Unix;

    SockAddr (const sockaddr_un&  sa) : sau(sa) {}

    bool is_unix () const { return family() == AF_UNIX; }

    Unix& unix () const { return *((Unix*)this); }

    #endif

protected:
    union {
        sockaddr     sa;
        sockaddr_in  sa4;
        sockaddr_in6 sa6;
        #ifndef _WIN32
        sockaddr_un  sau;
        #endif
    };
};

std::ostream& operator<< (std::ostream&, const SockAddr&);

struct SockAddr::Inet4 : SockAddr {
    Inet4 (const sockaddr_in& sa) : SockAddr(sa)      {}
    Inet4 (const Inet4& oth)      : SockAddr(oth.sa4) {}

    Inet4 (const std::string_view& ip, uint16_t port);
    Inet4 (const in_addr& addr, uint16_t port);

    const in_addr&     addr () const { return sa4.sin_addr; }
    const sockaddr_in& get  () const { return sa4; }

    uint16_t port () const { return ntohs(sa4.sin_port); }
    string   ip   () const;
};

struct SockAddr::Inet6 : SockAddr {
    Inet6 (const sockaddr_in6& sa) : SockAddr(sa)      {}
    Inet6 (const Inet6& oth)       : SockAddr(oth.sa6) {}

    Inet6 (const std::string_view& ip, uint16_t port, uint32_t scope_id = 0, uint32_t flowinfo = 0);
    Inet6 (const in6_addr& addr, uint16_t port, uint32_t scope_id = 0, uint32_t flowinfo = 0);

    const in6_addr&     addr () const { return sa6.sin6_addr; }
    const sockaddr_in6& get  () const { return sa6; }

    string   ip       () const;
    uint16_t port     () const { return ntohs(sa6.sin6_port); }
    uint32_t scope_id () const { return sa6.sin6_scope_id; }
    uint32_t flowinfo () const { return ntohl(sa6.sin6_flowinfo); }
};

#ifndef _WIN32

struct SockAddr::Unix : SockAddr {
    Unix (const sockaddr_un& sa) : SockAddr(sa)      {}
    Unix (const Unix& oth)       : SockAddr(oth.sau) {}

    Unix (const std::string_view& path);

    std::string_view path () const { return (char*)sau.sun_path; }

    const sockaddr_un& get () const { return sau; }
};

#endif

}}
