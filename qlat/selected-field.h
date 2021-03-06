#pragma once

#include <qlat/field.h>
#include <qlat/selected-points.h>

namespace qlat
{  //

inline void add_field_selection(FieldM<int64_t, 1>& f_rank,
                                const PointSelection& psel,
                                const long rank_psel = 1024L * 1024L * 1024L *
                                                       1024L * 1024L)
{
  TIMER_VERBOSE("add_field_selection(psel)");
  const Geometry& geo = f_rank.geo();
#pragma omp parallel for
  for (long i = 0; i < (long)psel.size(); ++i) {
    const Coordinate xl = geo.coordinate_l_from_g(psel[i]);
    if (geo.is_local(xl)) {
      int64_t& rank = f_rank.get_elem(xl);
      if (rank < 0) {
        rank = rank_psel;
      }
    }
  }
}

inline void mk_field_selection(FieldM<int64_t, 1>& f_rank,
                               const Coordinate& total_site,
                               const int64_t val = 0)
// select everything with val
// default val = 0 ; means selection everything
// val = -1 deselection everything
{
  TIMER_VERBOSE("mk_field_selection(f_rank,total_site)");
  Geometry geo;
  geo.init(total_site, 1);
  f_rank.init();
  f_rank.init(geo);
  qassert(f_rank.geo().is_only_local());
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    int64_t& rank = f_rank.get_elem(index);
    rank = val;
  }
}

inline void mk_field_selection(FieldM<int64_t, 1>& f_rank,
                               const Coordinate& total_site,
                               const std::vector<Coordinate>& xgs,
                               const long rank_xgs = 1024L * 1024L * 1024L *
                                                     1024L * 1024L)
{
  TIMER_VERBOSE("mk_field_selection(xgs)");
  mk_field_selection(f_rank, total_site, -1);
  add_field_selection(f_rank, xgs, rank_xgs);
}

inline void set_n_per_tslice(FieldM<int64_t, 1>& f_rank,
                             const long n_per_tslice_)
// will erase the rank information for points not selected
//
// if n_per_tslice_ == -1 then n_per_tslice = spatial_vol
//
// if n_per_tslice == spatial_vol than all points are selected regardless
// of rank.
//
// n_per_tslice is not enforced but only serve as an limit for f_rank
//
// 0 <= rank < n_per_tslice
{
  TIMER_VERBOSE("set_n_per_tslice");
  const Geometry& geo = f_rank.geo();
  qassert(geo.is_only_local());
  const Coordinate total_site = geo.total_site();
  const long spatial_vol = total_site[0] * total_site[1] * total_site[2];
  qassert(n_per_tslice_ == -1 or
          (0 <= n_per_tslice_ and n_per_tslice_ <= spatial_vol));
  const long n_per_tslice = n_per_tslice_ == -1 ? spatial_vol : n_per_tslice_;
  for (long index = 0; index < geo.local_volume(); ++index) {
    int64_t& rank = f_rank.get_elem(index);
    if (n_per_tslice == spatial_vol and rank < 0) {
      rank = spatial_vol - 1;
    } else if (not(0 <= rank and rank < n_per_tslice)) {
      rank = -1;
    }
  }
}

struct FieldSelection {
  FieldM<int64_t, 1>
      f_rank;  // rank when the points being selected (-1 if not selected)
  //
  long n_per_tslice;  // num points per time slice (not enfored and should work
                      // properly if not true)
  double prob;        // (double)n_per_tslice / (double)spatial_vol
  //
  FieldM<long, 1>
      f_local_idx;  // idx of points on this node (-1 if not selected)
  long n_elems;     // num points of this node
  //
  vector<int64_t> ranks;  // rank of the selected points
  vector<long> indices;   // local indices of selected points
  //
  void init()
  {
    f_rank.init();
    n_per_tslice = 0;
    prob = 0.0;
    f_local_idx.init();
    n_elems = 0;
    ranks.init();
    indices.init();
  }
  //
  FieldSelection() { init(); }
};

