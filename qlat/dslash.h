#pragma once

#include <qlat/qcd.h>
#include <qlat/qcd-utils.h>
#include <qlat/fermion-action.h>

QLAT_START_NAMESPACE

inline bool& is_checking_inverse()
  // qlat parameter
{
  static bool b = false;
  return b;
}

struct LowModes
{
  std::vector<double> eigen_values;
  CompressedEigenSystemInfo cesi;
  CompressedEigenSystemBases cesb;
  CompressedEigenSystemCoefs cesc;
};

inline void load_low_modes(LowModes& lm, const std::string& path)
{
  TIMER_VERBOSE("load_low_modes");
  load_compressed_eigen_vectors(lm.eigen_values, lm.cesi, lm.cesb, lm.cesc, path);
}

inline void load_or_compute_low_modes(LowModes& lm, const std::string& path,
    const GaugeField& gf, const FermionAction& fa, const LancArg& la)
  // TODO: currently only load low modes
{
  TIMER_VERBOSE("load_or_compute_low_modes");
  load_low_modes(lm, path);
}

inline void deflate(HalfVector& hv_out, const HalfVector& hv_in, const LowModes& lm)
{
  TIMER("deflate(hv,hv,lm)");
  const int ls = lm.cesi.ls;
  const long block_size = lm.cesb.block_vol_eo * ls * HalfVector::c_size;
  const long n_basis = lm.cesb.n_basis;
  const long n_vec = lm.cesc.n_vec;
  qassert(n_vec == lm.eigen_values.size());
  BlockedHalfVector bhv;
  convert_half_vector(bhv, hv_in, lm.cesi.block_site);
  const Geometry geo = geo_reform(bhv.geo, n_basis);
  Field<Complex> chv, phv;
  chv.init(geo);
  set_zero(chv);
  phv.init(geo_remult(geo, n_vec));
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    const Vector<ComplexF> vb = bhv.get_elems_const(xl);
    const Vector<ComplexF> vbs = lm.cesb.get_elems_const(xl);
    const Vector<ComplexF> vcs = lm.cesc.get_elems_const(xl);
    // project to coarse grid
    Vector<Complex> vc = chv.get_elems(xl);
#pragma omp parallel for
    for (int j = 0; j < n_basis; ++j) {
      Complex s = 0.0;
      for (long k = 0; k < block_size; ++k) {
        s += std::conj(vbs[j * block_size + k]) * vb[k];
      }
      vc[j] = s;
    }
    // compute inner products
    Vector<Complex> vp = phv.get_elems(xl);
#pragma omp parallel for
    for (int i = 0; i < n_vec; ++i) {
      Complex s = 0.0;
      for (int j = 0; j < n_basis; ++j) {
        s += (Complex)std::conj(vcs[i * n_basis + j]) * vc[j];
      }
      vp[i] = s;
    }
  }
  // glb sum inner products
  std::vector<Complex> phv_sum(n_vec, 0.0);
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    Vector<Complex> vp = phv.get_elems(xl);
#pragma omp parallel for
    for (int i = 0; i < n_vec; ++i) {
      phv_sum[i] += vp[i];
    }
  }
  phv.init();
  glb_sum_double_vec(get_data(phv_sum));
  // scale by eigen values
#pragma omp parallel for
  for (int i = 0; i < n_vec; ++i) {
    phv_sum[i] /= lm.eigen_values[i];
  }
  // producing coarse space vector
  set_zero(chv);
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    const Vector<ComplexF> vbs = lm.cesb.get_elems_const(xl);
    const Vector<ComplexF> vcs = lm.cesc.get_elems_const(xl);
    // compute inner products
    Vector<Complex> vc = chv.get_elems(xl);
    for (int i = 0; i < n_vec; ++i) {
#pragma omp parallel for
      for (int j = 0; j < n_basis; ++j) {
        vc[j] += (Complex)(vcs[i * n_basis + j]) * phv_sum[i];
      }
    }
    // project to fine grid
    Vector<ComplexF> vb = bhv.get_elems(xl);
    for (int j = 0; j < n_basis; ++j) {
#pragma omp parallel for
      for (long k = 0; k < block_size; ++k) {
        vb[k] += vbs[j * block_size + k] * (ComplexF)vc[j];
      }
    }
  }
  convert_half_vector(hv_out, bhv);
}

inline deflate(FermionField5d& out, const FermionField5d& in, const LowModes& lm)
{
  TIMER_VERBOSE("deflate(5d,5d,lm)");
  const Geometry& geo = geo_resize(in.geo);
  qassert(geo.eo == 1 or geo.eo == 2);
  const int ls = geo.multiplicity;
  qassert(ls == lm.cesi.ls);
  HalfVector hv;
  init_half_vector(hv, geo, ls);
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    qassert((xl[0] + xl[1] + xl[2] + xl[3]) % 2 == 2 - geo.eo);
    Vector<ComplexF> vhv = hv.get_elems(index);
    const Vector<WilsonVector> vin = in.get_elems_const(index);
    qassert(vhv.size() == vin.size() * sizeof(WilsonVector) / sizeof(Complex));
    const Vector<Complex> vff((const Complex*)vin.data(), vhv.size());
    for (int m = 0; m < vhv.size(); ++m) {
      vhv[m] = vff[m];
    }
  }
  deflate(hv, hv, lm);
  if (is_initialized(out) and out.geo.eo == 3 - geo.eo) {
    out.geo.eo = geo.eo;
  }
  out.init(geo);
  qassert(out.geo.eo == geo.eo);
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    qassert((xl[0] + xl[1] + xl[2] + xl[3]) % 2 == 2 - geo.eo);
    const Vector<ComplexF> vhv = hv.get_elems(index);
    Vector<WilsonVector> vout = out.get_elems(index);
    qassert(vhv.size() == vout.size() * sizeof(WilsonVector) / sizeof(Complex));
    Vector<Complex> vff((Complex*)vout.data(), vhv.size());
    for (int m = 0; m < vhv.size(); ++m) {
      vff[m] = vhv[m];
    }
  }
}

struct InverterParams
{
  double stop_rsd;
  long max_num_iter;
  long max_mixed_precision_cycle;
  //
  void init()
  {
    stop_rsd = 1.0e-8;
    max_num_iter = 50000;
    max_mixed_precision_cycle = 100;
  }
  //
  InverterParams()
  {
    init();
  }
};

