#include "SubnetTree.h"

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>

static PyObject* dummy = Py_BuildValue("s", "<dummy string>");

const uint8_t v4_mapped_prefix[12] = { 0, 0, 0, 0,
                                       0, 0, 0, 0,
                                       0, 0, 0xff, 0xff };

inline static prefix_t* make_prefix()
{
    prefix_t* rval = (prefix_t*) malloc(sizeof(prefix_t));
    rval->ref_count = 1;
    return rval;
}

inline static bool set_prefix(prefix_t* subnet, int family, inx_addr* addr, unsigned int width)
{
    if ( ! (family == AF_INET || family == AF_INET6) )
        return false;

    if ( family == AF_INET && width > 32 )
        return false;

    if ( family == AF_INET6 && width > 128 )
        return false;

    if ( family == AF_INET )
        {
        memcpy(&subnet->add.sin6, v4_mapped_prefix, sizeof(v4_mapped_prefix));
        memcpy(&subnet->add.sin6.s6_addr[12], addr, sizeof(in_addr));
        }

    else if ( family == AF_INET6 )
        memcpy(&subnet->add.sin6, addr, sizeof(subnet->add.sin6));

    subnet->family = AF_INET6;
    subnet->bitlen = (family == AF_INET ? width + 96 : width);

    return true;
}

inline static bool parse_cidr(const char *cidr, int *family, inx_addr *subnet, unsigned short *mask)
{
    char buffer[40];
    const char *addr_str = 0;
    const char *mask_str = 0;
    char *endptr;

    if ( ! cidr )
        return false;

    const char *slash = strchr(cidr, '/');

    if ( slash ) {
        int len = slash - cidr < 40 ? slash - cidr : 39;
        memcpy(buffer, cidr, len);
        buffer[len] = '\0';
        addr_str = buffer;
        mask_str = slash + 1;
    }
    else {
        addr_str = cidr;
        mask_str = 0;
    }

    *family = AF_INET;

    if ( inet_pton(*family, addr_str, subnet) != 1 ) {
        *family = AF_INET6;

        if ( inet_pton(*family, addr_str, subnet) != 1 )
            return false;
    }

    if ( mask_str ) {
        errno = 0;
        *mask = strtol(mask_str, &endptr, 10);

        if ( endptr == mask_str || errno != 0 )
            return false;

        if ( *family == AF_INET && *mask > 32 )
            return false;
        else if ( *mask > 128 )
            return false;
    }
    else {
        if ( *family == AF_INET )
            *mask = 32;
        else
            *mask = 128;
    }

    return true;
}

static void free_data(void *data)
{
    Py_DECREF(static_cast<PyObject*>(data));
}

SubnetTree::SubnetTree(bool arg_binary_lookup_mode)
{
    tree = New_Patricia(128);
    binary_lookup_mode = arg_binary_lookup_mode;
}

SubnetTree::~SubnetTree()
{
    Destroy_Patricia(tree, (void (*)())free_data);
}

PyObject* SubnetTree::insert(const char *cidr, PyObject* data)
{
    int family;
    inx_addr subnet;
    unsigned short mask;

    if ( ! parse_cidr(cidr, &family, &subnet, &mask) ) {
        PyErr_SetString(PyExc_ValueError, "Invalid CIDR.");
        return 0;
    }

    return insert(family, subnet, mask, data);
}

PyObject* SubnetTree::insert(unsigned long subnet, unsigned short mask, PyObject* data)
{
    inx_addr subnet_addr;
    memcpy (&subnet_addr, &subnet, sizeof(subnet));

    return insert(AF_INET, subnet_addr, mask, data);
}

PyObject* SubnetTree::insert(int family, inx_addr subnet, unsigned short mask, PyObject * data)
{
    prefix_t* sn = make_prefix();

    if ( ! sn ) {
        PyErr_SetString(PyExc_MemoryError, "out of memory");
        return 0;
    }

    bool res = set_prefix(sn, family, &subnet, mask);

    if ( ! res ) {
        Deref_Prefix(sn);
        PyErr_SetString(PyExc_RuntimeError, "invalid subnet/prefix");
        return 0;
    }

    patricia_node_t* node = patricia_lookup(tree, sn);
    Deref_Prefix(sn);

    if ( ! node ) {
        PyErr_SetString(PyExc_RuntimeError, "patricia_lookup failed.");
        return 0;
    }

    if ( ! data )
        data = dummy;

    Py_INCREF(data);
    node->data = data;

    Py_RETURN_TRUE;
}

PyObject* SubnetTree::remove(const char *cidr)
{
    int family;
    inx_addr subnet;
    unsigned short mask;

    if ( ! parse_cidr(cidr, &family, &subnet, &mask) ) {
        PyErr_SetString(PyExc_ValueError, "Invalid CIDR.");
        return 0;
    }

    return remove(family, subnet, mask);
}

PyObject* SubnetTree::remove(unsigned long addr, unsigned short mask)
{
    inx_addr subnet_addr;
    memcpy(&subnet_addr, &addr, sizeof(addr));

    return remove(AF_INET, subnet_addr, mask);
}