inline void update_field_selection(FieldSelection& fsel)
// update fsel based only on f_rank
// do not touch n_per_tslice and prob at all
{
  TIMER_VERBOSE("update_field_selection");
  const Geometry& geo = fsel.f_rank.geo();
  qassert(geo.is_only_local());
  fsel.f_local_idx.init();
  fsel.f_local_idx.init(geo);
  long n_elems = 0;
  for (long index = 0; index < geo.local_volume(); ++index) {
    const int64_t& rank = fsel.f_rank.get_elem(index);
    long& idx = fsel.f_local_idx.get_elem(index);
    if (0 <= rank) {
      idx = n_elems;
      n_elems += 1;
    } else {
      idx = -1;
    }
  }
  fsel.n_elems = n_elems;
  fsel.ranks.resize(fsel.n_elems);
  fsel.indices.resize(fsel.n_elems);
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const long idx = fsel.f_local_idx.get_elems_const(index)[0];
    if (idx >= 0) {
      const long rank = fsel.f_rank.get_elem(index);
      fsel.ranks[idx] = rank;
      fsel.indices[idx] = index;
    }
  }
}

inline void update_field_selection(FieldSelection& fsel,
                                   const long n_per_tslice_)
// only adjust parameter, do not change contents
{
  const Geometry& geo = fsel.f_rank.geo();
  qassert(geo.is_only_local());
  const Coordinate total_site = geo.total_site();
  const long spatial_vol = total_site[0] * total_site[1] * total_site[2];
  qassert(n_per_tslice_ == -1 or
          (0 <= n_per_tslice_ and n_per_tslice_ <= spatial_vol));
  fsel.n_per_tslice = n_per_tslice_ == -1 ? spatial_vol : n_per_tslice_;
  fsel.prob = (double)fsel.n_per_tslice / (double)spatial_vol;
}

inline void set_field_selection(FieldSelection& fsel,
                                const FieldM<int64_t, 1>& f_rank,
                                const long n_per_tslice_ = 0,
                                const bool is_limit_on_rank = false)
// call set_n_per_tslice if is_limit_on_rank=true
// otherwise will strictly follow f_rank without constaint of n_per_tslice
{
  TIMER_VERBOSE("set_field_selection(fsel,f_rank,n_per_tslice)");
  fsel.init();
  fsel.f_rank = f_rank;
  if (is_limit_on_rank) {
    set_n_per_tslice(fsel.f_rank, n_per_tslice_);
  }
  update_field_selection(fsel);
  update_field_selection(fsel, n_per_tslice_);
}

inline void set_field_selection(FieldSelection& fsel,
                                const Coordinate& total_site)
{
  TIMER_VERBOSE("set_field_selection(fsel,total_site)");
  fsel.init();
  mk_field_selection(fsel.f_rank, total_site);
  update_field_selection(fsel);
  update_field_selection(fsel, -1);  // select all points
}

template <class M>
struct SelectedField {
  // Avoid copy constructor when possible
  // (it is likely not be what you think it is)
  //
  bool initialized;
  long n_elems;
  box<Geometry> geo;
  vector<M> field;
  //
  void init()
  {
    initialized = false;
    geo.init();
    field.init();
  }
  void init(const Geometry& geo_, const long n_elems_, const int multiplicity)
  {
    if (initialized) {
      qassert(geo() == geo_remult(geo_, multiplicity));
      qassert(n_elems == n_elems_);
      qassert((long)field.size() == n_elems * multiplicity);
    } else {
      init();
      initialized = true;
      geo.set(geo_remult(geo_, multiplicity));
      n_elems = n_elems_;
      field.resize(n_elems * multiplicity);
      if (1 == get_field_init()) {
        set_zero(*this);
      } else if (2 == get_field_init()) {
        set_u_rand_float(get_data(field), RngState(show(get_time())));
      } else {
        qassert(0 == get_field_init());
      }
    }
  }
  void init(const FieldSelection& fsel, const int multiplicity)
  {
    init(fsel.f_rank.geo(), fsel.n_elems, multiplicity);
  }
  //
  SelectedField() { init(); }
  //
  M& get_elem(const long& idx)
  {
    qassert(1 == geo().multiplicity);
    return field[idx];
  }
  const M& get_elem(const long& idx) const
  {
    qassert(1 == geo().multiplicity);
    return field[idx];
  }
  //
  Vector<M> get_elems(const long idx)
  // qassert(geo().is_only_local())
  {
    return Vector<M>(&field[idx * geo().multiplicity], geo().multiplicity);
  }
  Vector<M> get_elems_const(const long idx) const
  // Be cautious about the const property
  // 改不改靠自觉
  // qassert(geo().is_only_local())
  {
    return Vector<M>(&field[idx * geo().multiplicity], geo().multiplicity);
  }
};

