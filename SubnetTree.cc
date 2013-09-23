#include "SubnetTree.h"

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>

static PyObject* dummy = Py_BuildValue("s", "<dummy string>");

inline static prefix_t* make_prefix(int family, inx_addr * addr, unsigned int width)
{
    if ( ! (family == AF_INET || family == AF_INET6) )
        return 0;

    if ( family == AF_INET && width > 32 )
        return 0;

    if ( family == AF_INET6 && width > 128 )
        return 0;

    prefix_t* subnet = (prefix_t*) malloc(sizeof(prefix_t));

	if ( ! subnet )
		return 0;

    if ( family == AF_INET )
        memcpy(&subnet->add.sin, addr, sizeof(subnet->add.sin));

    else if ( family == AF_INET6 )
        memcpy(&subnet->add.sin6, addr, sizeof(subnet->add.sin6));

    subnet->family = family;
    subnet->bitlen = width;
    subnet->ref_count = 1;

    return subnet;
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
    prefix_t* sn = make_prefix(family, &subnet, mask);
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
    prefix_t* subnet = make_prefix(family, &addr, mask);
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
    int mask = family == AF_INET ? 32 : 128;
    prefix_t* subnet = make_prefix(family, &addr, mask);
    patricia_node_t* node = patricia_search_best(tree, subnet);
    Deref_Prefix(subnet);

    if ( ! node )
        return 0;

    PyObject* data = (PyObject*)node->data;
    Py_INCREF(data);

    return data;
}

bool SubnetTree::get_binary_lookup_mode()
{
    return binary_lookup_mode;
}

void SubnetTree::set_binary_lookup_mode(bool arg_binary_lookup_mode)
{
    binary_lookup_mode = arg_binary_lookup_mode;
}