struct InverterDomainWall
{
  Geometry geo;
  FermionAction fa;
  GaugeField gf;
  InverterParams ip;
  ConstHandle<LowModes> lm;
  //
  InverterDomainWall()
  {
    init();
  }
  //
  void init()
  {
    ip.init();
  }
  //
  void setup()
  {
    TIMER_VERBOSE("Inv::setup");
    lm.init();
  }
  void setup(const GaugeField& gf_, const FermionAction& fa_)
  {
    geo = geo_reform(gf_.geo);
    gf.init();
    set_left_expanded_gauge_field(gf, gf_);
    fa = fa_;
    setup();
  }
  void setup(const GaugeField& gf_, const FermionAction& fa_, const LowModes& lm_)
  {
    setup(gf_, fa_);
    lm.init(lm_);
  }
  //
  double& stop_rsd()
  {
    return ip.stop_rsd;
  }
  const double& stop_rsd() const
  {
    return ip.stop_rsd;
  }
  //
  long& max_num_iter()
  {
    return ip.max_num_iter;
  }
  const long& max_num_iter() const
  {
    return ip.max_num_iter;
  }
  //
  long& max_mixed_precision_cycle()
  {
    return ip.max_mixed_precision_cycle;
  }
  const long& max_mixed_precision_cycle() const
  {
    return ip.max_mixed_precision_cycle;
  }
};

inline void setup_inverter(InverterDomainWall& inv)
{
  inv.setup();
}

inline void setup_inverter(InverterDomainWall& inv, const GaugeField& gf, const FermionAction& fa)
{
  inv.setup(gf, fa);
}

inline void setup_inverter(InverterDomainWall& inv, const GaugeField& gf, const FermionAction& fa, const LowModes& lm)
{
  inv.setup(gf, fa, lm);
}

inline void multiply_m_dwf_no_comm(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  TIMER("multiply_m_dwf_no_comm(5d,5d,Inv)");
  const Geometry geo = geo_resize(in.geo);
  qassert(is_matching_geo(inv.geo, geo));
  out.init(geo);
  set_zero(out);
  const FermionAction& fa = inv.fa;
  const GaugeField& gf = inv.gf;
  qassert(in.geo.multiplicity == fa.ls);
  qassert(out.geo.multiplicity == fa.ls);
  qassert(fa.mobius_scale == 1.0);
  qassert(fa.bs.size() == fa.ls);
  qassert(fa.cs.size() == fa.ls);
  const std::array<SpinMatrix,4>& gammas = SpinMatrixConstants::get_cps_gammas();
  const SpinMatrix& gamma5 = SpinMatrixConstants::get_gamma5();
  const SpinMatrix& unit = SpinMatrixConstants::get_unit();
  const SpinMatrix p_p = 0.5 * (unit + gamma5);
  const SpinMatrix p_m = 0.5 * (unit - gamma5);
  std::array<SpinMatrix,4> p_mu_p;
  std::array<SpinMatrix,4> p_mu_m;
  for (int mu = 0; mu < 4; ++mu) {
    p_mu_p[mu] = 0.5 * (unit + gammas[mu]);
    p_mu_m[mu] = 0.5 * (unit - gammas[mu]);
  }
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    Vector<WilsonVector> v = out.get_elems(xl);
    {
      const Vector<WilsonVector> iv = in.get_elems_const(xl);
      for (int m = 0; m < fa.ls; ++m) {
        v[m] = (5.0 - fa.m5) * iv[m];
        v[m] -= p_m * (m < fa.ls-1 ? iv[m+1] : (WilsonVector)(-fa.mass * iv[0]));
        v[m] -= p_p * (m > 0 ? iv[m-1] : (WilsonVector)(-fa.mass * iv[fa.ls-1]));
      }
    }
    for (int mu = 0; mu < 4; ++mu) {
      const Coordinate xl_p = coordinate_shifts(xl, mu);
      const Coordinate xl_m = coordinate_shifts(xl, -mu-1);
      const ColorMatrix u_p = gf.get_elem(xl, mu);
      const ColorMatrix u_m = matrix_adjoint(gf.get_elem(xl_m, mu));
      const Vector<WilsonVector> iv_p = in.get_elems_const(xl_p);
      const Vector<WilsonVector> iv_m = in.get_elems_const(xl_m);
      for (int m = 0; m < fa.ls; ++m) {
        v[m] -= u_p * (p_mu_m[mu] * iv_p[m]);
        v[m] -= u_m * (p_mu_p[mu] * iv_m[m]);
      }
    }
  }
}

inline void multiply_m_dwf(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
  // out can be the same object as in
{
  TIMER("multiply_m_dwf(5d,5d,Inv)");
  const Geometry geo1 = geo_resize(in.geo, 1);
  FermionField5d in1;
  in1.init(geo1);
  in1 = in;
  refresh_expanded_1(in1);
  multiply_m_dwf_no_comm(out, in1, inv);
}

inline void multiply_wilson_d_no_comm(FermionField5d& out, const FermionField5d& in, const GaugeField& gf, const double mass)
  // set_left_expanded_gauge_field(gf, gf_);
  // in.geo = geo_reform(geo, 1, ls);
  // refresh_expanded_1(in);
{
  TIMER("multiply_wilson_d_no_comm(5d,5d,gf,mass)");
  const Geometry geo = geo_resize(in.geo);
  qassert(is_matching_geo(gf.geo, geo));
  out.init(geo);
  set_zero(out);
  const int ls = in.geo.multiplicity;
  qassert(out.geo.multiplicity == ls);
  const std::array<SpinMatrix,4>& gammas = SpinMatrixConstants::get_cps_gammas();
  const SpinMatrix& unit = SpinMatrixConstants::get_unit();
  std::array<SpinMatrix,4> p_mu_p;
  std::array<SpinMatrix,4> p_mu_m;
  for (int mu = 0; mu < 4; ++mu) {
    p_mu_p[mu] = 0.5 * (unit + gammas[mu]);
    p_mu_m[mu] = 0.5 * (unit - gammas[mu]);
  }
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    Vector<WilsonVector> v = out.get_elems(xl);
    {
      const Vector<WilsonVector> iv = in.get_elems_const(xl);
      for (int m = 0; m < ls; ++m) {
        v[m] = (4.0 + mass) * iv[m];
      }
    }
    for (int mu = 0; mu < 4; ++mu) {
      const Coordinate xl_p = coordinate_shifts(xl, mu);
      const Coordinate xl_m = coordinate_shifts(xl, -mu-1);
      const ColorMatrix u_p = gf.get_elem(xl, mu);
      const ColorMatrix u_m = matrix_adjoint(gf.get_elem(xl_m, mu));
      const Vector<WilsonVector> iv_p = in.get_elems_const(xl_p);
      const Vector<WilsonVector> iv_m = in.get_elems_const(xl_m);
      for (int m = 0; m < ls; ++m) {
        v[m] -= u_p * (p_mu_m[mu] * iv_p[m]);
        v[m] -= u_m * (p_mu_p[mu] * iv_m[m]);
      }
    }
  }
}

