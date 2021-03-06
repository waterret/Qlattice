#include "lib.h"

namespace qlat
{  //

template <class M>
PyObject* assign_as_complex_field_ctype(PyField& pf, PyField& pf1)
{
  Field<Complex>& f = *(Field<Complex>*)pf.cdata;
  const Field<M>& f1 = *(Field<M>*)pf1.cdata;
  assign(f, f1);
  Py_RETURN_NONE;
}

template <class M>
PyObject* assign_from_complex_field_ctype(PyField& pf, PyField& pf1)
{
  Field<M>& f = *(Field<M>*)pf.cdata;
  const Field<Complex>& f1 = *(Field<Complex>*)pf1.cdata;
  assign(f, f1);
  Py_RETURN_NONE;
}

template <class M>
PyObject* get_elems_field_ctype(PyField& pf, const Coordinate& xg)
{
  const Field<M>& f = *(Field<M>*)pf.cdata;
  return py_convert(field_get_elems(f, xg));
}

template <class M>
PyObject* get_elem_field_ctype(PyField& pf, const Coordinate& xg, const int m)
{
  const Field<M>& f = *(Field<M>*)pf.cdata;
  if (m >= 0) {
    return py_convert(field_get_elem(f, xg, m));
  } else {
    return py_convert(field_get_elem(f, xg));
  }
}

template <class M>
PyObject* glb_sum_double_field_ctype(PyField& pf)
{
  const Field<M>& f = *(Field<M>*)pf.cdata;
  return py_convert(field_glb_sum_double(f));
}

template <class M>
PyObject* glb_sum_long_field_ctype(PyField& pf)
{
  const Field<M>& f = *(Field<M>*)pf.cdata;
  return py_convert(field_glb_sum_long(f));
}

template <class M>
PyObject* glb_sum_tslice_double_field_ctype(PyField& pf)
{
  const Field<M>& f = *(Field<M>*)pf.cdata;
  return py_convert(field_glb_sum_tslice_double(f));
}

template <class M>
PyObject* glb_sum_tslice_long_field_ctype(PyField& pf)
{
  const Field<M>& f = *(Field<M>*)pf.cdata;
  return py_convert(field_glb_sum_tslice_double(f));
}

}  // namespace qlat

EXPORT(set_phase_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  PyObject* p_lmom = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_field, &p_lmom)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  CoordinateD lmom;
  py_convert(lmom, p_lmom);
  pqassert(pf.ctype == "Complex");
  FieldM<Complex, 1>& f = *(FieldM<Complex, 1>*)pf.cdata;
  pqassert(f.geo().multiplicity == 1);
  set_phase_field(f, lmom);
  Py_RETURN_NONE;
});

EXPORT(assign_as_complex_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  PyObject* p_field1 = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_field, &p_field1)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  pqassert(pf.ctype == "Complex");
  PyField pf1 = py_convert_field(p_field1);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, assign_as_complex_field_ctype, pf1.ctype, pf, pf1);
  return p_ret;
});

EXPORT(assign_from_complex_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  PyObject* p_field1 = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_field, &p_field1)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyField pf1 = py_convert_field(p_field1);
  pqassert(pf1.ctype == "Complex");
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, assign_from_complex_field_ctype, pf.ctype, pf, pf1);
  return p_ret;
});

EXPORT(get_elems_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  PyObject* p_xg = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_field, &p_xg)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  Coordinate xg;
  py_convert(xg, p_xg);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, get_elems_field_ctype, pf.ctype, pf, xg);
  return p_ret;
});

EXPORT(get_elem_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  PyObject* p_xg = NULL;
  long m = -1;
  if (!PyArg_ParseTuple(args, "OO|l", &p_field, &p_xg, &m)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  Coordinate xg;
  py_convert(xg, p_xg);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, get_elem_field_ctype, pf.ctype, pf, xg, m);
  return p_ret;
});

EXPORT(glb_sum_double_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, glb_sum_double_field_ctype, pf.ctype, pf);
  return p_ret;
});

EXPORT(glb_sum_long_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, glb_sum_long_field_ctype, pf.ctype, pf);
  return p_ret;
});

EXPORT(glb_sum_tslice_double_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, glb_sum_tslice_double_field_ctype, pf.ctype, pf);
  return p_ret;
});

EXPORT(glb_sum_tslice_long_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, glb_sum_tslice_long_field_ctype, pf.ctype, pf);
  return p_ret;
});
