#pragma once
#include <xs.h>
#include <panda/net/sockaddr.h>

namespace xs { namespace net {
    const panda::net::SockAddr* _in_sockaddr_ptr (pTHX_ SV* arg);
    panda::net::SockAddr        _in_sockaddr     (pTHX_ SV* arg);
    Sv _create_sockaddr (pTHX_ const panda::net::SockAddr& var);
}}


namespace xs {
    template <> struct Typemap<const panda::net::SockAddr*> : TypemapBase<const panda::net::SockAddr*> {
        const panda::net::SockAddr* in (pTHX_ SV* arg) { return xs::net::_in_sockaddr_ptr(aTHX_ arg); }
        Sv create (pTHX_ const panda::net::SockAddr* var, const Sv& = {}) {
            if (!var) return Sv::undef;
            return xs::net::_create_sockaddr(aTHX_ *var);
        }
    };

    template <> struct Typemap<panda::net::SockAddr> : TypemapBase<panda::net::SockAddr> {
        panda::net::SockAddr in (pTHX_ SV* arg) { return xs::net::_in_sockaddr(aTHX_ arg); }
        Sv create (pTHX_ const panda::net::SockAddr& var, const Sv& = {}) { return xs::net::_create_sockaddr(aTHX_ var); }
    };
}
