#include "sockaddr.h"

namespace xs { namespace net {

using panda::net::SockAddr;

static PERL_THREAD_LOCAL HV* s1 = gv_stashpvs("Net::SockAddr::Inet4", GV_ADD);
static PERL_THREAD_LOCAL HV* s2 = gv_stashpvs("Net::SockAddr::Inet6", GV_ADD);
static PERL_THREAD_LOCAL HV* s3 = gv_stashpvs("Net::SockAddr::Unix", GV_ADD);

SockAddr* _in_sockaddr_ptr (SV* arg) {
    if (!SvOK(arg)) return nullptr;
    if (!Sv(arg).is_object_ref()) throw "invalid sockaddr";
    return (panda::net::SockAddr*)SvPVX(SvRV(arg));
}

SockAddr _in_sockaddr (SV* arg) {
    if (!SvOK(arg)) return {};
    if (Sv(arg).is_object_ref()) return *_in_sockaddr_ptr(arg);
    if (!SvPOK(arg) || SvCUR(arg) < sizeof(sa_family_t)) throw "invalid sockaddr";
    auto sa = (const sockaddr*)SvPVX(arg);
    size_t minlen;
    switch (sa->sa_family) {
        case AF_UNSPEC : return {};
        case AF_INET   : minlen = sizeof(sockaddr_in); break;
        case AF_INET6  : minlen = sizeof(sockaddr_in6); break;
        #ifndef _WIN32
        case AF_UNIX   : minlen = sizeof(sa_family_t) + 1 /* null-byte */; break;
        #endif
        default: throw "invalid sockaddr";
    }
    if (SvCUR(arg) < minlen) throw "invalid sockaddr";
    return sa;
}

Sv _create_sockaddr (const panda::net::SockAddr& var) {
    Stash stash;
    size_t sz = sizeof(var);
    switch (var.family()) {
        case AF_UNSPEC : return Sv::undef;
        case AF_INET   : stash = s1; break;
        case AF_INET6  : stash = s2; break;
        #ifndef _WIN32
        case AF_UNIX   : stash = s3; sz = sizeof(sa_family_t) + ((const SockAddr::Unix&)var).path().length() + 1; break;
        #endif
        default: throw "invalid sockaddr family";
    }
    auto base = Simple(panda::string_view(reinterpret_cast<const char*>(&var), sz));
    return stash.bless(base).ref();
}

}}
