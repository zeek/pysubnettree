extern "C" {
#include "Python.h"
#include "patricia.h"
}

#ifdef SWIG
// If a function is supposed to accept 4-byte tuples as packet by
// socket.inet_aton(), it needs to accept strings which contain 0s.
// Therefore, we need a size parameter.
%apply (char *STRING, int LENGTH) { (char *cidr, int size) };
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
