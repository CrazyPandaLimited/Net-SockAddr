#include <xs/net/sockaddr.h>
#include <xs/export.h>
#include <sstream>

using namespace xs;
using namespace xs::exp;
using namespace panda::net;
using panda::string;
using std::string_view;

template <class T>
static inline string_view addr2sv (const T& addr) { return string_view((const char*)&addr, sizeof(T)); }

MODULE = Net::SockAddr                PACKAGE = Net::SockAddr
PROTOTYPES: DISABLE

BOOT {
    Stash me(__PACKAGE__);
    Stash("Net::SockAddr::Inet4", GV_ADD).inherit(me);
    Stash("Net::SockAddr::Inet6", GV_ADD).inherit(me);
    Stash("Net::SockAddr::Unix",  GV_ADD).inherit(me);
    
    constant_t constants[] = {
        {"AF_UNSPEC", AF_UNSPEC, NULL},
        {"AF_INET",   AF_INET,   NULL},
        {"AF_INET6",  AF_INET6,  NULL},
        #ifndef _WIN32
        {"AF_UNIX",   AF_UNIX,   NULL},
        #endif
        {"autoexport", 1, NULL},
        {NULL, 0, NULL}
    };
    create_constants(aTHX_ me, constants);
    
    create_constants(aTHX_ me, Hash({
        {"INADDR_ANY",       Simple(addr2sv(SockAddr::Inet4::ADDR_ANY))       },
        {"INADDR_LOOPBACK",  Simple(addr2sv(SockAddr::Inet4::ADDR_LOOPBACK))  },
        {"INADDR_BROADCAST", Simple(addr2sv(SockAddr::Inet4::ADDR_BROADCAST)) },
        {"INADDR_NONE",      Simple(addr2sv(SockAddr::Inet4::ADDR_NONE))      },
        {"IN6ADDR_ANY",      Simple(addr2sv(SockAddr::Inet6::ADDR_ANY))       },
        {"IN6ADDR_LOOPBACK", Simple(addr2sv(SockAddr::Inet6::ADDR_LOOPBACK))  },
    }));
}

SockAddr SockAddr::new (SockAddr oth) {
    RETVAL = oth;
}

sa_family_t SockAddr::family ()

bool SockAddr::is_inet4 ()

bool SockAddr::is_inet6 ()

#ifndef _WIN32

bool SockAddr::is_unix ()

#endif

string_view SockAddr::get () {
    size_t len;
    switch (THIS->family()) {
        case AF_UNSPEC : XSRETURN_UNDEF;
        case AF_INET   : len = sizeof(sockaddr_in); break;
        case AF_INET6  : len = sizeof(sockaddr_in6); break;
        default        : len = sizeof(sockaddr_un); break; // AF_UNIX
    }
    RETVAL = string_view((const char*)THIS->get(), len);
}

std::string SockAddr::_to_string (...) {
    std::ostringstream os;
    os << *THIS;
    RETVAL = os.str();
}

bool SockAddr::_eq (SockAddr oth, ...) { RETVAL = *THIS == oth; }

bool SockAddr::_ne (SockAddr oth, ...) { RETVAL = *THIS != oth; }

MODULE = Net::SockAddr                PACKAGE = Net::SockAddr::Inet4
PROTOTYPES: DISABLE

SockAddr from_addr (string_view addr, uint16_t port) {
    if (addr.length() != sizeof(in_addr)) throw "invalid ip4 addr";
    RETVAL = SockAddr::Inet4(*(const in_addr*)addr.data(), port);
}

SockAddr new (SV*, string_view ip, uint16_t port) {
    RETVAL = SockAddr::Inet4(ip, port);
}

string SockAddr::ip () {
    RETVAL = THIS->inet4().ip();
}

uint16_t SockAddr::port () {
    RETVAL = THIS->inet4().port();
}

string_view SockAddr::addr () {
    RETVAL = addr2sv(THIS->inet4().addr());
}


MODULE = Net::SockAddr                PACKAGE = Net::SockAddr::Inet6
PROTOTYPES: DISABLE

SockAddr from_addr (string_view addr, uint16_t port, uint32_t scope_id = 0, uint32_t flow_info = 0) {
    if (addr.length() != sizeof(in6_addr)) throw "invalid ip6 addr";
    RETVAL = SockAddr::Inet6(*(const in6_addr*)addr.data(), port, scope_id, flow_info);
}

SockAddr new (SV*, string_view ip, uint16_t port, uint32_t scope_id = 0, uint32_t flow_info = 0) {
    RETVAL = SockAddr::Inet6(ip, port, scope_id, flow_info);
}

string SockAddr::ip () {
    RETVAL = THIS->inet6().ip();
}

uint16_t SockAddr::port () {
    RETVAL = THIS->inet6().port();
}

uint32_t SockAddr::scope_id () {
    RETVAL = THIS->inet6().scope_id();
}

uint32_t SockAddr::flowinfo () {
    RETVAL = THIS->inet6().flowinfo();
}

string_view SockAddr::addr () {
    RETVAL = addr2sv(THIS->inet6().addr());
}


MODULE = Net::SockAddr                PACKAGE = Net::SockAddr::Unix
PROTOTYPES: DISABLE

#ifndef _WIN32

SockAddr new (SV*, string_view path) {
    RETVAL = SockAddr::Unix(path);
}

string_view SockAddr::path () {
    RETVAL = THIS->unix().path();
}

#endif