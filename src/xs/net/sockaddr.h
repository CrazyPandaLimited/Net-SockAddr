#pragma once
#include <xs.h>
#include <panda/net/sockaddr.h>

namespace xs { namespace net {
    panda::net::SockAddr* _in_sockaddr_ptr (pTHX_ SV* arg);
    panda::net::SockAddr  _in_sockaddr     (pTHX_ SV* arg);
    Sv _create_sockaddr (pTHX_ const panda::net::SockAddr& var);
}}


namespace xs {
    template <class TYPE> struct Typemap<panda::net::SockAddr*, TYPE> : TypemapBase<panda::net::SockAddr*, TYPE> {
        static panda::net::SockAddr* in (pTHX_ SV* arg) { return xs::net::_in_sockaddr_ptr(aTHX_ arg); }
        static Sv out (pTHX_ const panda::net::SockAddr* var, const Sv& = {}) {
            if (!var) return Sv::undef;
            return xs::net::_create_sockaddr(aTHX_ *var);
        }
    };

    template <> struct Typemap<panda::net::SockAddr> : TypemapBase<panda::net::SockAddr> {
        static panda::net::SockAddr in (pTHX_ SV* arg) { return xs::net::_in_sockaddr(aTHX_ arg); }
        static Sv out (pTHX_ const panda::net::SockAddr& var, const Sv& = {}) { return xs::net::_create_sockaddr(aTHX_ var); }
    };

    template <> struct Typemap<const panda::net::SockAddr&> : Typemap<panda::net::SockAddr> {};
}
