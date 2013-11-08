KCN enabled w3m
================================================================================

build on NetBSD
--------------------------------------------------------------------------------
  You need to install libgc as follows:

    % cd /usr/pkg/devel/boehm-gc && sudo make update

You then run *configure* script as follows:

    % ./configure --with-gc=/usr/pkg

build on Linux
--------------------------------------------------------------------------------
  You need to install libgc, libtermcap and gettext.

    % sudo yum install libgc-devel (CentOS 5.x)
    % sudo yum install gc (CentOS 6.x)
    % sudo yum install libtermcap-devel.i386
    % sudo yum install gettext