PyObject* SubnetTree::remove(int family, inx_addr addr, unsigned short mask)
{
    prefix_t* subnet = make_prefix();

    if ( ! subnet ) {
        PyErr_SetString(PyExc_MemoryError, "out of memory");
        return 0;
    }

    bool res = set_prefix(subnet, family, &addr, mask);

    if ( ! res ) {
        Deref_Prefix(subnet);
        PyErr_SetString(PyExc_RuntimeError, "invalid subnet/prefix");
        return 0;
    }

    patricia_node_t* node = patricia_search_exact(tree, subnet);
    Deref_Prefix(subnet);

    if ( ! node ) {
        PyErr_SetString(PyExc_RuntimeError, "patricia_lookup failed.");
        return 0;
    }

    PyObject* data = (PyObject*)node->data;
    Py_DECREF(data);

    patricia_remove(tree, node);

    if ( data != dummy )
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

PyObject* SubnetTree::lookup(const char *cidr, int size) const
{
    int family;
    inx_addr subnet;
    unsigned short mask;

    if ( binary_lookup_mode ) {
        if ( size == 4 )
            family = AF_INET;

        else if ( size == 16 )
            family = AF_INET6;

        else {
            PyErr_SetString(PyExc_ValueError, "Invalid binary address.  Binary addresses are 4 or 16 bytes.");
            return 0;
        }

        memcpy(&subnet, cidr, size);
        return lookup(family, subnet);
    }

    else {
        if ( ! parse_cidr(cidr, &family, &subnet, &mask) ) {
            return 0;
        }

        return lookup(family, subnet);
    }
}

PyObject* SubnetTree::lookup(unsigned long addr) const
{
    inx_addr addr_addr;
    memcpy(&addr_addr, &addr, sizeof(addr));

    return lookup(AF_INET, addr_addr);
}

PyObject* SubnetTree::lookup(int family, inx_addr addr) const
{
    prefix_t* subnet = make_prefix();

    if ( ! subnet ) {
        PyErr_SetString(PyExc_RuntimeError, "invalid subnet/prefix");
        return 0;
    }

    int mask = family == AF_INET ? 32 : 128;
    bool res = set_prefix(subnet, family, &addr, mask);

    if ( ! res ) {
        Deref_Prefix(subnet);
        PyErr_SetString(PyExc_MemoryError, "out of memory");
        return 0;
    }

    patricia_node_t* node = patricia_search_best(tree, subnet);
    Deref_Prefix(subnet);

    if ( ! node )
        return 0;

    PyObject* data = (PyObject*)node->data;
    Py_INCREF(data);

    return data;
}

PyObject* SubnetTree::prefixes(bool ipv4_native /*=false*/, bool with_len /*=true*/) const
{
    char buf[INET6_ADDRSTRLEN];
    char buffer[64];
    bool wrote_buffer;
    PyObject* set = PySet_New(NULL);

    patricia_node_t *node;

    PATRICIA_WALK (tree->head, node) {
        prefix_t* pf = node->prefix;
        PyObject* pstr = NULL;

        wrote_buffer = false;

        if ( ipv4_native ) {
            // IPv4 addresses are stored mapped into the IPv6 space. (Xref:
            // https://en.wikipedia.org/wiki/IPv6#IPv4-mapped_IPv6_addresses)
            // We'll check the first 12 bytes (96 bits) of the stored address
            // to see if they match v4_mapped_prefix.
            uint8_t* addrstart = (uint8_t*) &pf->add.sin6;

            if ( memcmp(&v4_mapped_prefix, addrstart, 12) == 0 ) {
                // Skip over the mapped-prefix to the IPV4 addr part. And we
                // need to correct the bitlen to make it valid for IPv4 (by
                // subtracting the 96 mapped-prefix bits).
                addrstart += 12;

                if ( with_len ) {
                    snprintf(buffer, sizeof buffer, "%d.%d.%d.%d/%d",
                                               addrstart[0], addrstart[1],
                                               addrstart[2], addrstart[3],
                                               pf->bitlen - 96);
                }

                else {
                    snprintf(buffer, sizeof buffer, "%d.%d.%d.%d",
                                               addrstart[0], addrstart[1],
                                               addrstart[2], addrstart[3]);
                }

                wrote_buffer = true;
            }
        }

        if ( ! wrote_buffer ) {
            // Format as IPv6 address.

            const char* addrstr = inet_ntop(AF_INET6, &pf->add.sin6, buf, INET6_ADDRSTRLEN);

            if ( ! addrstr ) {
                PyErr_SetString(PyExc_ValueError, "Unable to string-ify IPv6 address.");
                return NULL;
            }

            if ( with_len )
                snprintf(buffer, sizeof buffer, "%s/%d", addrstr, pf->bitlen);
            else
                snprintf(buffer, sizeof buffer, "%s", addrstr);
        }

#if PY_MAJOR_VERSION >= 3
        pstr = PyUnicode_FromString(buffer);
#else
        pstr = PyString_FromString(buffer);
#endif

        PySet_Add(set, pstr);
        Py_DECREF(pstr);

    } PATRICIA_WALK_END;

    return set;
}

bool SubnetTree::get_binary_lookup_mode()
{
    return binary_lookup_mode;
}

void SubnetTree::set_binary_lookup_mode(bool arg_binary_lookup_mode)
{
    binary_lookup_mode = arg_binary_lookup_mode;
}
