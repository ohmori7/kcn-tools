KCN enabled w3m
================================================================================

build and install
================================================================================
  You first need to install required libraries, Boehm gc, termcap and gettex.
Please refert to a section, ''required libraries and their installations'', in
this document if you need any help about installations of required libraries.

You can now build and install as follows:

    % ./configure
    % make
    % sudo make install

Required libraries and their installations
================================================================================
NetBSD
--------------------------------------------------------------------------------
  You need to install libgc on NetBSD as follows:

    % cd /usr/pkg/devel/boehm-gc && sudo make update

Other libraries, termcap and gettext, seem to be included in base system.

CentOS
--------------------------------------------------------------------------------
  You need to install libgc, libtermcap and gettext as follows:

    % sudo yum install libgc-devel (CentOS 5.x)
    % sudo yum install gc-devel (CentOS 6.x)
    % sudo yum install libtermcap-devel
    % sudo yum install gettext
