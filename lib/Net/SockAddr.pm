package Net::SockAddr;
use 5.012;
use XS::Framework;

XS::Loader::bootstrap();

use overload
    '""'     => \&_to_string,
    '=='     => \&_eq,
    '!='     => \&_ne,
    fallback => 1,
;

1;