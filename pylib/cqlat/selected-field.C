#include "lib.h"

namespace qlat
{  //

template <class M>
PyObject* mk_sfield_ctype()
{
  SelectedField<M>* pf = new SelectedField<M>();
  return py_convert((void*)pf);
}

template <class M>
PyObject* mk_sfield_fsel_ctype(const FieldSelection& fsel, const int multiplicity)
{
  SelectedField<M>* pf = new SelectedField<M>();
  SelectedField<M>& f = *pf;
  pqassert(multiplicity > 0);
  f.init(fsel, multiplicity);
  return py_convert((void*)pf);
}

template <class M>
PyObject* free_sfield_ctype(PyField& pf)
{
  SelectedField<M>& f = *(SelectedField<M>*)pf.cdata;
  delete &f;
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_sfield_ctype(PyField& pf_new, PyField& pf)
{
  SelectedField<M>& f_new = *(SelectedField<M>*)pf_new.cdata;
  SelectedField<M>& f = *(SelectedField<M>*)pf.cdata;
  f_new = f;
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_sfield_field_ctype(PyField& psf, PyField& pf,
                                 const FieldSelection& fsel)
{
  SelectedField<M>& sf = *(SelectedField<M>*)psf.cdata;
  const Field<M>& f = *(Field<M>*)pf.cdata;
  set_selected_field(sf, f, fsel);
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_sfield_sfield_ctype(PyField& psf, PyField& psf0,
                                  const FieldSelection& fsel,
                                  const FieldSelection& fsel0)
{
  SelectedField<M>& sf = *(SelectedField<M>*)psf.cdata;
  const SelectedField<M>& sf0 = *(SelectedField<M>*)psf0.cdata;
  set_selected_field(sf, sf0, fsel, fsel0);
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_add_sfield_ctype(PyField& pf_new, PyField& pf)
{
  SelectedField<M>& f_new = *(SelectedField<M>*)pf_new.cdata;
  SelectedField<M>& f = *(SelectedField<M>*)pf.cdata;
  f_new += f;
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_sub_sfield_ctype(PyField& pf_new, PyField& pf)
{
  SelectedField<M>& f_new = *(SelectedField<M>*)pf_new.cdata;
  SelectedField<M>& f = *(SelectedField<M>*)pf.cdata;
  f_new -= f;
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_mul_sfield_ctype(PyField& pf, const Complex& factor)
{
  SelectedField<M>& f = *(SelectedField<M>*)pf.cdata;
  f *= factor;
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_mul_sfield_ctype(PyField& pf, const double& factor)
{
  SelectedField<M>& f = *(SelectedField<M>*)pf.cdata;
  f *= factor;
  Py_RETURN_NONE;
}

template <class M>
PyObject* set_zero_sfield_ctype(PyField& pf)
{
  SelectedField<M>& f = *(SelectedField<M>*)pf.cdata;
  set_zero(f);
  Py_RETURN_NONE;
}

template <class M>
PyObject* get_n_elems_sfield_ctype(PyField& pf)
{
  SelectedField<M>& sf = *(SelectedField<M>*)pf.cdata;
  const long ret = sf.n_elems;
  return py_convert(ret);
}

template <class M>
PyObject* get_total_site_sfield_ctype(PyField& pf)
{
  SelectedField<M>& f = *(SelectedField<M>*)pf.cdata;
  const Coordinate ret = f.geo().total_site();
  return py_convert(ret);
}

template <class M>
PyObject* get_multiplicity_sfield_ctype(PyField& pf)
{
  SelectedField<M>& sf = *(SelectedField<M>*)pf.cdata;
  const long ret = sf.geo().multiplicity;
  return py_convert(ret);
}

template <class M>
PyObject* set_geo_sfield_ctype(Geometry& geo, PyField& pf)
{
  const SelectedField<M>& sf = *(SelectedField<M>*)pf.cdata;
  geo = sf.geo();
  Py_RETURN_NONE;
}

template <class M>
PyObject* qnorm_sfield_ctype(PyField& pf)
{
  SelectedField<M>& f = *(SelectedField<M>*)pf.cdata;
  const double ret = qnorm(f);
  return py_convert(ret);
}

template <class M>
PyObject* save_sfield_ctype(PyField& pf, const std::string& path,
                            const FieldSelection& fsel)
{
  const SelectedField<M>& f = *(SelectedField<M>*)pf.cdata;
  const long ret = write_selected_field(f, path, fsel);
  return py_convert(ret);
}

template <class M>
PyObject* load_sfield_ctype(PyField& pf, const std::string& path,
                            const FieldSelection& fsel)
{
  SelectedField<M>& f = *(SelectedField<M>*)pf.cdata;
  const long ret = read_selected_field(f, path, fsel);
  return py_convert(ret);
}

template <class M>
PyObject* convert_float_from_double_sfield_ctype(PyField& pf_new, PyField& pf)
{
  pqassert(pf_new.ctype == "float");
  SelectedField<float>& f_new = *(SelectedField<float>*)pf_new.cdata;
  const SelectedField<M>& f = *(SelectedField<M>*)pf.cdata;
  convert_field_float_from_double(f_new, f);
  Py_RETURN_NONE;
}

template <class M>
PyObject* convert_double_from_float_sfield_ctype(PyField& pf_new, PyField& pf)
{
  pqassert(pf.ctype == "float");
  const SelectedField<float>& f = *(SelectedField<float>*)pf.cdata;
  SelectedField<M>& f_new = *(SelectedField<M>*)pf_new.cdata;
  convert_field_double_from_float(f_new, f);
  Py_RETURN_NONE;
}

template <class M>
PyObject* to_from_endianness_sfield_ctype(PyField& pf,
                                          const std::string& endianness_tag)
{
  SelectedField<M>& f = *(SelectedField<M>*)pf.cdata;
  if ("big_32" == endianness_tag) {
    to_from_big_endian_32(get_data(f));
  } else if ("big_64" == endianness_tag) {
    to_from_big_endian_64(get_data(f));
  } else if ("little_32" == endianness_tag) {
    to_from_little_endian_32(get_data(f));
  } else if ("little_64" == endianness_tag) {
    to_from_little_endian_64(get_data(f));
  } else {
    pqassert(false);
  }
  Py_RETURN_NONE;
}

}  // namespace qlat

EXPORT(mk_sfield, {
  using namespace qlat;
  PyObject* p_ctype = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_ctype)) {
    return NULL;
  }
  std::string ctype;
  py_convert(ctype, p_ctype);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, mk_sfield_ctype, ctype);
  return p_ret;
});

EXPORT(mk_sfield_fsel, {
  using namespace qlat;
  PyObject* p_ctype = NULL;
  PyObject* p_fsel = NULL;
  int multiplicity = 0;
  if (!PyArg_ParseTuple(args, "OOi", &p_ctype, &p_fsel, &multiplicity)) {
    return NULL;
  }
  std::string ctype;
  py_convert(ctype, p_ctype);
  const FieldSelection& fsel = py_convert_type<FieldSelection>(p_fsel);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, mk_sfield_fsel_ctype, ctype, fsel, multiplicity);
  return p_ret;
});

EXPORT(free_sfield, {
  using namespace qlat;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, free_sfield_ctype, pf.ctype, pf);
  return p_ret;
});

EXPORT(set_sfield, {
  using namespace qlat;
  PyObject* p_sfield_new = NULL;
  PyObject* p_sfield = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_sfield_new, &p_sfield)) {
    return NULL;
  }
  PyField pf_new = py_convert_field(p_sfield_new);
  PyField pf = py_convert_field(p_sfield);
  pqassert(pf_new.ctype == pf.ctype);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, set_sfield_ctype, pf.ctype, pf_new, pf);
  return p_ret;
});