inline void multiply_d_minus(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  TIMER("multiply_d_minus(5d,5d,Inv)");
  const Geometry geo = geo_resize(in.geo);
  out.init(geo);
  set_zero(out);
  const FermionAction& fa = inv.fa;
  qassert(is_matching_geo(inv.geo, in.geo));
  qassert(is_matching_geo(inv.geo, out.geo));
  qassert(in.geo.multiplicity == fa.ls);
  qassert(out.geo.multiplicity == fa.ls);
  qassert(fa.bs.size() == fa.ls);
  qassert(fa.cs.size() == fa.ls);
  const GaugeField& gf = inv.gf;
  const Geometry geo1 = geo_resize(in.geo, 1);
  FermionField5d in1;
  in1.init(geo1);
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    Vector<WilsonVector> v = out.get_elems(xl);
    Vector<WilsonVector> v1 = in1.get_elems(xl);
    const Vector<WilsonVector> iv = in.get_elems_const(xl);
    for (int m = 0; m < fa.ls; ++m) {
      const Complex& c = fa.cs[m];
      v1[m] = c * iv[m];
      v[m] -= iv[m];
    }
  }
  refresh_expanded_1(in1);
  FermionField5d out1;
  multiply_wilson_d_no_comm(out1, in1, gf, -fa.m5);
  out += out1;
}

inline void multiply_m_full(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
  // out can be the same object as in
{
  TIMER("multiply_m_full(5d,5d,Inv)");
  const Geometry geo = geo_resize(in.geo);
  out.init(geo);
  set_zero(out);
  const FermionAction& fa = inv.fa;
  qassert(is_matching_geo(inv.geo, in.geo));
  qassert(is_matching_geo(inv.geo, out.geo));
  qassert(geo.multiplicity == fa.ls);
  qassert(in.geo.multiplicity == fa.ls);
  qassert(out.geo.multiplicity == fa.ls);
  qassert(fa.bs.size() == fa.ls);
  qassert(fa.cs.size() == fa.ls);
  const SpinMatrix& gamma5 = SpinMatrixConstants::get_gamma5();
  const SpinMatrix& unit = SpinMatrixConstants::get_unit();
  const SpinMatrix p_p = 0.5 * (unit + gamma5);
  const SpinMatrix p_m = 0.5 * (unit - gamma5);
  const GaugeField& gf = inv.gf;
  const Geometry geo1 = geo_resize(in.geo, 1);
  FermionField5d in1, fftmp;
  in1.init(geo1);
  fftmp.init(geo);
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    const Vector<WilsonVector> iv = in.get_elems_const(xl);
    Vector<WilsonVector> v = fftmp.get_elems(xl);
    Vector<WilsonVector> v1 = in1.get_elems(xl);
    for (int m = 0; m < fa.ls; ++m) {
      const Complex& b = fa.bs[m];
      const Complex& c = fa.cs[m];
      v1[m] = b * iv[m];
      v[m] = iv[m];
      const WilsonVector tmp =
        (p_m * (m < fa.ls-1 ? iv[m+1] : (WilsonVector)(-fa.mass * iv[0]))) +
        (p_p * (m > 0 ? iv[m-1] : (WilsonVector)(-fa.mass * iv[fa.ls-1])));
      v1[m] += c * tmp;
      v[m] -= tmp;
    }
  }
  refresh_expanded_1(in1);
  multiply_wilson_d_no_comm(out, in1, gf, -fa.m5);
  out += fftmp;
}

inline void get_half_fermion(FermionField5d& half, const FermionField5d& ff, const int eo)
  // 2:even 1:odd
{
  TIMER("get_half_fermion");
  Geometry geoh = geo_resize(ff.geo);
  geoh.eo = eo;
  half.init(geoh);
  qassert(half.geo.eo == eo);
  qassert(ff.geo.eo == 0);
  qassert(is_matching_geo(ff.geo, half.geo));
#pragma omp parallel for
  for (long index = 0; index < geoh.local_volume(); ++index) {
    const Coordinate xl = geoh.coordinate_from_index(index);
    assign(half.get_elems(xl), ff.get_elems_const(xl));
  }
}

inline void set_half_fermion(FermionField5d& ff, const FermionField5d& half, const int eo)
  // 2:even 1:odd
{
  TIMER("set_half_fermion");
  const Geometry geoh = half.geo;
  Geometry geo = geo_resize(geoh);
  geo.eo = 0;
  ff.init(geo);
  qassert(half.geo.eo == eo);
  qassert(ff.geo.eo == 0);
  qassert(is_matching_geo(ff.geo, half.geo));
#pragma omp parallel for
  for (long index = 0; index < geoh.local_volume(); ++index) {
    const Coordinate xl = geoh.coordinate_from_index(index);
    assign(ff.get_elems(xl), half.get_elems_const(xl));
  }
}

inline void project_eo(FermionField5d& ff, const int eo)
{
  TIMER("project_eo");
  qassert(eo == 1 or eo == 2);
  FermionField5d half;
  get_half_fermion(half, ff, eo);
  set_zero(ff);
  set_half_fermion(ff, half, eo);
}