template <class M>
bool is_initialized(const SelectedField<M>& sf)
{
  return sf.initialized;
}

template <class M>
bool is_consistent(const SelectedField<M>& sf, const FieldSelection& fsel)
{
  return sf.initialized and sf.n_elems == fsel.n_elems and
         geo_remult(sf.geo(), 1) == fsel.f_local_idx.geo() and
         fsel.f_rank.geo() == fsel.f_local_idx.geo() and
         (long) sf.field.size() == sf.n_elems * (long)sf.geo().multiplicity and
         fsel.f_local_idx.geo().is_only_local();
}

template <class M>
void qswap(SelectedField<M>& f1, SelectedField<M>& f2)
{
  std::swap(f1.initialized, f2.initialized);
  std::swap(f1.n_elems, f2.n_elems);
  qswap(f1.geo, f2.geo);
  qswap(f1.field, f2.field);
}

template <class M>
Vector<M> get_data(const SelectedField<M>& sf)
{
  return get_data(sf.field);
}

template <class M>
void set_zero(SelectedField<M>& sf)
{
  TIMER("set_zero(SelectedField)");
  set_zero(get_data(sf));
}

template <class M>
double qnorm(const SelectedField<M>& sf)
{
  double s = qnorm(sf.field);
  glb_sum(s);
  return s;
}

template <class M>
const SelectedField<M>& operator+=(SelectedField<M>& f,
                                   const SelectedField<M>& f1)
{
  TIMER("sel_field_operator+=");
  if (not f.initialized) {
    f = f1;
  } else {
    qassert(f1.initialized);
    qassert(is_matching_geo_mult(f.geo(), f1.geo()));
    qassert(f.field.size() == f1.field.size());
#pragma omp parallel for
    for (long k = 0; k < (long)f.field.size(); ++k) {
      f.field[k] += f1.field[k];
    }
  }
  return f;
}

template <class M>
const SelectedField<M>& operator-=(SelectedField<M>& f,
                                   const SelectedField<M>& f1)
{
  TIMER("sel_field_operator-=");
  if (not f.initialized) {
    f.init(f1.geo(), f1.n_elems, f1.geo().multiplicity);
    set_zero(f);
    f -= f1;
  } else {
    qassert(f1.initialized);
    qassert(is_matching_geo_mult(f.geo(), f1.geo()));
    qassert(f.field.size() == f1.field.size());
#pragma omp parallel for
    for (long k = 0; k < (long)f.field.size(); ++k) {
      f.field[k] -= f1.field[k];
    }
  }
  return f;
}

template <class M>
const SelectedField<M>& operator*=(SelectedField<M>& f, const double factor)
{
  TIMER("sel_field_operator*=(F,D)");
  qassert(f.initialized);
#pragma omp parallel for
  for (long k = 0; k < (long)f.field.size(); ++k) {
    f.field[k] *= factor;
  }
  return f;
}

template <class M>
const SelectedField<M>& operator*=(SelectedField<M>& f, const Complex factor)
{
  TIMER("sel_field_operator*=(F,C)");
  qassert(f.initialized);
#pragma omp parallel for
  for (long k = 0; k < (long)f.field.size(); ++k) {
    f.field[k] *= factor;
  }
  return f;
}

inline void set_selected_gindex(SelectedField<long>& sfgi,
                                const FieldSelection& fsel)
{
  TIMER_VERBOSE("set_selected_gindex");
  const Geometry& geo = fsel.f_rank.geo();
  const Coordinate total_site = geo.total_site();
  sfgi.init(fsel, 1);
#pragma omp parallel for
  for (long idx = 0; idx < fsel.n_elems; ++idx) {
    const long index = fsel.indices[idx];
    const Coordinate xl = geo.coordinate_from_index(index);
    const Coordinate xg = geo.coordinate_g_from_l(xl);
    const long gindex = index_from_coordinate(xg, total_site);
    sfgi.get_elem(idx) = gindex;
  }
}

