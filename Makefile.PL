use 5.012;
use Config;
use XS::Install;

my %params = (
    NAME          => 'Net::SockAddr',
    ABSTRACT_FROM => 'lib/Net/SockAddr.pod',
    CPLUS         => 11,
    SRC           => ['src'],
    INC           => '-Isrc',
    BIN_DEPS      => ['XS::Framework', 'Test::Catch', 'Panda::Export'],
    CCFLAGS       => '-Wall -Wextra',
    OPTIMIZE      => '-O2',
    test          => {SRC => 't'},
    TEST_REQUIRES => {'Test::Exception' => 0},
    BIN_SHARE     => {
        CPLUS    => 11,
        INCLUDE  => {'src' => '/'},
    },
);

write_makefile(%params);
