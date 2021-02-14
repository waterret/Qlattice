#include "lib.h"

namespace qlat
{  //

template <class M>
PyObject* mk_field_ctype(PyObject* p_geo, const int multiplicity)
{
  Field<M>* pf = new Field<M>();
  Field<M>& f = *pf;
  if (p_geo != NULL) {
    const Geometry& geo = py_convert_type<Geometry>(p_geo);
    if (multiplicity == 0) {
      f.init(geo);
    } else {
      f.init(geo, multiplicity);
    }
  }
  return py_convert((void*)pf);
}

template <class M>
PyObject* free_field_ctype(PyField& pf)
{
  Field<M>& f = *(Field<M>*)pf.cdata;
  delete &f;
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_field_ctype(PyField& pf_new, PyField& pf)
{
  Field<M>& f_new = *(Field<M>*)pf_new.cdata;
  Field<M>& f = *(Field<M>*)pf.cdata;
  f_new = f;
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_add_field_ctype(PyField& pf_new, PyField& pf)
{
  Field<M>& f_new = *(Field<M>*)pf_new.cdata;
  Field<M>& f = *(Field<M>*)pf.cdata;
  f_new += f;
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_sub_field_ctype(PyField& pf_new, PyField& pf)
{
  Field<M>& f_new = *(Field<M>*)pf_new.cdata;
  Field<M>& f = *(Field<M>*)pf.cdata;
  f_new -= f;
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_mul_field_ctype(PyField& pf, const Complex& factor)
{
  Field<M>& f = *(Field<M>*)pf.cdata;
  f *= factor;
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_mul_field_ctype(PyField& pf, const double& factor)
{
  Field<M>& f = *(Field<M>*)pf.cdata;
  f *= factor;
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_zero_field_ctype(PyField& pf)
{
  Field<M>& f = *(Field<M>*)pf.cdata;
  set_zero(f);
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_unit_field_ctype(PyField& pf, const Complex& coef = 1.0)
{
  Field<M>& f = *(Field<M>*)pf.cdata;
  set_unit(f, coef);
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_geo_field_ctype(Geometry& geo, PyField& pf)
{
  const Field<M>& f = *(Field<M>*)pf.cdata;
  geo = f.geo();
  Py_RETURN_NONE;
}

template <class M>
PyObject* get_mview_field_ctype(PyField& pf, PyObject* p_field)
{
  Field<M>& f = *(Field<M>*)pf.cdata;
  Vector<M> fv = get_data(f);
  PyObject* p_mview = py_convert(fv);
  pqassert(p_field != NULL);
  Py_INCREF(p_field);
  pqassert(!((PyMemoryViewObject*)p_mview)->mbuf->master.obj);
  ((PyMemoryViewObject*)p_mview)->mbuf->master.obj = p_field;
  return p_mview;
}

template <class M>
PyObject* qnorm_field_ctype(PyField& pf)
{
  Field<M>& f = *(Field<M>*)pf.cdata;
  const double ret = qnorm(f);
  return py_convert(ret);
}

template <class M>
PyObject* split_fields_field_ctype(std::vector<PyField>& pf_vec, PyField& pf)
{
  Field<M>& f = *(Field<M>*)pf.cdata;
  const int nf = pf_vec.size();
  std::vector<Handle<Field<M> > > vec(nf);
  for (int i = 0; i < nf; ++i) {
    pqassert(pf_vec[i].ctype == pf.ctype);
    vec[i].init(*(Field<M>*)pf_vec[i].cdata);
  }
  split_fields(vec, f);
  Py_RETURN_NONE;
}

template <class M>
PyObject* merge_fields_field_ctype(PyField& pf, const std::vector<PyField>& pf_vec)
{
  Field<M>& f = *(Field<M>*)pf.cdata;
  const int nf = pf_vec.size();
  std::vector<ConstHandle<Field<M> > > vec(nf);
  for (int i = 0; i < nf; ++i) {
    pqassert(pf_vec[i].ctype == pf.ctype);
    vec[i].init(*(Field<M>*)pf_vec[i].cdata);
  }
  merge_fields(f, vec);
  Py_RETURN_NONE;
}

}  // namespace qlat

EXPORT(mk_field, {
  using namespace qlat;
  PyObject* p_ctype = NULL;
  PyObject* p_geo = NULL;
  int multiplicity = 0;
  if (!PyArg_ParseTuple(args, "O|Oi", &p_ctype, &p_geo, &multiplicity)) {
    return NULL;
  }
  std::string ctype;
  py_convert(ctype, p_ctype);
  PyObject* pfield;
  FIELD_DISPATCH(pfield, mk_field_ctype, ctype, p_geo, multiplicity);
  return pfield;
});

EXPORT(free_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, free_field_ctype, pf.ctype, pf);
  return p_ret;
});

EXPORT(set_field, {
  using namespace qlat;
  PyObject* p_field_new = NULL;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_field_new, &p_field)) {
    return NULL;
  }
  PyField pf_new = py_convert_field(p_field_new);
  PyField pf = py_convert_field(p_field);
  pqassert(pf_new.ctype == pf.ctype);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, set_field_ctype, pf.ctype, pf_new, pf);
  return p_ret;
});

EXPORT(set_add_field, {
  using namespace qlat;
  PyObject* p_field_new = NULL;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_field_new, &p_field)) {
    return NULL;
  }
  PyField pf_new = py_convert_field(p_field_new);
  PyField pf = py_convert_field(p_field);
  pqassert(pf_new.ctype == pf.ctype);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, set_add_field_ctype, pf.ctype, pf_new, pf);
  return p_ret;
});