template <class M>
void only_keep_selected_points(Field<M>& f, const FieldSelection& fsel)
{
  TIMER("only_keep_selected_points");
  qassert(f.geo().is_only_local());
  qassert(fsel.f_local_idx.geo().is_only_local());
  qassert(geo_remult(f.geo()) == geo_remult(fsel.f_local_idx.geo()));
  const Geometry& geo = f.geo();
  const FieldM<long, 1>& f_local_idx = fsel.f_local_idx;
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const long idx = f_local_idx.get_elems_const(index)[0];
    if (idx < 0) {
      qassert(idx == -1);
      Vector<M> fv = f.get_elems(index);
      set_zero(fv);
    }
  }
}

template <class M>
void set_selected_field(SelectedField<M>& sf, const Field<M>& f,
                        const FieldSelection& fsel)
{
  TIMER("set_selected_field(sf,f,fsel)");
  qassert(f.geo().is_only_local());
  qassert(fsel.f_local_idx.geo().is_only_local());
  qassert(geo_remult(f.geo()) == fsel.f_local_idx.geo());
  const Geometry& geo = f.geo();
  const int multiplicity = geo.multiplicity;
  sf.init(fsel, multiplicity);
#pragma omp parallel for
  for (long idx = 0; idx < fsel.n_elems; ++idx) {
    const long index = fsel.indices[idx];
    const Vector<M> fv = f.get_elems_const(index);
    Vector<M> sfv = sf.get_elems(idx);
    for (int m = 0; m < multiplicity; ++m) {
      sfv[m] = fv[m];
    }
  }
}

template <class M>
void set_selected_field(SelectedField<M>& sf, const SelectedField<M>& sf0,
                        const FieldSelection& fsel, const FieldSelection& fsel0)
{
  TIMER("set_selected_field(sf,sf0,fsel,fsel0)");
  qassert(sf0.geo().is_only_local());
  qassert(fsel.f_local_idx.geo().is_only_local());
  qassert(fsel0.f_local_idx.geo().is_only_local());
  qassert(geo_remult(sf0.geo()) == fsel0.f_local_idx.geo());
  qassert(geo_remult(sf0.geo()) == fsel.f_local_idx.geo());
  const Geometry& geo = sf0.geo();
  const int multiplicity = geo.multiplicity;
  sf.init(fsel, multiplicity);
#pragma omp parallel for
  for (long idx = 0; idx < fsel.n_elems; ++idx) {
    const long index = fsel.indices[idx];
    const long idx0 = fsel0.f_local_idx.get_elem(index);
    Vector<M> sfv = sf.get_elems(idx);
    qassert(idx0 >= 0);
    const Vector<M> fv = sf0.get_elems_const(idx0);
    for (int m = 0; m < multiplicity; ++m) {
      sfv[m] = fv[m];
    }
  }
}

template <class M>
void set_selected_points(SelectedPoints<M>& sp, const SelectedField<M>& sf,
                         const PointSelection& psel, const FieldSelection& fsel)
{
  TIMER("set_selected_points(sp,sf,psel,fsel)");
  const Geometry& geo = sf.geo();
  qassert(is_consistent(sf, fsel));
  const long n_points = psel.size();
  sp.init(psel, geo.multiplicity);
  set_zero(sp.points);
#pragma omp parallel for
  for (long idx = 0; idx < n_points; ++idx) {
    const Coordinate& xg = psel[idx];
    const Coordinate xl = geo.coordinate_l_from_g(xg);
    if (geo.is_local(xl)) {
      const long sf_idx = fsel.f_local_idx.get_elem(xl);
      qassert(0 <= sf_idx and sf_idx < sf.n_elems);
      const Vector<M> fv = sf.get_elems_const(sf_idx);
      Vector<M> spv = sp.get_elems(idx);
      for (int m = 0; m < geo.multiplicity; ++m) {
        spv[m] = fv[m];
      }
    }
  }
  glb_sum_byte_vec(get_data(sp.points));
}

template <class M>
void set_field_selected(Field<M>& f, const SelectedField<M>& sf,
                        const FieldSelection& fsel)
{
  TIMER("set_field_selected");
  qassert(sf.geo().is_only_local());
  qassert(fsel.f_local_idx.geo().is_only_local());
  qassert(geo_remult(sf.geo()) == fsel.f_local_idx.geo());
  const Geometry& geo = sf.geo();
  f.init();
  f.init(sf.geo());
  set_zero(f);
  const int multiplicity = geo.multiplicity;
#pragma omp parallel for
  for (long idx = 0; idx < fsel.n_elems; ++idx) {
    const long index = fsel.indices[idx];
    Vector<M> fv = f.get_elems(index);
    const Vector<M> sfv = sf.get_elems_const(idx);
    for (int m = 0; m < multiplicity; ++m) {
      fv[m] = sfv[m];
    }
  }
}

