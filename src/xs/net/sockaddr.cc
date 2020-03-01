#include "sockaddr.h"

namespace xs { namespace net {

using panda::net::SockAddr;

static PERL_THREAD_LOCAL struct {
    Stash s1 = Stash("Net::SockAddr::Inet4", GV_ADD);
    Stash s2 = Stash("Net::SockAddr::Inet6", GV_ADD);
    Stash s3 = Stash("Net::SockAddr::Unix", GV_ADD);
} tls;

SockAddr* _in_sockaddr_ptr (SV* arg) {
    if (!SvOK(arg)) return nullptr;
    if (!Sv(arg).is_object_ref()) throw "invalid sockaddr";
    return (panda::net::SockAddr*)SvPVX(SvRV(arg));
}

SockAddr _in_sockaddr (SV* arg) {
    if (!SvOK(arg)) return {};
    if (Sv(arg).is_object_ref()) return *_in_sockaddr_ptr(arg);
    if (!SvPOK(arg) || SvCUR(arg) < sizeof(sockaddr)) throw "invalid sockaddr";
    auto sa = (const sockaddr*)SvPVX(arg);
    size_t minlen;
    switch (sa->sa_family) {
        case AF_UNSPEC : return {};
        case AF_INET   : minlen = sizeof(sockaddr_in); break;
        case AF_INET6  : minlen = sizeof(sockaddr_in6); break;
        #ifndef _WIN32
        case AF_UNIX   : minlen = sizeof(sockaddr_un); break;
        #endif
        default: throw "invalid sockaddr";
    }
    if (SvCUR(arg) < minlen) throw "invalid sockaddr";
    return sa;
}

Sv _create_sockaddr (const panda::net::SockAddr& var) {
    Stash stash;
    switch (var.family()) {
        case AF_UNSPEC : return Sv::undef;
        case AF_INET   : stash = tls.s1; break;
        case AF_INET6  : stash = tls.s2; break;
        #ifndef _WIN32
        case AF_UNIX   : stash = tls.s3; break;
        #endif
        default: throw "invalid sockaddr family";
    }
    auto base = Simple(panda::string_view(reinterpret_cast<const char*>(&var), sizeof(var)));
    return stash.bless(base).ref();
}

}}
