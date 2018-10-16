#include "sockaddr.h"

namespace xs { namespace net {

    using panda::net::SockAddr;

    const SockAddr* _in_sockaddr_ptr (pTHX_ SV* arg) {
        if (!SvOK(arg)) return nullptr;
        if (!Sv(arg).is_object_ref()) throw "invalid sockaddr";
        return (const panda::net::SockAddr*)SvPVX(SvRV(arg));
    }

    SockAddr _in_sockaddr (pTHX_ SV* arg) {
        if (!SvOK(arg)) return {};
        if (Sv(arg).is_object_ref()) return *_in_sockaddr_ptr(aTHX_ arg);
        if (!SvPOK(arg) || SvCUR(arg) < sizeof(sockaddr)) throw "invalid sockaddr";
        const sockaddr& sa = *((const sockaddr*)SvPVX(arg));
        size_t minlen;
        switch (sa.sa_family) {
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

    Sv _create_sockaddr (pTHX_ const panda::net::SockAddr& var) {
        Stash stash;
        switch (var.family()) {
            case AF_UNSPEC : return Sv::undef;
            case AF_INET   : thread_local Stash _s1("Net::SockAddr::Inet4"); stash = _s1; break;
            case AF_INET6  : thread_local Stash _s2("Net::SockAddr::Inet6"); stash = _s2; break;
            #ifndef _WIN32
            case AF_UNIX   : thread_local Stash _s3("Net::SockAddr::Unix");  stash = _s3; break;
            #endif
            default: throw "invalid sockaddr family";
        }
        auto base = Simple(std::string_view(reinterpret_cast<const char*>(&var), sizeof(var)));
        return stash.bless(base).ref();
    }

}}
