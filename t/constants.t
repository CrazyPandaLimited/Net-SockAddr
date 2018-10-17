use 5.012;
use warnings;
use lib 't/lib';
use MyTest;
use Net::SockAddr;
use Socket();

is AF_INET,   Socket::AF_INET,  "AF_INET";
is AF_INET6,  Socket::AF_INET6, "AF_INET6";
is AF_UNIX(), Socket::AF_UNIX,  "AF_UNIX" if $^O ne 'MSWin32';

is INADDR_ANY,       Socket::INADDR_ANY,       "INADDR_ANY";
is INADDR_LOOPBACK,  Socket::INADDR_LOOPBACK,  "INADDR_LOOPBACK";
is INADDR_BROADCAST, Socket::INADDR_BROADCAST, "INADDR_BROADCAST";
is INADDR_NONE,      Socket::INADDR_NONE,      "INADDR_NONE";
is IN6ADDR_ANY,      Socket::IN6ADDR_ANY,      "IN6ADDR_ANY";
is IN6ADDR_LOOPBACK, Socket::IN6ADDR_LOOPBACK, "IN6ADDR_LOOPBACK";

done_testing();