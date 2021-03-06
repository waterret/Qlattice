#pragma once

#include <qlat/config.h>
#include <qlat/mvector.h>

#include <cmath>

namespace qlat
{  //

template <int DIMN, class T = ComplexT>
struct alignas(QLAT_ALIGNED_BYTES) MatrixT
{
  T p[DIMN * DIMN];
  //
  // convert to double array
  qacc double* d() { return (double*)p; }
  qacc const double* d() const { return (const double*)p; }
  //
  // convert to Eigen MatrixT
  qacc Eigen::Matrix<T, DIMN, DIMN, Eigen::RowMajor>& em()
  {
    return *((Eigen::Matrix<T, DIMN, DIMN, Eigen::RowMajor>*)this);
  }
  qacc const Eigen::Matrix<T, DIMN, DIMN, Eigen::RowMajor>& em() const
  {
    return *((Eigen::Matrix<T, DIMN, DIMN, Eigen::RowMajor>*)this);
  }
  //
  qacc T& operator()(int i, int j)
  {
    qassert(0 <= i && i < DIMN);
    qassert(0 <= j && j < DIMN);
    return p[i * DIMN + j];
  }
  qacc const T& operator()(int i, int j) const
  {
    qassert(0 <= i && i < DIMN);
    qassert(0 <= j && j < DIMN);
    return p[i * DIMN + j];
  }
  //
  qacc const MatrixT& operator+=(const MatrixT& x)
  {
    *this = *this + x;
    return *this;
  }
  //
  qacc const MatrixT& operator-=(const MatrixT& x)
  {
    *this = *this - x;
    return *this;
  }
  //
  qacc const MatrixT& operator*=(const MatrixT& x)
  {
    *this = *this * x;
    return *this;
  }
  //
  qacc const MatrixT& operator*=(const T& x)
  {
    *this = *this * x;
    return *this;
  }
  //
  qacc const MatrixT& operator/=(const T& x)
  {
    *this = *this / x;
    return *this;
  }
};

template <int DIMN, class T>
qacc MatrixT<DIMN, T> operator+(const MatrixT<DIMN, T>& x,
                                const MatrixT<DIMN, T>& y)
{
  MatrixT<DIMN, T> ret;
  ret.em() = x.em() + y.em();
  return ret;
}

template <int DIMN, class T>
qacc MatrixT<DIMN, T> operator-(const MatrixT<DIMN, T>& x,
                                const MatrixT<DIMN, T>& y)
{
  MatrixT<DIMN, T> ret;
  ret.em() = x.em() - y.em();
  return ret;
}

template <int DIMN, class T>
qacc MatrixT<DIMN, T> operator-(const MatrixT<DIMN, T>& x)
{
  MatrixT<DIMN, T> ret;
  ret.em() = -x.em();
  return ret;
}

template <int DIMN, class T>
qacc MatrixT<DIMN, T> operator*(const MatrixT<DIMN, T>& x,
                                const MatrixT<DIMN, T>& y)
{
  MatrixT<DIMN, T> ret;
  ret.em() = x.em() * y.em();
  return ret;
}

template <int DIMN, class T>
qacc MatrixT<DIMN, T> operator*(const double x, const MatrixT<DIMN, T>& y)
{
  MatrixT<DIMN, T> ret;
  ret.em() = x * y.em();
  return ret;
}

template <int DIMN, class T>
qacc MatrixT<DIMN, T> operator*(const MatrixT<DIMN, T>& x, const double y)
{
  MatrixT<DIMN, T> ret;
  ret.em() = x.em() * y;
  return ret;
}

template <int DIMN, class T>
qacc MatrixT<DIMN, T> operator*(const Complex& x, const MatrixT<DIMN, T>& y)
{
  MatrixT<DIMN, T> ret;
  ret.em() = x * y.em();
  return ret;
}

template <int DIMN, class T>
qacc MatrixT<DIMN, T> operator*(const MatrixT<DIMN, T>& x, const Complex& y)
{
  MatrixT<DIMN, T> ret;
  ret.em() = x.em() * y;
  return ret;
}

template <int DIMN, class T>
qacc MatrixT<DIMN, T> operator/(const MatrixT<DIMN, T>& x, const T& y)
{
  MatrixT<DIMN, T> ret;
  ret.em() = x.em() / y;
  return ret;
}

template <int DIMN, class T>
qacc MvectorT<DIMN, T> operator*(const MatrixT<DIMN, T>& x,
                                 const MvectorT<DIMN, T>& y)
{
  MvectorT<DIMN, T> ret;
  ret.em() = x.em() * y.em();
  return ret;
}

template <int DIMN, class T>
qacc void set_zero(MatrixT<DIMN, T>& m)
{
  std::memset((void*)&m, 0, sizeof(MatrixT<DIMN, T>));
}

template <int DIMN>
qacc void set_unit(MatrixT<DIMN, float>& m, const Complex& coef = 1.0)
{
  set_zero(m);
  for (int i = 0; i < DIMN; ++i) {
    m(i, i) = coef.real();
  }
}

template <int DIMN>
qacc void set_unit(MatrixT<DIMN, double>& m, const Complex& coef = 1.0)
{
  set_zero(m);
  for (int i = 0; i < DIMN; ++i) {
    m(i, i) = coef.real();
  }
}

template <int DIMN, class T>
qacc void set_unit(MatrixT<DIMN, T>& m, const Complex& coef = 1.0)
{
  set_zero(m);
  for (int i = 0; i < DIMN; ++i) {
    m(i, i) = coef;
  }
}

template <int DIMN, class T>
qacc double qnorm(const MatrixT<DIMN, T>& m)
{
  return m.em().squaredNorm();
}

template <int DIMN, class T>
qacc Complex matrix_trace(const MatrixT<DIMN, T>& x)
{
  return x.em().trace();
}

template <int DIMN, class T>
qacc Complex matrix_trace(const MatrixT<DIMN, T>& x, const MatrixT<DIMN, T>& y)
{
  return (x.em() * y.em()).trace();
}

template <int DIMN, class T>
qacc MatrixT<DIMN, T> matrix_adjoint(const MatrixT<DIMN, T>& x)
{
  MatrixT<DIMN, T> ret;
  ret.em() = x.em().adjoint();
  return ret;
}

template <int DIMN, class T>
qacc MatrixT<DIMN, T> matrix_transpose(const MatrixT<DIMN, T>& x)
{
  MatrixT<DIMN, T> ret;
  ret.em() = x.em().transpose();
  return ret;
}

template <int DIMN, class T>
qacc MatrixT<DIMN, T> matrix_conjugate(const MatrixT<DIMN, T>& x)
{
  MatrixT<DIMN, T> ret;
  ret.em() = x.em().conjugate();
  return ret;
}

template <int DIMN, class T>
qacc void matrix_ludcmp(MatrixT<DIMN, T>& a,
                        array<int, (unsigned long)DIMN>& indx, T& d)
// https://www.astro.umd.edu/~ricotti/NEWWEB/teaching/ASTR415/InClassExamples/NR3/legacy/nr2/C_211/progs.htm
// https://www.astro.umd.edu/~ricotti/NEWWEB/teaching/ASTR415/InClassExamples/NR3/legacy/nr2/C_211/recipes/ludcmp.c
//
// a: input and output
// indx, d: output
{
  MvectorT<DIMN, T> vv;
  d = 1.0;
  for (int i = 0; i < DIMN; ++i) {
    double big = 0.0;
    for (int j = 0; j < DIMN; ++j) {
      const double temp = std::abs(a(i, j));
      if (temp > big) {
        big = temp;
      }
    }
    if (big == 0.0) {
      // matrix singular
      qassert(false);
    }
    vv(i) = 1.0 / big;
  }
  for (int j = 0; j < DIMN; ++j) {
    for (int i = 0; i < j; ++i) {
      T sum = a(i, j);
      for (int k = 0; k < i; ++k) {
        sum -= a(i, k) * a(k, j);
      }
      a(i, j) = sum;
    }
    double big = 0.0;
    int imax = -1;
    for (int i = j; i < DIMN; ++i) {
      T sum = a(i, j);
      for (int k = 0; k < j; ++k) {
        sum -= a(i, k) * a(k, j);
      }
      a(i, j) = sum;
      const T dum = vv(i) * std::abs(sum);
      if (dum >= big) {
        big = dum;
        imax = i;
      }
    }
    if (j != imax) {
      for (int k = 0; k < DIMN; ++k) {
        const T dum = a(imax, k);
        a(imax, k) = a(j, k);
        a(j, k) = dum;
      }
      d = -d;
      vv(imax) = vv(j);
    }
    indx[j] = imax;
    if (a(j, j) == 0.0) {
      // matrix singular
      qassert(false);
    }
    if (j != DIMN - 1) {
      const T dum = 1.0 / a(j, j);
      for (int i = j + 1; i < DIMN; ++i) {
        a(i, j) *= dum;
      }
    }
  }
}

template <int DIMN, class T>
qacc void matrix_lubksb(MvectorT<DIMN, T>& b, const MatrixT<DIMN, T>& a,
                        const array<int, (unsigned long)DIMN>& indx)
// https://www.astro.umd.edu/~ricotti/NEWWEB/teaching/ASTR415/InClassExamples/NR3/legacy/nr2/C_211/progs.htm
// https://www.astro.umd.edu/~ricotti/NEWWEB/teaching/ASTR415/InClassExamples/NR3/legacy/nr2/C_211/recipes/lubksb.c
//
// b: input and output
{
  int ii = -1;
  for (int i = 0; i < DIMN; ++i) {
    const int ip = indx[i];
    T sum = b(ip);
    b(ip) = b(i);
    if (ii >= 0) {
      for (int j = ii; j < i; j++) {
        sum -= a(i, j) * b(j);
      }
    } else if (sum != 0.0) {
      ii = i;
    }
    b(i) = sum;
  }
  for (int i = DIMN - 1; i >= 0; --i) {
    T sum = b(i);
    for (int j = i + 1; j < DIMN; ++j) {
      sum -= a(i, j) * b(j);
    }
    b(i) = sum / a(i, i);
  }
}

template <int DIMN, class T>
MatrixT<DIMN, T> matrix_inverse_eigen(const MatrixT<DIMN, T>& x)
{
  MatrixT<DIMN, T> ret;
  ret.em() = x.em().inverse();
  return ret;
}

template <int DIMN, class T>
qacc MatrixT<DIMN, T> matrix_inverse(const MatrixT<DIMN, T>& x)
{
  MatrixT<DIMN, T> ret;
  MatrixT<DIMN, T> a = x;
  array<int, DIMN> indx;
  T d;
  matrix_ludcmp(a, indx, d);
  MvectorT<DIMN, T> col;
  for (int j = 0; j < DIMN; ++j) {
    set_zero(col);
    col(j) = 1.0;
    matrix_lubksb(col, a, indx);
    for (int i = 0; i < DIMN; ++i) {
      ret(i, j) = col(i);
    }
  }
  return ret;
}

template <int DIMN, class T>
T matrix_determinant_eigen(const MatrixT<DIMN, T>& x)
{
  return x.em().determinant();
}

template <int DIMN, class T>
qacc T matrix_determinant(const MatrixT<DIMN, T>& x)
{
  MatrixT<DIMN, T> a = x;
  array<int, DIMN> indx;
  T d;
  matrix_ludcmp(a, indx, d);
  for (int j = 0; j < DIMN; ++j) {
    d *= a(j, j);
  }
  return d;
}

template <class T = ComplexT>
struct ColorMatrixT : MatrixT<NUM_COLOR, T> {
  qacc ColorMatrixT() {}
  qacc ColorMatrixT(const MatrixT<NUM_COLOR, T>& m) { *this = m; }
  //
  qacc const ColorMatrixT& operator=(const MatrixT<NUM_COLOR, T>& m)
  {
    *this = (const ColorMatrixT&)m;
    return *this;
  }
};

template <class T = ComplexT>
struct WilsonMatrixT : MatrixT<4 * NUM_COLOR, T> {
  qacc WilsonMatrixT() {}
  qacc WilsonMatrixT(const MatrixT<4 * NUM_COLOR, T>& m) { *this = m; }
  //
  qacc const WilsonMatrixT& operator=(const MatrixT<4 * NUM_COLOR, T>& m)
  {
    *this = (const WilsonMatrixT&)m;
    return *this;
  }
};

template <class T = ComplexT>
struct NonRelWilsonMatrixT : MatrixT<2 * NUM_COLOR, T> {
  qacc NonRelWilsonMatrixT() {}
  qacc NonRelWilsonMatrixT(const MatrixT<2 * NUM_COLOR, T>& m) { *this = m; }
  //
  qacc const NonRelWilsonMatrixT& operator=(const MatrixT<2 * NUM_COLOR, T>& m)
  {
    *this = (const NonRelWilsonMatrixT&)m;
    return *this;
  }
};

template <class T = ComplexT>
struct SpinMatrixT : MatrixT<4, T> {
  qacc SpinMatrixT() {}
  qacc SpinMatrixT(const MatrixT<4, T>& m) { *this = m; }
  //
  qacc const SpinMatrixT& operator=(const MatrixT<4, T>& m)
  {
    *this = (const SpinMatrixT&)m;
    return *this;
  }
};

template <class T = ComplexT>
struct SpinMatrixConstantsT {
  SpinMatrixT<T> unit;
  array<SpinMatrixT<T>, 4>
      gammas;  // Not using CPS's convention, but a more standard one.
  array<SpinMatrixT<T>, 4> cps_gammas;  // CPS's convention gamma matrices
  SpinMatrixT<T> gamma5;                     // Same as CPS's gamma5
  array<SpinMatrixT<T>, 3> cap_sigmas;
  array<SpinMatrixT<T>, 3> cps_cap_sigmas;  // CPS's convention sigmas
  array<SpinMatrixT<T>, 16> gms;
  array<SpinMatrixT<T>, 16> cps_gms;
  //
  qacc SpinMatrixConstantsT() { init(); }
  //
  qacc void init()
  {
    Complex ii(0.0, 1.0);
    // TIMER_VERBOSE("SpinMatrixConstants::init()");
    unit.em() << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
    // gamma_x
    gammas[0].em() << 0, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, 0;
    // gamma_y
    gammas[1].em() << 0, 0, 0, (T)-ii, 0, 0, (T)ii, 0, 0, (T)ii, 0, 0, (T)-ii,
        0, 0, 0;
    // gamma_z
    gammas[2].em() << 0, 0, 1, 0, 0, 0, 0, -1, -1, 0, 0, 0, 0, 1, 0, 0;
    gammas[0] *= (T)(-ii);
    gammas[1] *= (T)(-ii);
    gammas[2] *= (T)(-ii);
    // gamma_t
    gammas[3].em() << 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0;
    //
    cps_gammas[0] = -gammas[0];
    cps_gammas[1] = gammas[1];
    cps_gammas[2] = -gammas[2];
    cps_gammas[3] = gammas[3];
    // gamma_5 = gamma_x * gamma_y * gamma_z * gamma_t;
    gamma5.em() << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1;
    // Sigma_x
    cap_sigmas[0].em() << 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0;
    // Sigma_y
    cap_sigmas[1].em() << 0, (T)-ii, 0, 0, (T)ii, 0, 0, 0, 0, 0, 0, (T)-ii, 0,
        0, (T)ii, 0;
    // Sigma_z
    cap_sigmas[2].em() << 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1;
    //
    cps_cap_sigmas[0] = -cap_sigmas[0];
    cps_cap_sigmas[1] = cap_sigmas[1];
    cps_cap_sigmas[2] = -cap_sigmas[2];
    //
    for (int a = 0; a < 2; ++a) {
      const SpinMatrixT<T> ma = a == 0 ? unit : gammas[0];
      for (int b = 0; b < 2; ++b) {
        const SpinMatrixT<T> mb =
            b == 0 ? ma : (SpinMatrixT<T>)(ma * gammas[1]);
        for (int c = 0; c < 2; ++c) {
          const SpinMatrixT<T> mc =
              c == 0 ? mb : (SpinMatrixT<T>)(mb * gammas[2]);
          for (int d = 0; d < 2; ++d) {
            const SpinMatrixT<T> md =
                d == 0 ? mc : (SpinMatrixT<T>)(mc * gammas[3]);
            const int idx = a + 2 * b + 4 * c + 8 * d;
            gms[idx] = md;
          }
        }
      }
    }
    for (int a = 0; a < 2; ++a) {
      const SpinMatrixT<T> ma = a == 0 ? unit : cps_gammas[0];
      for (int b = 0; b < 2; ++b) {
        const SpinMatrixT<T> mb =
            b == 0 ? ma : (SpinMatrixT<T>)(ma * cps_gammas[1]);
        for (int c = 0; c < 2; ++c) {
          const SpinMatrixT<T> mc =
              c == 0 ? mb : (SpinMatrixT<T>)(mb * cps_gammas[2]);
          for (int d = 0; d < 2; ++d) {
            const SpinMatrixT<T> md =
                d == 0 ? mc : (SpinMatrixT<T>)(mc * cps_gammas[3]);
            const int idx = a + 2 * b + 4 * c + 8 * d;
            cps_gms[idx] = md;
          }
        }
      }
    }
  }
  //
  static const box<SpinMatrixConstantsT<T> >& get_instance_box()
  {
    static box<SpinMatrixConstantsT<T> > smcs =
        box<SpinMatrixConstantsT<T> >(SpinMatrixConstantsT<T>());
    return smcs;
  }
  //
  static const SpinMatrixConstantsT<T>& get_instance()
  {
    return get_instance_box()();
  }
  //
  static const SpinMatrixT<T>& get_unit() { return get_instance().unit; }
  static const SpinMatrixT<T>& get_gamma(int mu)
  {
    qassert(0 <= mu && mu < 4);
    return get_instance().gammas[mu];
  }
  static const array<SpinMatrixT<T>, 4>& get_gammas()
  {
    return get_instance().gammas;
  }
  static const array<SpinMatrixT<T>, 4>& get_cps_gammas()
  {
    return get_instance().cps_gammas;
  }
  static const array<SpinMatrixT<T>, 16>& get_gms()
  {
    return get_instance().gms;
  }
  static const array<SpinMatrixT<T>, 16>& get_cps_gms()
  {
    return get_instance().cps_gms;
  }
  static const SpinMatrixT<T>& get_gamma5()
  {
    return get_instance().gamma5;
  }
  static const SpinMatrixT<T>& get_cap_sigma(int i)
  {
    return get_instance().cap_sigmas[i];
  }
  static const array<SpinMatrixT<T>, 3>& get_cap_sigmas()
  {
    return get_instance().cap_sigmas;
  }
  static const array<SpinMatrixT<T>, 3>& get_cps_cap_sigmas()
  {
    return get_instance().cps_cap_sigmas;
  }
  static const SpinMatrixT<T> get_cps_sigmas(int i, int j)
  {
    return (get_instance().cps_gammas[i] * get_instance().cps_gammas[j] -
            get_instance().cps_gammas[j] * get_instance().cps_gammas[i]) /
           2.;
  }
};

template <class T>
qacc WilsonMatrixT<T> operator*(const ColorMatrixT<T>& cm,
                                const WilsonMatrixT<T>& m)
{
  WilsonMatrixT<T> ret;
  set_zero(ret);
  for (int s1 = 0; s1 < 4; ++s1) {
    for (int s2 = 0; s2 < 4; ++s2) {
      for (int c1 = 0; c1 < NUM_COLOR; ++c1) {
        for (int c2 = 0; c2 < NUM_COLOR; ++c2) {
          for (int c3 = 0; c3 < NUM_COLOR; ++c3) {
            ret(s1 * NUM_COLOR + c1, s2 * NUM_COLOR + c2) +=
                cm(c1, c3) * m(s1 * NUM_COLOR + c3, s2 * NUM_COLOR + c2);
          }
        }
      }
    }
  }
  return ret;
}

template <class T>
qacc WilsonMatrixT<T> operator*(const WilsonMatrixT<T>& m,
                                const ColorMatrixT<T>& cm)
{
  return matrix_adjoint((const ColorMatrixT<T>&)matrix_adjoint(cm) *
                        (const WilsonMatrixT<T>&)matrix_adjoint(m));
}

template <class T>
qacc WilsonMatrixT<T> operator*(const SpinMatrixT<T>& sm,
                                const WilsonMatrixT<T>& m)
{
  WilsonMatrixT<T> ret;
  set_zero(ret);
  for (int s1 = 0; s1 < 4; ++s1) {
    for (int s3 = 0; s3 < 4; ++s3) {
      if (sm(s1, s3) == 0.0) {
        continue;
      }
      for (int s2 = 0; s2 < 4; ++s2) {
        for (int c1 = 0; c1 < NUM_COLOR; ++c1) {
          for (int c2 = 0; c2 < NUM_COLOR; ++c2) {
            ret(s1 * NUM_COLOR + c1, s2 * NUM_COLOR + c2) +=
                sm(s1, s3) * m(s3 * NUM_COLOR + c1, s2 * NUM_COLOR + c2);
          }
        }
      }
    }
  }
  return ret;
}

template <class T>
qacc WilsonMatrixT<T> operator*(const WilsonMatrixT<T>& m,
                                const SpinMatrixT<T>& sm)
{
  return matrix_adjoint((const SpinMatrixT<T>&)matrix_adjoint(sm) *
                        (const WilsonMatrixT<T>&)matrix_adjoint(m));
}

template <class T>
qacc Complex matrix_trace(const SpinMatrixT<T>& sm, const WilsonMatrixT<T>& m)
{
  Complex ret = 0;
  for (int s1 = 0; s1 < 4; ++s1) {
    for (int s3 = 0; s3 < 4; ++s3) {
      if (sm(s1, s3) == 0.0) {
        continue;
      }
      for (int c1 = 0; c1 < NUM_COLOR; ++c1) {
        ret += sm(s1, s3) * m(s3 * NUM_COLOR + c1, s1 * NUM_COLOR + c1);
      }
    }
  }
  return ret;
}

template <class T>
qacc Complex matrix_trace(const WilsonMatrixT<T>& m, const SpinMatrixT<T>& sm)
{
  return matrix_trace(sm, m);
}

template <class T>
qacc WilsonVectorT<T> operator*(const ColorMatrixT<T>& cm,
                                const WilsonVectorT<T>& m)
{
  WilsonVectorT<T> ret;
  set_zero(ret);
  for (int s1 = 0; s1 < 4; ++s1) {
    for (int c1 = 0; c1 < NUM_COLOR; ++c1) {
      for (int c2 = 0; c2 < NUM_COLOR; ++c2) {
        ret(s1 * NUM_COLOR + c1) += cm(c1, c2) * m(s1 * NUM_COLOR + c2);
      }
    }
  }
  return ret;
}

template <class T>
qacc WilsonVectorT<T> operator*(const SpinMatrixT<T>& sm,
                                const WilsonVectorT<T>& m)
{
  WilsonVectorT<T> ret;
  set_zero(ret);
  for (int s1 = 0; s1 < 4; ++s1) {
    for (int s2 = 0; s2 < 4; ++s2) {
      if (sm(s1, s2) == 0.0) {
        continue;
      }
      for (int c1 = 0; c1 < NUM_COLOR; ++c1) {
        ret(s1 * NUM_COLOR + c1) += sm(s1, s2) * m(s2 * NUM_COLOR + c1);
      }
    }
  }
  return ret;
}

template <class T>
qacc void convert_mspincolor_from_wm(WilsonMatrixT<T>& msc,
                                     const WilsonMatrixT<T>& wm)
{
  for (int s1 = 0; s1 < 4; ++s1) {
    for (int s2 = 0; s2 < 4; ++s2) {
      for (int c1 = 0; c1 < NUM_COLOR; ++c1) {
        for (int c2 = 0; c2 < NUM_COLOR; ++c2) {
          msc.p[(s1 * 4 + s2) * NUM_COLOR * NUM_COLOR + c1 * NUM_COLOR + c2] =
              wm(s1 * NUM_COLOR + c1, s2 * NUM_COLOR + c2);
        }
      }
    }
  }
}

template <class T>
qacc void convert_wm_from_mspincolor(WilsonMatrixT<T>& wm,
                                     const WilsonMatrixT<T>& msc)
{
  for (int s1 = 0; s1 < 4; ++s1) {
    for (int s2 = 0; s2 < 4; ++s2) {
      for (int c1 = 0; c1 < NUM_COLOR; ++c1) {
        for (int c2 = 0; c2 < NUM_COLOR; ++c2) {
          wm(s1 * NUM_COLOR + c1, s2 * NUM_COLOR + c2) =
              msc.p[(s1 * 4 + s2) * NUM_COLOR * NUM_COLOR + c1 * NUM_COLOR +
                    c2];
        }
      }
    }
  }
}

#ifndef QLAT_NO_DEFAULT_TYPE

typedef ColorMatrixT<> ColorMatrix;

typedef WilsonMatrixT<> WilsonMatrix;

typedef NonRelWilsonMatrixT<> NonRelWilsonMatrix;

typedef SpinMatrixT<> SpinMatrix;

typedef SpinMatrixConstantsT<> SpinMatrixConstants;

#endif

template <int DIMN, class T>
std::string show(const MatrixT<DIMN, T>& m)
{
  std::ostringstream out;
  out << m.em();
  return out.str();
}

}  // namespace qlat
