#include "lib.h"

EXPORT(set_point_src_prop, {
  using namespace qlat;
  PyObject* p_prop = NULL;
  PyObject* p_geo = NULL;
  PyObject* p_xg = NULL;
  Complex value = 1.0;
  if (!PyArg_ParseTuple(args, "OOO|D", &p_prop, &p_geo, &p_xg, &value)) {
    return NULL;
  }
  Propagator4d& prop = py_convert_type<Propagator4d>(p_prop);
  const Geometry& geo = py_convert_type<Geometry>(p_geo);
  Coordinate xg;
  py_convert(xg, p_xg);
  set_point_src(prop, geo, xg, value);
  Py_RETURN_NONE;
});

EXPORT(set_wall_src_prop, {
  using namespace qlat;
  PyObject* p_prop = NULL;
  PyObject* p_geo = NULL;
  int tslice = -1;
  PyObject* p_lmom = NULL;
  if (!PyArg_ParseTuple(args, "OOi|O", &p_prop, &p_geo, &tslice, &p_lmom)) {
    return NULL;
  }
  Propagator4d& prop = py_convert_type<Propagator4d>(p_prop);
  const Geometry& geo = py_convert_type<Geometry>(p_geo);
  CoordinateD lmom;
  py_convert(lmom, p_lmom);
  set_wall_src(prop, geo, tslice, lmom);
  Py_RETURN_NONE;
});

EXPORT(free_invert_prop, {
  using namespace qlat;
  PyObject* p_prop_sol = NULL;
  PyObject* p_prop_src = NULL;
  double mass = 0.0;
  double m5 = 1.0;
  PyObject* p_momtwist = NULL;
  if (!PyArg_ParseTuple(args, "OOd|dO", &p_prop_sol, &p_prop_src, &mass, &m5,
                        &p_momtwist)) {
    return NULL;
  }
  Propagator4d& prop_sol = py_convert_type<Propagator4d>(p_prop_sol);
  const Propagator4d& prop_src = py_convert_type<Propagator4d>(p_prop_src);
  CoordinateD momtwist;
  py_convert(momtwist, p_momtwist);
  free_invert(prop_sol, prop_src, mass, m5, momtwist);
  Py_RETURN_NONE;
});

EXPORT(convert_wm_from_mspincolor_prop, {
  using namespace qlat;
  PyObject* p_prop_wm = NULL;
  PyObject* p_prop_msc = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_prop_wm, &p_prop_msc)) {
    return NULL;
  }
  Propagator4d& prop_wm = py_convert_type<Propagator4d>(p_prop_wm);
  const Propagator4d& prop_msc = py_convert_type<Propagator4d>(p_prop_msc);
  convert_wm_from_mspincolor(prop_wm, prop_msc);
  Py_RETURN_NONE;
});

EXPORT(convert_mspincolor_from_wm_prop, {
  using namespace qlat;
  PyObject* p_prop_msc = NULL;
  PyObject* p_prop_wm = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_prop_msc, &p_prop_wm)) {
    return NULL;
  }
  Propagator4d& prop_msc = py_convert_type<Propagator4d>(p_prop_msc);
  const Propagator4d& prop_wm = py_convert_type<Propagator4d>(p_prop_wm);
  convert_mspincolor_from_wm(prop_msc, prop_wm);
  Py_RETURN_NONE;
});