inline void multiply_m_e_e(FermionField5d& out, const FermionField5d& in, const FermionAction& fa)
  // works for _o_o as well
{
  TIMER("multiply_m_e_e");
  if (is_initialized(out) and out.geo.eo == 3 - in.geo.eo) {
    out.geo.eo = in.geo.eo;
  }
  out.init(geo_resize(in.geo));
  qassert(is_matching_geo_mult(out.geo, in.geo));
  qassert(in.geo.eo == 1 or in.geo.eo == 2);
  const SpinMatrix& gamma5 = SpinMatrixConstants::get_gamma5();
  const SpinMatrix& unit = SpinMatrixConstants::get_unit();
  const SpinMatrix p_p = 0.5 * (unit + gamma5);
  const SpinMatrix p_m = 0.5 * (unit - gamma5);
  FermionField5d in_copy;
  ConstHandle<FermionField5d> hin;
  if (&out != &in) {
    hin.init(in);
  } else {
    in_copy.init(geo_resize(in.geo));
    in_copy = in;
    hin.init(in_copy);
  }
  const Geometry& geo = out.geo;
  std::vector<Complex> bee(fa.ls), cee(fa.ls);
  for (int m = 0; m < fa.ls; ++m) {
    bee[m] = 1.0 + fa.bs[m] * (4.0 - fa.m5);
    cee[m] = 1.0 - fa.cs[m] * (4.0 - fa.m5);
  }
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    const Vector<WilsonVector> iv = hin().get_elems_const(xl);
    Vector<WilsonVector> v = out.get_elems(xl);
    for (int m = 0; m < fa.ls; ++m) {
      v[m] = bee[m] * iv[m];
      const WilsonVector tmp =
        (p_m * (m < fa.ls-1 ? iv[m+1] : (WilsonVector)(-fa.mass * iv[0]))) +
        (p_p * (m > 0 ? iv[m-1] : (WilsonVector)(-fa.mass * iv[fa.ls-1])));
      v[m] -= cee[m] * tmp;
    }
  }
}

inline void multiply_mdag_e_e(FermionField5d& out, const FermionField5d& in, const FermionAction& fa)
  // works for _o_o as well
{
  TIMER("multiply_mdag_e_e");
  if (is_initialized(out) and out.geo.eo == 3 - in.geo.eo) {
    out.geo.eo = in.geo.eo;
  }
  out.init(geo_resize(in.geo));
  qassert(is_matching_geo_mult(out.geo, in.geo));
  qassert(in.geo.eo == 1 or in.geo.eo == 2);
  const SpinMatrix& gamma5 = SpinMatrixConstants::get_gamma5();
  const SpinMatrix& unit = SpinMatrixConstants::get_unit();
  const SpinMatrix p_p = 0.5 * (unit + gamma5);
  const SpinMatrix p_m = 0.5 * (unit - gamma5);
  FermionField5d in_copy;
  ConstHandle<FermionField5d> hin;
  if (&out != &in) {
    hin.init(in);
  } else {
    in_copy.init(geo_resize(in.geo));
    in_copy = in;
    hin.init(in_copy);
  }
  const Geometry& geo = out.geo;
  std::vector<Complex> bee(fa.ls), cee(fa.ls);
  for (int m = 0; m < fa.ls; ++m) {
    bee[m] = std::conj(1.0 + fa.bs[m] * (4.0 - fa.m5));
    cee[m] = std::conj(1.0 - fa.cs[m] * (4.0 - fa.m5));
  }
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    const Vector<WilsonVector> iv = hin().get_elems_const(xl);
    Vector<WilsonVector> v = out.get_elems(xl);
    for (int m = 0; m < fa.ls; ++m) {
      v[m] = bee[m] * iv[m];
      const WilsonVector tmp =
        (p_p * (m < fa.ls-1 ? (WilsonVector)(cee[m+1] * iv[m+1]) : (WilsonVector)(-std::conj(fa.mass) * cee[0] * iv[0]))) +
        (p_m * (m > 0 ? (WilsonVector)(cee[m-1] * iv[m-1]) : (WilsonVector)(-std::conj(fa.mass) * cee[fa.ls-1] * iv[fa.ls-1])));
      v[m] -= tmp;
    }
  }
}

inline void multiply_m_e_e_inv(FermionField5d& out, const FermionField5d& in, const FermionAction& fa)
  // works for _o_o as well
{
  TIMER("multiply_m_e_e_inv");
  if (is_initialized(out) and out.geo.eo == 3 - in.geo.eo) {
    out.geo.eo = in.geo.eo;
  }
  out.init(geo_resize(in.geo));
  qassert(is_matching_geo_mult(out.geo, in.geo));
  qassert(out.geo.eo == in.geo.eo);
  qassert(in.geo.eo == 1 or in.geo.eo == 2);
  const SpinMatrix& gamma5 = SpinMatrixConstants::get_gamma5();
  const SpinMatrix& unit = SpinMatrixConstants::get_unit();
  const SpinMatrix p_p = 0.5 * (unit + gamma5);
  const SpinMatrix p_m = 0.5 * (unit - gamma5);
  const Geometry& geo = out.geo;
  std::vector<Complex> bee(fa.ls), cee(fa.ls);
  for (int m = 0; m < fa.ls; ++m) {
    bee[m] = 1.0 + fa.bs[m] * (4.0 - fa.m5);
    cee[m] = 1.0 - fa.cs[m] * (4.0 - fa.m5);
  }
  std::vector<Complex> lee(fa.ls-1), leem(fa.ls-1);
  for (int m = 0; m < fa.ls-1; ++m) {
    lee[m] = -cee[m+1] / bee[m];
    leem[m] = m == 0 ? fa.mass * cee[fa.ls-1] / bee[0] : leem[m-1] * cee[m-1] / bee[m];
  }
  std::vector<Complex> dee(fa.ls, 0.0);
  dee[fa.ls-1] = fa.mass * cee[fa.ls-1];
  for (int m = 0; m < fa.ls-1; ++m) {
    dee[fa.ls-1] *= cee[m] / bee[m];
  }
  for (int m = 0; m < fa.ls; ++m) {
    dee[m] += bee[m];
  }
  std::vector<Complex> uee(fa.ls-1), ueem(fa.ls-1);
  for (int m = 0; m < fa.ls-1; ++m) {
    uee[m] = -cee[m] / bee[m];
    ueem[m] = m == 0 ? fa.mass * cee[0] / bee[0] : ueem[m-1] * cee[m] / bee[m];
  }
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    const Vector<WilsonVector> iv = in.get_elems_const(xl);
    Vector<WilsonVector> v = out.get_elems(xl);
    std::memcpy(v.data(), iv.data(), iv.data_size());
    WilsonVector tmp;
    // {L^m_{ee}}^{-1}
    set_zero(tmp);
    for (int m = 0; m < fa.ls-1; ++m) {
      tmp += (-leem[m]) * v[m];
    }
    v[fa.ls-1] += p_m * tmp;
    // {L'_{ee}}^{-1}
    for (int m = 1; m < fa.ls; ++m) {
      v[m] += (-lee[m-1]) * p_p * v[m-1];
    }
    // {D_{ee}}^{-1}
    for (int m = 0; m < fa.ls; ++m) {
      v[m] *= 1.0 / dee[m];
    }
    // {U^'_{ee}}^{-1}
    for (int m = fa.ls-2; m >= 0; --m) {
      v[m] += (-uee[m]) * p_m * v[m+1];
    }
    // {U^m_{ee}}^{-1}
    for (int m = 0; m < fa.ls-1; ++m) {
      v[m] += (-ueem[m]) * p_p * v[fa.ls-1];
    }
  }
}

