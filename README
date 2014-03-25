..	-*- mode: rst-mode -*-
..
.. Version number is filled in automatically.
.. |version| replace:: 0.23

===============================================
PySubnetTree - A Python Module for CIDR Lookups
===============================================

.. rst-class:: opening

    The PySubnetTree package provides a Python data structure
    ``SubnetTree`` which maps subnets given in `CIDR
    <http://tools.ietf.org/html/rfc4632>`_ notation (incl.
    corresponding IPv6 versions) to Python objects. Lookups are
    performed by longest-prefix matching.


Download
--------

You can find the latest PySubnetTree release for download at
http://www.bro.org/download.

PySubnetTree's git repository is located at `git://git.bro.org/pysubnettree.git
<git://git.bro.org/pysubnettree.git>`__. You can browse the repository
`here <http://git.bro.org/pysubnettree.git>`__.

This document describes PySubnetTree |version|. See the ``CHANGES``
file for version history.


Example
-------

A simple example which associates CIDR prefixes with strings::

    >>> import SubnetTree
    >>> t = SubnetTree.SubnetTree()
    >>> t["10.1.0.0/16"] = "Network 1"
    >>> t["10.1.42.0/24"] = "Network 1, Subnet 42"
    >>> print "10.1.42.1" in t
    True
    >>> print t["10.1.42.1"]
    Network 1, Subnet 42
    >>> print t["10.1.43.1"]
    Network 1
    >>> print "10.20.1.1" in t
    False
    >>> try:
    ...     print t["10.20.1.1"]
    ... except KeyError, err:
    ...     print "Error: %s not found" % err
    Error: '10.20.1.1' not found

By default, CIDR prefixes and IP addresses are given as strings.
Alternatively, a ``SubnetTree`` object can be switched into *binary
mode*, in which single addresses are passed in the form of packed
binary strings as, e.g., returned by `socket.inet_aton
<http://docs.python.org/2/library/socket.html#socket.inet_aton>`_::


    >>> t.get_binary_lookup_mode()
    False
    >>> t.set_binary_lookup_mode(True)
    >>> t.get_binary_lookup_mode()
    True
    >>> import socket
    >>> print t[socket.inet_aton("10.1.42.1")]
    Network 1, Subnet 42

A SubnetTree also provides methods ``insert(prefix,object=None)`` for insertion
of prefixes (``object`` can be skipped to use the tree like a set), and
``remove(prefix)`` for removing entries (``remove`` performs an _exact_ match
rather than longest-prefix).

Internally, the CIDR prefixes of a ``SubnetTree`` are managed by a
Patricia tree data structure and lookups are therefore efficient
even with a large number of prefixes.

PySubnetTree comes with a BSD license.


Prerequisites
-------------

This package requires Python 2.4 or newer.

Installation
------------

Installation is pretty simple::

   > python setup.py install
