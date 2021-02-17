#include "lib.h"

namespace qlat
{  //

template <class M>
PyObject* mk_spfield_ctype(const long n_points, const int multiplicity)
{
  SelectedPoints<M>* pspfield = new SelectedPoints<M>();
  if (n_points >= 0) {
    SelectedPoints<M>& spfield = *pspfield;
    spfield.init(n_points, multiplicity);
  }
  return py_convert((void*)pspfield);
}

template <class M>
PyObject* free_spfield_ctype(PyField& pf)
{
  SelectedPoints<M>& f = *(SelectedPoints<M>*)pf.cdata;
  delete &f;
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_spfield_ctype(PyField& pf_new, PyField& pf)
{
  SelectedPoints<M>& f_new = *(SelectedPoints<M>*)pf_new.cdata;
  SelectedPoints<M>& f = *(SelectedPoints<M>*)pf.cdata;
  f_new = f;
  Py_RETURN_NONE;
}

}  // namespace qlat

EXPORT(mk_spfield, {
  using namespace qlat;
  PyObject* p_ctype = NULL;
  long n_points = -1;
  int multiplicity = 0;
  if (!PyArg_ParseTuple(args, "O|li", &p_ctype, &n_points, &multiplicity)) {
    return NULL;
  }
  std::string ctype;
  py_convert(ctype, p_ctype);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, mk_spfield_ctype, ctype, n_points, multiplicity);
  return p_ret;
});

EXPORT(free_spfield, {
  using namespace qlat;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, free_spfield_ctype, pf.ctype, pf);
  return p_ret;
});

EXPORT(set_spfield, {
  using namespace qlat;
  PyObject* p_spfield_new = NULL;
  PyObject* p_spfield = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_spfield_new, &p_spfield)) {
    return NULL;
  }
  PyField pf_new = py_convert_field(p_spfield_new);
  PyField pf = py_convert_field(p_spfield);
  pqassert(pf_new.ctype == pf.ctype);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, set_spfield_ctype, pf.ctype, pf_new, pf);
  return p_ret;
});