inline void multiply_mdag_e_e_inv(FermionField5d& out, const FermionField5d& in, const FermionAction& fa)
  // works for _o_o as well
{
  TIMER("multiply_mdag_e_e_inv");
  if (is_initialized(out) and out.geo.eo == 3 - in.geo.eo) {
    out.geo.eo = in.geo.eo;
  }
  out.init(geo_resize(in.geo));
  qassert(is_matching_geo_mult(out.geo, in.geo));
  qassert(out.geo.eo == in.geo.eo);
  qassert(in.geo.eo == 1 or in.geo.eo == 2);
  const SpinMatrix& gamma5 = SpinMatrixConstants::get_gamma5();
  const SpinMatrix& unit = SpinMatrixConstants::get_unit();
  const SpinMatrix p_p = 0.5 * (unit + gamma5);
  const SpinMatrix p_m = 0.5 * (unit - gamma5);
  const Geometry& geo = out.geo;
  std::vector<Complex> bee(fa.ls), cee(fa.ls);
  for (int m = 0; m < fa.ls; ++m) {
    bee[m] = 1.0 + fa.bs[m] * (4.0 - fa.m5);
    cee[m] = 1.0 - fa.cs[m] * (4.0 - fa.m5);
  }
  std::vector<Complex> lee(fa.ls-1), leem(fa.ls-1);
  for (int m = 0; m < fa.ls-1; ++m) {
    lee[m] = -cee[m+1] / bee[m];
    leem[m] = m == 0 ? fa.mass * cee[fa.ls-1] / bee[0] : leem[m-1] * cee[m-1] / bee[m];
  }
  std::vector<Complex> dee(fa.ls, 0.0);
  dee[fa.ls-1] = fa.mass * cee[fa.ls-1];
  for (int m = 0; m < fa.ls-1; ++m) {
    dee[fa.ls-1] *= cee[m] / bee[m];
  }
  for (int m = 0; m < fa.ls; ++m) {
    dee[m] += bee[m];
  }
  std::vector<Complex> uee(fa.ls-1), ueem(fa.ls-1);
  for (int m = 0; m < fa.ls-1; ++m) {
    uee[m] = -cee[m] / bee[m];
    ueem[m] = m == 0 ? fa.mass * cee[0] / bee[0] : ueem[m-1] * cee[m] / bee[m];
  }
  for (int m = 0; m < fa.ls; ++m) {
    bee[m] = std::conj(bee[m]);
    cee[m] = std::conj(cee[m]);
    dee[m] = std::conj(dee[m]);
  }
  for (int m = 0; m < fa.ls-1; ++m) {
    lee[m] = std::conj(lee[m]);
    leem[m] = std::conj(leem[m]);
    uee[m] = std::conj(uee[m]);
    ueem[m] = std::conj(ueem[m]);
  }
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    const Vector<WilsonVector> iv = in.get_elems_const(xl);
    Vector<WilsonVector> v = out.get_elems(xl);
    std::memcpy(v.data(), iv.data(), iv.data_size());
    WilsonVector tmp;
    // {U^m_{ee}}^\dagger^{-1}
    set_zero(tmp);
    for (int m = 0; m < fa.ls-1; ++m) {
      tmp += (-ueem[m]) * v[m];
    }
    v[fa.ls-1] += p_p * tmp;
    // {U^'_{ee}}^\dagger^{-1}
    for (int m = 1; m < fa.ls; ++m) {
      v[m] += (-uee[m-1]) * p_m * v[m-1];
    }
    // {D_{ee}}^\dagger^{-1}
    for (int m = 0; m < fa.ls; ++m) {
      v[m] *= 1.0 / dee[m];
    }
    // {L'_{ee}}^\dagger^{-1}
    for (int m = fa.ls-2; m >= 0; --m) {
      v[m] += (-lee[m]) * p_p * v[m+1];
    }
    // {L^m_{ee}}^\dagger^{-1}
    for (int m = 0; m < fa.ls-1; ++m) {
      v[m] += (-leem[m]) * p_m * v[fa.ls-1];
    }
  }
}

inline void multiply_wilson_d_e_o_no_comm(FermionField5d& out, const FermionField5d& in, const GaugeField& gf)
  // set_left_expanded_gauge_field(gf, gf_);
  // in.geo = geo_reform(geo, 1, ls);
  // refresh_expanded_1(in);
{
  TIMER("multiply_wilson_d_e_o_no_comm(5d,5d,gf)");
  qassert(is_matching_geo(gf.geo, in.geo));
  qassert(in.geo.eo == 1 or in.geo.eo == 2);
  Geometry geo = geo_resize(in.geo);
  geo.eo = 3 - in.geo.eo;
  if (is_initialized(out) and out.geo.eo == 3 - geo.eo) {
    out.geo.eo = geo.eo;
  }
  out.init(geo);
  set_zero(out);
  const int ls = in.geo.multiplicity;
  qassert(out.geo.multiplicity == ls);
  qassert(is_matching_geo(out.geo, in.geo));
  qassert(out.geo.eo != in.geo.eo);
  qassert(out.geo.eo == 1 or out.geo.eo == 2);
  const std::array<SpinMatrix,4>& gammas = SpinMatrixConstants::get_cps_gammas();
  const SpinMatrix& unit = SpinMatrixConstants::get_unit();
  std::array<SpinMatrix,4> p_mu_p;
  std::array<SpinMatrix,4> p_mu_m;
  for (int mu = 0; mu < 4; ++mu) {
    p_mu_p[mu] = 0.5 * (unit + gammas[mu]);
    p_mu_m[mu] = 0.5 * (unit - gammas[mu]);
  }
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    Vector<WilsonVector> v = out.get_elems(xl);
    for (int mu = 0; mu < 4; ++mu) {
      const Coordinate xl_p = coordinate_shifts(xl, mu);
      const Coordinate xl_m = coordinate_shifts(xl, -mu-1);
      const ColorMatrix u_p = gf.get_elem(xl, mu);
      const ColorMatrix u_m = matrix_adjoint(gf.get_elem(xl_m, mu));
      const Vector<WilsonVector> iv_p = in.get_elems_const(xl_p);
      const Vector<WilsonVector> iv_m = in.get_elems_const(xl_m);
      for (int m = 0; m < ls; ++m) {
        v[m] -= u_p * (p_mu_m[mu] * iv_p[m]);
        v[m] -= u_m * (p_mu_p[mu] * iv_m[m]);
      }
    }
  }
}