EXPORT(set_sub_field, {
  using namespace qlat;
  PyObject* p_field_new = NULL;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_field_new, &p_field)) {
    return NULL;
  }
  PyField pf_new = py_convert_field(p_field_new);
  PyField pf = py_convert_field(p_field);
  pqassert(pf_new.ctype == pf.ctype);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, set_sub_field_ctype, pf.ctype, pf_new, pf);
  return p_ret;
});

EXPORT(set_mul_double_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  double factor = 0.0;
  if (!PyArg_ParseTuple(args, "Od", &p_field, &factor)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, set_mul_field_ctype, pf.ctype, pf, factor);
  return p_ret;
});

EXPORT(set_zero_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, set_zero_field_ctype, pf.ctype, pf);
  return p_ret;
});

EXPORT(set_unit_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  Complex coef = 1.0;
  if (!PyArg_ParseTuple(args, "O|D", &p_field, &coef)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, set_unit_field_ctype, pf.ctype, pf, coef);
  return p_ret;
});

EXPORT(set_geo_field, {
  using namespace qlat;
  PyObject* p_geo = NULL;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_geo, &p_field)) {
    return NULL;
  }
  Geometry& geo = py_convert_type<Geometry>(p_geo);
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, set_geo_field_ctype, pf.ctype, geo, pf);
  return p_ret;
});

EXPORT(get_mview_field, {
  using namespace qlat;
  PyObject* p_field= NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, get_mview_field_ctype, pf.ctype, pf, p_field);
  return p_ret;
});

EXPORT(qnorm_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, qnorm_field_ctype, pf.ctype, pf);
  return p_ret;
});

EXPORT(split_fields_field, {
  using namespace qlat;
  PyObject* p_field_vec = NULL;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_field_vec, &p_field)) {
    return NULL;
  }
  std::vector<PyField> pf_vec;
  py_convert(pf_vec, p_field_vec);
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, split_fields_field_ctype, pf.ctype, pf_vec, pf);
  return p_ret;
});

EXPORT(merge_fields_field, {
  using namespace qlat;
  PyObject* p_field = NULL;
  PyObject* p_field_vec = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_field, &p_field_vec)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  std::vector<PyField> pf_vec;
  py_convert(pf_vec, p_field_vec);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, merge_fields_field_ctype, pf.ctype, pf, pf_vec);
  return p_ret;
});