KCN enabled w3m
================================================================================

build on NetBSD
--------------------------------------------------------------------------------
  You need to install libgc as follows:

    % cd /usr/pkg/devel/boehm-gc && sudo make update

You can now build and install as follows:

    % ./configure --with-gc=/usr/pkg
    % make
    % sudo make install

build on CentOS
--------------------------------------------------------------------------------
  You need to install libgc, libtermcap and gettext as follows:

    % sudo yum install libgc-devel (CentOS 5.x)
    % sudo yum install gc-devel (CentOS 6.x)
    % sudo yum install libtermcap-devel
    % sudo yum install gettext

You can now build and install as follows:

    % ./configure
    % make
    % sudo make install