inline void multiply_wilson_ddag_e_o_no_comm(FermionField5d& out, const FermionField5d& in, const GaugeField& gf)
  // set_left_expanded_gauge_field(gf, gf_);
  // in.geo = geo_reform(geo, 1, ls);
  // refresh_expanded_1(in);
{
  TIMER("multiply_wilson_ddag_e_o_no_comm(5d,5d,gf)");
  qassert(is_matching_geo(gf.geo, in.geo));
  qassert(in.geo.eo == 1 or in.geo.eo == 2);
  Geometry geo = geo_resize(in.geo);
  geo.eo = 3 - in.geo.eo;
  if (is_initialized(out) and out.geo.eo == 3 - geo.eo) {
    out.geo.eo = geo.eo;
  }
  out.init(geo);
  set_zero(out);
  const int ls = in.geo.multiplicity;
  qassert(out.geo.multiplicity == ls);
  qassert(is_matching_geo(out.geo, in.geo));
  qassert(out.geo.eo != in.geo.eo);
  qassert(out.geo.eo == 1 or out.geo.eo == 2);
  const std::array<SpinMatrix,4>& gammas = SpinMatrixConstants::get_cps_gammas();
  const SpinMatrix& unit = SpinMatrixConstants::get_unit();
  std::array<SpinMatrix,4> p_mu_p;
  std::array<SpinMatrix,4> p_mu_m;
  for (int mu = 0; mu < 4; ++mu) {
    p_mu_p[mu] = 0.5 * (unit + gammas[mu]);
    p_mu_m[mu] = 0.5 * (unit - gammas[mu]);
  }
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    Vector<WilsonVector> v = out.get_elems(xl);
    for (int mu = 0; mu < 4; ++mu) {
      const Coordinate xl_p = coordinate_shifts(xl, mu);
      const Coordinate xl_m = coordinate_shifts(xl, -mu-1);
      const ColorMatrix u_p = gf.get_elem(xl, mu);
      const ColorMatrix u_m = matrix_adjoint(gf.get_elem(xl_m, mu));
      const Vector<WilsonVector> iv_p = in.get_elems_const(xl_p);
      const Vector<WilsonVector> iv_m = in.get_elems_const(xl_m);
      for (int m = 0; m < ls; ++m) {
        v[m] -= u_p * (p_mu_p[mu] * iv_p[m]);
        v[m] -= u_m * (p_mu_m[mu] * iv_m[m]);
      }
    }
  }
}

inline void multiply_m_e_o(FermionField5d& out, const FermionField5d& in, const GaugeField& gf, const FermionAction& fa)
  // works for _o_e as well
{
  TIMER("multiply_m_e_o(5d,5d,gf,fa)");
  const SpinMatrix& gamma5 = SpinMatrixConstants::get_gamma5();
  const SpinMatrix& unit = SpinMatrixConstants::get_unit();
  const SpinMatrix p_p = 0.5 * (unit + gamma5);
  const SpinMatrix p_m = 0.5 * (unit - gamma5);
  const int in_geo_eo = in.geo.eo;
  FermionField5d in1;
  in1.init(geo_resize(in.geo, 1));
  const Geometry& geo = in.geo;
  std::vector<Complex> beo(fa.ls), ceo(fa.ls);
  for (int m = 0; m < fa.ls; ++m) {
    beo[m] = fa.bs[m];
    ceo[m] = -fa.cs[m];
  }
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    const Vector<WilsonVector> iv = in.get_elems_const(xl);
    Vector<WilsonVector> v = in1.get_elems(xl);
    for (int m = 0; m < fa.ls; ++m) {
      v[m] = beo[m] * iv[m];
      const WilsonVector tmp =
        (p_m * (m < fa.ls-1 ? iv[m+1] : (WilsonVector)(-fa.mass * iv[0]))) +
        (p_p * (m > 0 ? iv[m-1] : (WilsonVector)(-fa.mass * iv[fa.ls-1])));
      v[m] -= ceo[m] * tmp;
    }
  }
  refresh_expanded_1(in1);
  multiply_wilson_d_e_o_no_comm(out, in1, gf);
  qassert(is_matching_geo(out.geo, in.geo));
  qassert(out.geo.eo != in_geo_eo);
  qassert(in.geo.eo == 1 or in.geo.eo == 2);
  qassert(out.geo.eo == 1 or out.geo.eo == 2);
}

