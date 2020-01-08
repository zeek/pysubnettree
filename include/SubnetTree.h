extern "C" {
#include "Python.h"
#include "patricia.h"
}

#ifdef SWIG
// If a function is supposed to accept 4-byte tuples as packet by
// socket.inet_aton(), it needs to accept strings which contain 0s.
// Therefore, we need a size parameter.
%typemap(in) (char* cidr, int size) (PyObject* ascii)
	%{
	Py_ssize_t len;

#if PY_MAJOR_VERSION >= 3
	if ( PyUnicode_Check($input) )
		{
		ascii = PyUnicode_AsASCIIString($input);

		if ( ! ascii )
			{
			PyErr_SetString(PyExc_TypeError, "Expected a ASCII encodable string or bytes");
			return NULL;
			}

		PyBytes_AsStringAndSize(ascii, &$1, &len);
		$2 = len;
		}
	else if ( PyBytes_Check($input) )
		{
		PyBytes_AsStringAndSize($input, &$1, &len);
		$2 = len;
		}
	else
		{
		PyErr_SetString(PyExc_TypeError, "Expected a string or bytes");
		return NULL;
		}
#else
	if ( ! PyString_Check($input) )
		{
		PyErr_SetString(PyExc_TypeError, "Expected a string or bytes");
		return NULL;
		}
	else
		{
		PyString_AsStringAndSize($input, &$1, &len);
		$2 = len;
		}
#endif
	%}

%typemap(arginit) (char* cidr, int size)
	{
	ascii$argnum = NULL;
	}

%typemap(freearg) (char* cidr, int size)
	{
	Py_XDECREF(ascii$argnum);
	}

%typecheck(SWIG_TYPECHECK_STRING) (char* cidr, int size)
	{
	// The typemap above will check types and throw a type error when
	// needed, so just let everything through.
	$1 = 1;
	}

#endif

typedef union _inx_addr {
        struct in_addr sin;
        struct in6_addr sin6;
} inx_addr;

class SubnetTree
{
public:
   SubnetTree(bool binary_lookup_mode = false);
   ~SubnetTree();

   PyObject* insert(const char *cidr, PyObject* data = 0);
   PyObject* insert(unsigned long subnet, unsigned short mask, PyObject* data = 0);

   PyObject* remove(const char *cidr);
   PyObject* remove(unsigned long subnet, unsigned short mask);

   PyObject* lookup(const char *cidr, int size) const;
   PyObject* lookup(unsigned long addr) const;

   PyObject* prefixes(bool ipv4_native = false, bool with_len = true) const;

   bool get_binary_lookup_mode();
   void set_binary_lookup_mode(bool binary_lookup_mode = true);

#ifndef SWIG
   bool operator[](const char* addr) const { return lookup(addr, strlen(addr)); }
   bool operator[](unsigned long addr) const { return lookup(addr); }
#else
   %extend {
       PyObject* __contains__(char *cidr, int size)
       {
           if ( ! cidr ) {
               PyErr_SetString(PyExc_TypeError, "index must be string");
               return 0;
           }

           PyObject* obj = self->lookup(cidr, size);
           if ( obj )
               {
               Py_DECREF(obj);
               }

           if ( PyErr_Occurred() )
               return 0;

           if ( obj != 0 )
               Py_RETURN_TRUE;
           else
               Py_RETURN_FALSE;
       }

       PyObject* __contains__(unsigned long addr)
       {
           PyObject* obj = self->lookup(addr);

           if ( obj )
               {
               Py_DECREF(obj);
               }

           if ( PyErr_Occurred() )
               return 0;

           if ( obj != 0 )
               Py_RETURN_TRUE;
           else
               Py_RETURN_FALSE;
       }

       PyObject* __getitem__(char *cidr, int size)
       {
           if ( ! cidr ) {
               PyErr_SetString(PyExc_TypeError, "index must be string");
               return 0;
           }

           PyObject* data = self->lookup(cidr, size);
           if ( ! data ) {
               PyErr_SetString(PyExc_KeyError, cidr);
               return 0;
           }

           return data;
       }

       PyObject*  __setitem__(const char* cidr, PyObject* data)
       {
           if ( ! cidr ) {
               PyErr_SetString(PyExc_TypeError, "index must be string");
               return 0;
           }

           if ( self->insert(cidr, data) )
               Py_RETURN_TRUE;
           else
               return 0;
       }

       PyObject* __delitem__(const char* cidr)
       {
           if ( ! cidr ) {
               PyErr_SetString(PyExc_TypeError, "index must be string");
               return 0;
           }

           if ( self->remove(cidr) )
               Py_RETURN_TRUE;
           else
               return 0;
       }

   }
#endif

private:
   PyObject* insert(int family, inx_addr subnet, unsigned short mask, PyObject * data);
   PyObject* remove(int family, inx_addr subnet, unsigned short mask);
   PyObject* lookup(int family, inx_addr subnet) const;

   patricia_tree_t* tree;
   bool binary_lookup_mode;
};