EXPORT(set_sfield_field, {
  using namespace qlat;
  PyObject* p_sfield = NULL;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_sfield, &p_field)) {
    return NULL;
  }
  PyField psf = py_convert_field(p_sfield);
  PyObject* p_fsel = PyObject_GetAttrString(p_sfield, "fsel");
  const FieldSelection& fsel = py_convert_type<FieldSelection>(p_fsel);
  PyField pf = py_convert_field(p_field);
  pqassert(psf.ctype == pf.ctype);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, set_sfield_field_ctype, pf.ctype, psf, pf, fsel);
  return p_ret;
});

EXPORT(set_sfield_sfield, {
  using namespace qlat;
  PyObject* p_sfield = NULL;
  PyObject* p_sfield0 = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_sfield, &p_sfield0)) {
    return NULL;
  }
  PyField psf = py_convert_field(p_sfield);
  PyObject* p_fsel = PyObject_GetAttrString(p_sfield, "fsel");
  const FieldSelection& fsel = py_convert_type<FieldSelection>(p_fsel);
  PyField psf0 = py_convert_field(p_sfield0);
  PyObject* p_fsel0 = PyObject_GetAttrString(p_sfield0, "fsel");
  const FieldSelection& fsel0 = py_convert_type<FieldSelection>(p_fsel0);
  pqassert(psf.ctype == psf0.ctype);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, set_sfield_sfield_ctype, psf0.ctype, psf, psf0, fsel, fsel0);
  return p_ret;
});