inline void multiply_mdag_e_o(FermionField5d& out, const FermionField5d& in, const GaugeField& gf, const FermionAction& fa)
  // works for _o_e as well
{
  TIMER("multiply_mdag_e_o(5d,5d,gf,fa)");
  const SpinMatrix& gamma5 = SpinMatrixConstants::get_gamma5();
  const SpinMatrix& unit = SpinMatrixConstants::get_unit();
  const SpinMatrix p_p = 0.5 * (unit + gamma5);
  const SpinMatrix p_m = 0.5 * (unit - gamma5);
  const int in_geo_eo = in.geo.eo;
  qassert(is_matching_geo(gf.geo, in.geo));
  qassert(in.geo.eo == 1 or in.geo.eo == 2);
  Geometry geo = geo_resize(in.geo);
  geo.eo = 3 - in.geo.eo;
  std::vector<Complex> beo(fa.ls), ceo(fa.ls);
  for (int m = 0; m < fa.ls; ++m) {
    beo[m] = std::conj(fa.bs[m]);
    ceo[m] = std::conj(-fa.cs[m]);
  }
  FermionField5d in1;
  in1.init(geo_resize(in.geo, 1));
  in1 = in;
  refresh_expanded_1(in1);
  FermionField5d out1;
  multiply_wilson_ddag_e_o_no_comm(out1, in1, gf);
  in1.init();
  if (is_initialized(out) and out.geo.eo == 3 - geo.eo) {
    out.geo.eo = geo.eo;
  }
  out.init(geo);
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    const Vector<WilsonVector> iv = out1.get_elems_const(xl);
    Vector<WilsonVector> v = out.get_elems(xl);
    for (int m = 0; m < fa.ls; ++m) {
      v[m] = beo[m] * iv[m];
      const WilsonVector tmp =
        (p_p * (m < fa.ls-1 ? (WilsonVector)(ceo[m+1] * iv[m+1]) : (WilsonVector)(-std::conj(fa.mass) * ceo[0] * iv[0]))) +
        (p_m * (m > 0 ? (WilsonVector)(ceo[m-1] * iv[m-1]) : (WilsonVector)(-std::conj(fa.mass) * ceo[fa.ls-1] * iv[fa.ls-1])));
      v[m] -= tmp;
    }
  }
  qassert(is_matching_geo(out.geo, in.geo));
  qassert(out.geo.eo != in_geo_eo);
  qassert(in.geo.eo == 1 or in.geo.eo == 2);
  qassert(out.geo.eo == 1 or out.geo.eo == 2);
}

inline void multiply_m(FermionField5d& out, const FermionField5d& in, const GaugeField& gf, const FermionAction& fa)
{
  TIMER("multiply_m");
  FermionField5d in_e, in_o;
  get_half_fermion(in_e, in, 2);
  get_half_fermion(in_o, in, 1);
  FermionField5d tmp_e, tmp_o;
  FermionField5d out_e, out_o;
  multiply_m_e_o(tmp_e, in_o, gf, fa);
  multiply_m_e_e(out_e, in_e, fa);
  multiply_m_e_e(tmp_o, in_o, fa);
  multiply_m_e_o(out_o, in_e, gf, fa);
  out_e += tmp_e;
  out_o += tmp_o;
  set_half_fermion(out, out_e, 2);
  set_half_fermion(out, out_o, 1);
}

inline void multiply_mdag(FermionField5d& out, const FermionField5d& in, const GaugeField& gf, const FermionAction& fa)
{
  TIMER("multiply_mdag");
  FermionField5d in_e, in_o;
  get_half_fermion(in_e, in, 2);
  get_half_fermion(in_o, in, 1);
  FermionField5d tmp_e, tmp_o;
  FermionField5d out_e, out_o;
  multiply_mdag_e_o(tmp_e, in_o, gf, fa);
  multiply_mdag_e_e(out_e, in_e, fa);
  multiply_mdag_e_e(tmp_o, in_o, fa);
  multiply_mdag_e_o(out_o, in_e, gf, fa);
  out_e += tmp_e;
  out_o += tmp_o;
  set_half_fermion(out, out_e, 2);
  set_half_fermion(out, out_o, 1);
}

inline void multiply_mpc_sym2(FermionField5d& out, const FermionField5d& in, const GaugeField& gf, const FermionAction& fa)
{
  TIMER("multiply_mpc_sym2");
  FermionField5d tmp;
  multiply_m_e_e_inv(tmp, in, fa);
  multiply_m_e_o(tmp, tmp, gf, fa);
  multiply_m_e_e_inv(tmp, tmp, fa);
  multiply_m_e_o(tmp, tmp, gf, fa);
  if (is_initialized(out) and out.geo.eo == 3 - in.geo.eo) {
    out.geo.eo = in.geo.eo;
  }
  out.init(geo_resize(in.geo));
  out = in;
  out -= tmp;
}

inline void multiply_mpcdag_sym2(FermionField5d& out, const FermionField5d& in, const GaugeField& gf, const FermionAction& fa)
{
  TIMER("multiply_mpcdag_sym2");
  FermionField5d tmp;
  multiply_mdag_e_o(tmp, in, gf, fa);
  multiply_mdag_e_e_inv(tmp, tmp, fa);
  multiply_mdag_e_o(tmp, tmp, gf, fa);
  multiply_mdag_e_e_inv(tmp, tmp, fa);
  if (is_initialized(out) and out.geo.eo == 3 - in.geo.eo) {
    out.geo.eo = in.geo.eo;
  }
  out.init(geo_resize(in.geo));
  out = in;
  out -= tmp;
}

inline void multiply_hermop_sym2(FermionField5d& out, const FermionField5d& in, const GaugeField& gf, const FermionAction& fa)
{
  TIMER("multiply_hermop_sym2");
  multiply_mpc_sym2(out, in, gf, fa);
  multiply_mpcdag_sym2(out, out, gf, fa);
}

inline void multiply_m_e_e(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  multiply_m_e_e(out, in, inv.fa);
}

inline void multiply_mdag_e_e(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  multiply_mdag_e_e(out, in, inv.fa);
}

inline void multiply_m_e_e_inv(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  multiply_m_e_e_inv(out, in, inv.fa);
}

inline void multiply_mdag_e_e_inv(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  multiply_mdag_e_e_inv(out, in, inv.fa);
}

inline void multiply_m_e_o(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  multiply_m_e_o(out, in, inv.gf, inv.fa);
}

inline void multiply_mdag_e_o(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  multiply_mdag_e_o(out, in, inv.gf, inv.fa);
}

inline void multiply_m_eo_eo(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv,
    const int eo_out, const int eo_in)
  // out need to be initialized with correct geo and eo
{
  TIMER("multiply_m_eo_eo");
  Geometry geo = geo_resize(in.geo);
  geo.eo = eo_out;
  out.init(geo);
  qassert(is_matching_geo(out.geo, in.geo));
  qassert(out.geo.eo == eo_out);
  qassert(in.geo.eo == eo_in);
  if (eo_out == eo_in) {
    multiply_m_e_e(out, in, inv.fa);
  } else {
    multiply_m_e_o(out, in, inv.gf, inv.fa);
  }
}