template <class M>
bool is_consistent(const SelectedPoints<M>& sp, const SelectedField<M>& sf,
                   const PointSelection& psel, const FieldSelection& fsel)
{
  TIMER("is_consistent(sp,sf)");
  qassert(is_consistent(sp, psel));
  qassert(is_consistent(sf, fsel));
  const Geometry& geo = sf.geo();
  const long n_points = psel.size();
  double qnorm_diff = 0.0;
  qfor(idx, n_points, {
    const Coordinate& xg = psel[idx];
    const Coordinate xl = geo.coordinate_l_from_g(xg);
    if (geo.is_local(xl)) {
      const long sf_idx = fsel.f_local_idx.get_elem(xl);
      if (sf_idx >= 0) {
        const Vector<M> fv = sf.get_elems_const(sf_idx);
        const Vector<M> spv = sp.get_elems_const(idx);
        for (int m = 0; m < geo.multiplicity; ++m) {
          qnorm_diff += qnorm(spv[m] - fv[m]);
        }
      }
    }
  });
  glb_sum(qnorm_diff);
  return qnorm_diff == 0.0;
}

template <class M>
void acc_field(Field<M>& f, const Complex coef, const SelectedField<M>& sf,
               const FieldSelection& fsel)
{
  TIMER("acc_field(f,coef,sf,fsel)");
  const Geometry& geo = fsel.f_rank.geo();
  const int multiplicity = sf.geo().multiplicity;
  if (not is_initialized(f)) {
    f.init(geo_remult(geo, multiplicity));
    set_zero(f);
  }
  qassert(multiplicity == f.geo().multiplicity);
  qassert(sf.n_elems == fsel.n_elems);
#pragma omp parallel for
  for (long idx = 0; idx < fsel.n_elems; ++idx) {
    const long index = fsel.indices[idx];
    const Coordinate xl = geo.coordinate_from_index(index);
    Vector<M> fv = f.get_elems(xl);
    const Vector<M> sfv = sf.get_elems_const(idx);
    for (int m = 0; m < multiplicity; ++m) {
      fv[m] += coef * sfv[m];
    }
  }
}

// old code

template <class M>
void set_selected_field_slow(SelectedField<M>& sf, const Field<M>& f,
                             const FieldSelection& fsel)
{
  TIMER("set_selected_field_slow");
  qassert(f.geo().is_only_local());
  qassert(fsel.f_local_idx.geo().is_only_local());
  qassert(geo_remult(f.geo()) == fsel.f_local_idx.geo());
  const Geometry& geo = f.geo();
  const int multiplicity = geo.multiplicity;
  sf.init(fsel, multiplicity);
  const FieldM<long, 1>& f_local_idx = fsel.f_local_idx;
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const long idx = f_local_idx.get_elems_const(index)[0];
    if (idx >= 0) {
      qassert(idx < fsel.n_elems);
      const long offset = idx * multiplicity;
      const Vector<M> fv = f.get_elems_const(index);
      for (int m = 0; m < multiplicity; ++m) {
        sf.field[offset + m] = fv[m];
      }
    }
  }
}

template <class M>
void set_field_selected_slow(Field<M>& f, const SelectedField<M>& sf,
                             const FieldSelection& fsel)
{
  TIMER("set_field_selected_slow");
  qassert(sf.geo().is_only_local());
  qassert(fsel.f_local_idx.geo().is_only_local());
  qassert(geo_remult(sf.geo()) == fsel.f_local_idx.geo());
  const Geometry& geo = sf.geo();
  f.init();
  f.init(sf.geo());
  set_zero(f);
  const int multiplicity = geo.multiplicity;
  const FieldM<long, 1>& f_local_idx = fsel.f_local_idx;
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const long idx = f_local_idx.get_elems_const(index)[0];
    if (idx >= 0) {
      qassert(idx < fsel.n_elems);
      const long offset = idx * multiplicity;
      Vector<M> fv = f.get_elems(index);
      for (int m = 0; m < multiplicity; ++m) {
        fv[m] = sf.field[offset + m];
      }
    }
  }
}

}  // namespace qlat