EXPORT(set_add_sfield, {
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
  FIELD_DISPATCH(p_ret, set_add_sfield_ctype, pf.ctype, pf_new, pf);
  return p_ret;
});

EXPORT(set_sub_sfield, {
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
  FIELD_DISPATCH(p_ret, set_sub_sfield_ctype, pf.ctype, pf_new, pf);
  return p_ret;
});

EXPORT(set_mul_double_sfield, {
  using namespace qlat;
  PyObject* p_field = NULL;
  double factor = 0.0;
  if (!PyArg_ParseTuple(args, "Od", &p_field, &factor)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, set_mul_sfield_ctype, pf.ctype, pf, factor);
  return p_ret;
});

EXPORT(set_zero_sfield, {
  using namespace qlat;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, set_zero_sfield_ctype, pf.ctype, pf);
  return p_ret;
});

EXPORT(get_n_elems_sfield, {
  using namespace qlat;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, get_n_elems_sfield_ctype, pf.ctype, pf);
  return p_ret;
});

EXPORT(get_total_site_sfield, {
  using namespace qlat;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, get_total_site_sfield_ctype, pf.ctype, pf);
  return p_ret;
});

EXPORT(get_multiplicity_sfield, {
  using namespace qlat;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, get_multiplicity_sfield_ctype, pf.ctype, pf);
  return p_ret;
});

EXPORT(set_geo_sfield, {
  using namespace qlat;
  PyObject* p_geo = NULL;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_geo, &p_field)) {
    return NULL;
  }
  Geometry& geo = py_convert_type<Geometry>(p_geo);
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, set_geo_sfield_ctype, pf.ctype, geo, pf);
  return p_ret;
});

EXPORT(qnorm_sfield, {
  using namespace qlat;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_field)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, qnorm_sfield_ctype, pf.ctype, pf);
  return p_ret;
});

EXPORT(save_sfield, {
  using namespace qlat;
  PyObject* p_field = NULL;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_field, &p_path)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_fsel = PyObject_GetAttrString(p_field, "fsel");
  const FieldSelection& fsel = py_convert_type<FieldSelection>(p_fsel);
  std::string path;
  py_convert(path, p_path);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, save_sfield_ctype, pf.ctype, pf, path, fsel);
  return p_ret;
});

EXPORT(load_sfield, {
  using namespace qlat;
  PyObject* p_field = NULL;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_field, &p_path)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  PyObject* p_fsel = PyObject_GetAttrString(p_field, "fsel");
  const FieldSelection& fsel = py_convert_type<FieldSelection>(p_fsel);
  std::string path;
  py_convert(path, p_path);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, load_sfield_ctype, pf.ctype, pf, path, fsel);
  return p_ret;
});

EXPORT(convert_float_from_double_sfield, {
  using namespace qlat;
  PyObject* p_field_new = NULL;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_field_new, &p_field)) {
    return NULL;
  }
  PyField pf_new = py_convert_field(p_field_new);
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, convert_float_from_double_sfield_ctype, pf.ctype,
                 pf_new, pf);
  return p_ret;
});

EXPORT(convert_double_from_float_sfield, {
  using namespace qlat;
  PyObject* p_field_new = NULL;
  PyObject* p_field = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_field_new, &p_field)) {
    return NULL;
  }
  PyField pf_new = py_convert_field(p_field_new);
  PyField pf = py_convert_field(p_field);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, convert_double_from_float_sfield_ctype, pf_new.ctype,
                 pf_new, pf);
  return p_ret;
});

EXPORT(to_from_endianness_sfield, {
  using namespace qlat;
  PyObject* p_field = NULL;
  PyObject* p_endianness_tag = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_field, &p_endianness_tag)) {
    return NULL;
  }
  PyField pf = py_convert_field(p_field);
  std::string endianness_tag;
  py_convert(endianness_tag, p_endianness_tag);
  PyObject* p_ret = NULL;
  FIELD_DISPATCH(p_ret, to_from_endianness_sfield_ctype, pf.ctype, pf,
                 endianness_tag);
  return p_ret;
});