inline void multiply_mdag_eo_eo(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv,
    const int eo_out, const int eo_in)
  // out need to be initialized with correct geo and eo
{
  TIMER("multiply_mdag_eo_eo");
  Geometry geo = geo_resize(in.geo);
  geo.eo = eo_out;
  out.init(geo);
  qassert(is_matching_geo(out.geo, in.geo));
  qassert(out.geo.eo == eo_out);
  qassert(in.geo.eo == eo_in);
  if (eo_out == eo_in) {
    multiply_mdag_e_e(out, in, inv.fa);
  } else {
    multiply_mdag_e_o(out, in, inv.gf, inv.fa);
  }
}

inline void multiply_m(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  multiply_m(out, in, inv.gf, inv.fa);
}

inline void multiply_mdag(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  multiply_mdag(out, in, inv.gf, inv.fa);
}

inline void multiply_mpc_sym2(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  multiply_mpc_sym2(out, in, inv.gf, inv.fa);
}

inline void multiply_mpcdag_sym2(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  multiply_mpcdag_sym2(out, in, inv.gf, inv.fa);
}

inline void multiply_hermop_sym2(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  multiply_hermop_sym2(out, in, inv.gf, inv.fa);
}

inline void multiply_m_with_prec_sym2(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  TIMER("multiply_m_with_prec_sym2");
  FermionField5d in_e, in_o;
  FermionField5d out_e, out_o;
  get_half_fermion(in_e, in, 2);
  get_half_fermion(in_o, in, 1);
  //
  multiply_m_e_o(out_e, in_o, inv);
  multiply_m_e_e_inv(out_e, out_e, inv);
  out_e += in_e;
  multiply_m_e_e(out_o, in_o, inv);
  //
  multiply_m_e_e(out_e, out_e, inv);
  multiply_mpc_sym2(out_o, out_o, inv);
  //
  FermionField5d tmp;
  multiply_m_e_e_inv(tmp, out_e, inv);
  multiply_m_e_o(tmp, tmp, inv);
  out_o += tmp;
  //
  set_half_fermion(out, out_e, 2);
  set_half_fermion(out, out_o, 1);
}

inline Complex dot_product(const FermionField5d& ff1, const FermionField5d& ff2)
  // return ff1^dag * ff2
{
  TIMER("dot_product");
  qassert(is_matching_geo(ff1.geo, ff2.geo));
  qassert(ff1.geo.eo == ff2.geo.eo);
  const Geometry& geo = ff1.geo;
  Complex sum = 0.0;
  for (long index = 0; index < geo.local_volume(); ++index) {
    const Coordinate xl = geo.coordinate_from_index(index);
    const Vector<WilsonVector> v1 = ff1.get_elems_const(xl);
    const Vector<WilsonVector> v2 = ff2.get_elems_const(xl);
    const Vector<Complex> cv1((const Complex*)v1.data(), v1.data_size() / sizeof(Complex));
    const Vector<Complex> cv2((const Complex*)v2.data(), v2.data_size() / sizeof(Complex));
    qassert(cv1.size() == cv2.size());
    for (int k = 0; k < cv1.size(); ++k) {
      sum += std::conj(cv1[k]) * cv2[k];
    }
  }
  glb_sum(sum);
  return sum;
}

template <class Inv>
inline long cg_with_f(FermionField5d& out, const FermionField5d& in, const Inv& inv, void f(FermionField5d&, const FermionField5d&, const Inv&))
  // f(out, in, inv);
{
  TIMER_VERBOSE("cg_with_f");
  const Geometry geo = geo_resize(in.geo);
  out.init(geo);
  if (inv.max_num_iter() == 0) {
    return 0;
  }
  FermionField5d r, p, tmp, ap;
  r.init(geo);
  p.init(geo);
  tmp.init(geo);
  r = in;
  f(tmp, out, inv);
  r -= tmp;
  p = r;
  const double norm_in = norm(in);
  double norm_r = norm(r);
  for (long iter = 0; iter < inv.max_num_iter(); ++iter) {
    f(ap, p, inv);
    const double alpha = norm_r / dot_product(p, ap).real();
    tmp = p;
    tmp *= alpha;
    out += tmp;
    tmp = ap;
    tmp *= alpha;
    r -= tmp;
    const double new_norm_r = norm(r);
    // displayln_info(fname + ssprintf(": iter=%ld %E", iter, sqrt(new_norm_r / norm_in)));
    if (new_norm_r <= norm_in * sqr(inv.stop_rsd())) {
      displayln_info(fname + ssprintf(": iter=%ld %E", iter, sqrt(new_norm_r / norm_in)));
      return iter;
    }
    const double beta = new_norm_r / norm_r;
    p *= beta;
    p += r;
    norm_r = new_norm_r;
  }
  displayln_info(fname + ssprintf(": max_num_iter=%ld %E", inv.max_num_iter(), sqrt(norm_r / norm_in)));
}

inline void inverse(FermionField5d& out, const FermionField5d& in, const InverterDomainWall& inv)
{
  TIMER_VERBOSE("inverse(5d,5d,inv)");
  out.init(geo_resize(in.geo));
  if (inv.fa.is_using_zmobius == true and inv.fa.cg_diagonal_mee == 2) {
    FermionField5d in_e, in_o;
    FermionField5d out_e, out_o;
    get_half_fermion(in_e, in, 2);
    get_half_fermion(in_o, in, 1);
    //
    FermionField5d tmp;
    multiply_m_e_e_inv(out_e, in_e, inv);
    multiply_m_e_o(tmp, out_e, inv);
    in_o -= tmp;
    multiply_mpcdag_sym2(in_o, in_o, inv);
    //
    if (not inv.lm.null()) {
      deflate(out_o, in_o, inv.lm());
    }
    cg_with_f(out_o, in_o, inv, multiply_hermop_sym2);
    //
    multiply_m_e_e_inv(out_o, out_o, inv);
    multiply_m_e_o(tmp, out_o, inv);
    multiply_m_e_e_inv(tmp, tmp, inv);
    out_e -= tmp;
    //
    set_half_fermion(out, out_e, 2);
    set_half_fermion(out, out_o, 1);
  } else {
    qassert(false);
  }
  if (is_checking_inverse()) {
    FermionField5d tmp;
    multiply_m(tmp, out, inv);
    tmp -= in;
    displayln_info(fname + ssprintf(": %E from %E", norm(tmp), norm(in)));
  }
}

QLAT_END_NAMESPACE