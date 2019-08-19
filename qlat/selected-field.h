#pragma once

#include <qlat/field-serial-io.h>

namespace qlat
{  //

struct FieldSelection {
  FieldM<int64_t, 1>
      f_rank;  // rank when the points being selected (-1 if not selected)
  long n_per_tslice;  // num points per time slice
  //
  FieldM<long, 1>
      f_local_idx;  // idx of points on this node (-1 if not selected)
  long n_elems;     // num points of this node
  //
  std::vector<int64_t> ranks;   // rank of the selected points
  std::vector<long> indices;    // local indices of selected points
  std::vector<Coordinate> xls;  // local coordinates of selected points
  //
  void init()
  {
    f_rank.init();
    n_per_tslice = 0;
    f_local_idx.init();
    n_elems = 0;
    clear(ranks);
    clear(indices);
    clear(xls);
  }
  //
  FieldSelection() { init(); }
};

inline void mk_field_selection(FieldM<int64_t, 1>& f_rank,
                               const long n_per_tslice_,
                               const Coordinate& total_site, const RngState& rs)
// not selected points has value = -1;
// if n_per_tslice_ <= spatial_vol / 2 --> random select
// else if n_per_tslice_ < 0 -->
// random select spatial_vol / 2 points
// and select all the rest of the points with rank spatial_vol - 1.
{
  TIMER_VERBOSE("mk_field_selection");
  const long spatial_vol = total_site[0] * total_site[1] * total_site[2];
  qassert(n_per_tslice_ == -1 or n_per_tslice_ == spatial_vol or
          (0 < n_per_tslice_ and n_per_tslice_ <= spatial_vol / 2));
  const long n_per_tslice = n_per_tslice_ == -1 or n_per_tslice_ == spatial_vol
                                ? spatial_vol / 2
                                : n_per_tslice_;
  Geometry geo;
  geo.init(total_site, 1);
  f_rank.init();
  f_rank.init(geo);
  qassert(f_rank.geo.is_only_local());
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    f_rank.get_elems(index)[0] = -1;
  }
  std::vector<Field<int64_t> > fs;
  const Coordinate new_size_node = get_default_serial_new_size_node(geo);
  shuffle_field(fs, f_rank, new_size_node);
  qassert(fs.size() <= 1);
  if (fs.size() == 1) {
    // require each tslice is on one node.
    Field<int64_t>& nf = fs[0];
    const Geometry& ngeo = nf.geo;
    qassert(ngeo.multiplicity == 1);
    qassert(new_size_node == ngeo.geon.size_node);
    const int t_start = ngeo.node_site[3] * ngeo.geon.coor_node[3];
    const int t_end = t_start + ngeo.node_site[3];
#pragma omp parallel for
    for (int t = t_start; t < t_end; ++t) {
      RngState rst = rs.split(t);
      for (int i = 0; i < n_per_tslice; ++i) {
        while (true) {
          const Coordinate xg =
              mod(Coordinate(rand_gen(rst), rand_gen(rst), rand_gen(rst), t),
                  total_site);
          const Coordinate xl = ngeo.coordinate_l_from_g(xg);
          if (not ngeo.is_local(xl)) {
            displayln(ssprintf("xl=%s", show(xl).c_str()));
            displayln(ssprintf("xg=%s", show(xg).c_str()));
            displayln(ssprintf("ngeo=%s", show(ngeo).c_str()));
            displayln(ssprintf("total_site=%s", show(total_site).c_str()));
            qassert(false);
          }
          int64_t& val = nf.get_elem(xl);
          if (val == -1) {
            val = i;
            break;
          }
        }
      }
    }
  }
  shuffle_field_back(f_rank, fs, new_size_node);
  if (n_per_tslice_ == -1 or n_per_tslice_ == spatial_vol) {
#pragma omp parallel for
    for (long index = 0; index < geo.local_volume(); ++index) {
      int64_t& rank = f_rank.get_elems(index)[0];
      if (rank < 0) {
        rank = spatial_vol - 1;
      }
    }
  }
}

inline void set_field_selection(FieldSelection& fsel,
                                const FieldM<int64_t, 1>& f_rank,
                                const long n_per_tslice_)
// not selected points has value = -1;
// if n_per_tslice_ <= spatial_vol / 2 --> random select
// else if n_per_tslice_ < 0 -->
// random select spatial_vol / 2 points
// and select all the rest of the points with rank spatial_vol - 1.
{
  TIMER_VERBOSE("set_field_selection");
  const Geometry& geo = f_rank.geo;
  const Coordinate total_site = geo.total_site();
  const long spatial_vol = total_site[0] * total_site[1] * total_site[2];
  qassert(n_per_tslice_ == -1 or n_per_tslice_ == spatial_vol or
          (0 < n_per_tslice_ and n_per_tslice_ <= spatial_vol / 2));
  const long n_per_tslice = n_per_tslice_ == -1 ? spatial_vol : n_per_tslice_;
  qassert(geo.is_only_local());
  fsel.init();
  fsel.f_rank = f_rank;
  fsel.n_per_tslice = n_per_tslice;
  fsel.f_local_idx.init(geo);
  long n_elems = 0;
  for (long index = 0; index < geo.local_volume(); ++index) {
    int64_t& rank = fsel.f_rank.get_elems(index)[0];
    long& idx = fsel.f_local_idx.get_elems(index)[0];
    if (n_per_tslice == spatial_vol and rank < 0) {
      rank = spatial_vol - 1;
    }
    if (0 <= rank and rank < n_per_tslice) {
      idx = n_elems;
      n_elems += 1;
    } else {
      idx = -1;
    }
  }
  fsel.n_elems = n_elems;
  fsel.ranks.resize(fsel.n_elems);
  fsel.indices.resize(fsel.n_elems);
  fsel.xls.resize(fsel.n_elems);
#pragma omp parallel for
  for (long index = 0; index < geo.local_volume(); ++index) {
    const long idx = fsel.f_local_idx.get_elems_const(index)[0];
    if (idx >= 0) {
      const long rank = fsel.f_rank.get_elems(index)[0];
      const Coordinate xl = geo.coordinate_from_index(index);
      fsel.ranks[idx] = rank;
      fsel.indices[idx] = index;
      fsel.xls[idx] = xl;
    }
  }
}

inline void set_field_selection(FieldSelection& fsel,
                                const Coordinate& total_site,
                                const long n_per_tslice, const RngState& rs)
{
  FieldM<int64_t, 1> f;
  mk_field_selection(f, n_per_tslice, total_site, rs);
  set_field_selection(fsel, f, n_per_tslice);
}

inline void write_field_selection(const FieldSelection& fsel,
                                  const std::string& path)
{
  TIMER_VERBOSE("write_field_selection");
  write_field_64(fsel.f_rank, path);
}

inline long read_field_selection(FieldSelection& fsel, const std::string& path,
                                 const long n_per_tslice)
{
  TIMER_VERBOSE("read_field_selection");
  fsel.init();
  FieldM<int64_t, 1> f_rank;
  const long total_bytes = read_field_64(f_rank, path);
  if (total_bytes > 0) {
    set_field_selection(fsel, f_rank, n_per_tslice);
  }
  return total_bytes;
}

template <class M>
struct SelectedField {
  bool initialized;
  Geometry geo;
  std::vector<M> field;
  //
  void init()
  {
    initialized = false;
    geo.init();
    clear(field);
  }
  //
  SelectedField() { init(); }
};

template <class M>
void only_keep_selected_points(Field<M>& f, const FieldSelection& fsel)
{
  TIMER("only_keep_selected_points");
  qassert(f.geo.is_only_local());
  qassert(fsel.f_local_idx.geo.is_only_local());
  qassert(geo_remult(f.geo) == geo_remult(fsel.f_local_idx.geo));
  const Geometry& geo = f.geo;
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
void set_selected_field_slow(SelectedField<M>& sf, const Field<M>& f,
                             const FieldSelection& fsel)
{
  TIMER("set_selected_field_slow");
  qassert(f.geo.is_only_local());
  qassert(fsel.f_local_idx.geo.is_only_local());
  qassert(geo_remult(f.geo) == fsel.f_local_idx.geo);
  const Geometry& geo = f.geo;
  sf.init();
  sf.initialized = true;
  sf.geo = geo;
  const int multiplicity = geo.multiplicity;
  sf.field.resize(fsel.n_elems * multiplicity);
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
void set_selected_field(SelectedField<M>& sf, const Field<M>& f,
                        const FieldSelection& fsel)
{
  TIMER("set_selected_field");
  qassert(f.geo.is_only_local());
  qassert(fsel.f_local_idx.geo.is_only_local());
  qassert(geo_remult(f.geo) == fsel.f_local_idx.geo);
  const Geometry& geo = f.geo;
  sf.init();
  sf.initialized = true;
  sf.geo = geo;
  const int multiplicity = geo.multiplicity;
  sf.field.resize(fsel.n_elems * multiplicity);
#pragma omp parallel for
  for (long idx = 0; idx < fsel.n_elems; ++idx) {
    const long index = fsel.indices[idx];
    const Vector<M> fv = f.get_elems_const(index);
    const long offset = idx * multiplicity;
    for (int m = 0; m < multiplicity; ++m) {
      sf.field[offset + m] = fv[m];
    }
  }
}

template <class M>
void set_field_selected_slow(Field<M>& f, const SelectedField<M>& sf,
                             const FieldSelection& fsel)
{
  TIMER("set_field_selected_slow");
  qassert(sf.geo.is_only_local());
  qassert(fsel.f_local_idx.geo.is_only_local());
  qassert(geo_remult(sf.geo) == fsel.f_local_idx.geo);
  const Geometry& geo = sf.geo;
  f.init();
  f.init(sf.geo);
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

template <class M>
void set_field_selected(Field<M>& f, const SelectedField<M>& sf,
                        const FieldSelection& fsel)
{
  TIMER("set_field_selected");
  qassert(sf.geo.is_only_local());
  qassert(fsel.f_local_idx.geo.is_only_local());
  qassert(geo_remult(sf.geo) == fsel.f_local_idx.geo);
  const Geometry& geo = sf.geo;
  f.init();
  f.init(sf.geo);
  set_zero(f);
  const int multiplicity = geo.multiplicity;
#pragma omp parallel for
  for (long idx = 0; idx < fsel.n_elems; ++idx) {
    const long index = fsel.indices[idx];
    Vector<M> fv = f.get_elems(index);
    const long offset = idx * multiplicity;
    for (int m = 0; m < multiplicity; ++m) {
      fv[m] = sf.field[offset + m];
    }
  }
}

inline std::string make_selected_field_header(const Geometry& geo,
                                              const long n_per_tslice,
                                              const int sizeof_M,
                                              const crc32_t crc32)
{
  const Coordinate total_site = geo.total_site();
  std::ostringstream out;
  // const std::string todo = "NOT yet implemented";
  out << "BEGIN_SELECTED_FIELD_HEADER" << std::endl;
  out << "selected_field_version = 1.0" << std::endl;
  out << "total_site[0] = " << total_site[0] << std::endl;
  out << "total_site[1] = " << total_site[1] << std::endl;
  out << "total_site[2] = " << total_site[2] << std::endl;
  out << "total_site[3] = " << total_site[3] << std::endl;
  out << "n_per_tslice = " << n_per_tslice << std::endl;
  out << "multiplicity = " << geo.multiplicity << std::endl;
  out << "sizeof(M) = " << sizeof_M << std::endl;
  out << ssprintf("selected_field_crc32 = %08X", crc32) << std::endl;
  out << "END_HEADER" << std::endl;
  return out.str();
}

template <class M>
long write_selected_field(
    const Field<M>& f, const std::string& path,
    const FieldSelection& fsel, const Coordinate& new_size_node_ = Coordinate())
{
  TIMER_VERBOSE_FLOPS("write_selected_field");
  displayln_info(fname + ssprintf(": fn='%s'.", path.c_str()));
  const Geometry geo = geo_resize(f.geo);
  const Coordinate total_site = geo.total_site();
  qassert(fsel.f_rank.geo == geo_remult(geo));
  const Coordinate new_size_node = new_size_node_ != Coordinate()
                                       ? new_size_node_
                                       : get_default_serial_new_size_node(geo);
  qassert(new_size_node[0] == 1);
  qassert(new_size_node[1] == 1);
  qassert(new_size_node[2] == 1);
  std::vector<Field<int64_t> > fs_rank;
  shuffle_field(fs_rank, fsel.f_rank, new_size_node);
  std::vector<FieldSelection> fsels(fs_rank.size());
  for (int i = 0; i < (int)fs_rank.size(); ++i) {
    FieldM<int64_t, 1> fs_rank_i;
    fs_rank_i.init(fs_rank[i]);
    set_field_selection(fsels[i], fs_rank_i, fsel.n_per_tslice);
  }
  qassert(fsels.size() == fs_rank.size());
  clear(fs_rank);
  std::vector<Field<M> > fs;
  shuffle_field(fs, f, new_size_node);
  qassert(fs.size() == fsels.size());
  std::vector<SelectedField<M> > sfs(fsels.size());
  for (int i = 0; i < (int)fsels.size(); ++i) {
    set_selected_field(sfs[i], fs[i], fsels[i]);
  }
  qassert(sfs.size() == fsels.size());
  clear(fsels);
  clear(fs);
  const int new_num_node = product(new_size_node);
  crc32_t crc = 0;
  for (int i = 0; i < (int)sfs.size(); ++i) {
    const int new_id_node = sfs[i].geo.geon.id_node;
    qassert(sfs[i].geo.geon.num_node == new_num_node);
    const Vector<M> v = get_data(sfs[i].field);
    crc ^= crc32_shift(crc32_par(v),
                       (new_num_node - new_id_node - 1) * v.data_size());
  }
  glb_sum_byte(crc);
  if (get_force_field_write_sizeof_M() == 0) {
    qtouch_info(path + ".partial", make_selected_field_header(
                                       geo, fsel.n_per_tslice, sizeof(M), crc));
  } else {
    const int sizeof_M = get_force_field_write_sizeof_M();
    qassert((geo.multiplicity * sizeof(M)) % sizeof_M == 0);
    const int multiplicity = (geo.multiplicity * sizeof(M)) / sizeof_M;
    qtouch_info(path + ".partial",
                make_selected_field_header(geo_remult(geo, multiplicity),
                                           fsel.n_per_tslice, sizeof_M, crc));
    get_force_field_write_sizeof_M() = 0;
  }
  const int mpi_tag = 8;
  if (get_id_node() == 0) {
    qassert(sfs.size() > 0);
    Vector<M> v = get_data(sfs[0].field);
    FILE* fp = qopen(path + ".partial", "a");
    qassert(fp != NULL);
    const int num_node = get_num_node();
    for (int new_id_node = 0; new_id_node < new_num_node; ++new_id_node) {
      const int id_node =
          get_id_node_from_new_id_node(new_id_node, new_num_node, num_node);
      if (0 == id_node) {
        assign(v, get_data(sfs[new_id_node].field));
      } else {
        MPI_Recv(v.data(), v.data_size(), MPI_BYTE, id_node, mpi_tag,
                 get_comm(), MPI_STATUS_IGNORE);
      }
      qwrite_data(v, fp);
    }
    qclose(fp);
  } else {
    for (size_t i = 0; i < sfs.size(); ++i) {
      const Vector<M> v = get_data(sfs[i].field);
      MPI_Send((void*)v.data(), v.data_size(), MPI_BYTE, 0, mpi_tag,
               get_comm());
    }
  }
  qrename_info(path + ".partial", path);
  const long total_bytes =
      fsel.n_per_tslice * total_site[3] * geo.multiplicity * sizeof(M);
  timer.flops += total_bytes;
  return total_bytes;
}

inline void read_selected_geo_info(Coordinate& total_site, int& multiplicity,
                                   long& n_per_tslice, int& sizeof_M,
                                   crc32_t& crc, const std::string& path)
{
  TIMER("read_geo_info");
  if (get_id_node() == 0) {
    FILE* fp = qopen(path, "r");
    if (fp != NULL) {
      const std::string header = "BEGIN_SELECTED_FIELD_HEADER\n";
      std::vector<char> check_line(header.size(), 0);
      if (1 == fread(check_line.data(), header.size(), 1, fp)) {
        if (std::string(check_line.data(), check_line.size()) == header) {
          std::vector<std::string> infos;
          infos.push_back(header);
          while (infos.back() != "END_HEADER\n" && infos.back() != "") {
            infos.push_back(qgetline(fp));
          }
          for (int m = 0; m < 4; ++m) {
            reads(total_site[m],
                  info_get_prop(infos, ssprintf("total_site[%d] = ", m)));
          }
          reads(multiplicity, info_get_prop(infos, "multiplicity = "));
          reads(n_per_tslice, info_get_prop(infos, "n_per_tslice = "));
          reads(sizeof_M, info_get_prop(infos, "sizeof(M) = "));
          crc = read_crc32(info_get_prop(infos, "selected_field_crc32 = "));
        }
      }
    }
    qclose(fp);
  }
  bcast(Vector<Coordinate>(&total_site, 1));
  bcast(Vector<int>(&multiplicity, 1));
  bcast(Vector<long>(&n_per_tslice, 1));
  bcast(Vector<int>(&sizeof_M, 1));
  bcast(Vector<crc32_t>(&crc, 1));
}

template <class M>
long read_selected_field(Field<M>& f, const std::string& path,
                         const FieldSelection& fsel,
                         const Coordinate& new_size_node_ = Coordinate())
{
  TIMER_VERBOSE_FLOPS("read_selected_field");
  displayln_info(fname + ssprintf(": fn='%s'.", path.c_str()));
  Coordinate total_site;
  int multiplicity = 0;
  long n_per_tslice = 0;
  int sizeof_M = 0;
  crc32_t crc_info = 0;
  read_selected_geo_info(total_site, multiplicity, n_per_tslice, sizeof_M, crc_info, path);
  if (total_site == Coordinate() or multiplicity == 0) {
    displayln_info(fname + ssprintf(": fn='%s' can not be parsed.", path.c_str()));
    return 0;
  }
  qassert(fsel.n_per_tslice == n_per_tslice);
  get_incorrect_field_read_sizeof_M() = 0;
  if (sizeof_M != sizeof(M)) {
    get_incorrect_field_read_sizeof_M() = sizeof_M;
    displayln(fname + ssprintf(": WARNING: sizeof(M) do not match. "
                               "Expected %d, Actual file %d",
                               sizeof(M), sizeof_M));
    qassert((multiplicity * sizeof_M) % sizeof(M) == 0);
    multiplicity = (multiplicity * sizeof_M) / sizeof(M);
  }
  Geometry geo;
  geo.init(total_site, multiplicity);
  f.init();
  f.init(geo, geo.multiplicity);
  qassert(fsel.f_rank.geo == geo_remult(geo));
  const Coordinate new_size_node = new_size_node_ != Coordinate()
                                       ? new_size_node_
                                       : get_default_serial_new_size_node(geo);
  qassert(new_size_node[0] == 1);
  qassert(new_size_node[1] == 1);
  qassert(new_size_node[2] == 1);
  std::vector<Field<int64_t> > fs_rank;
  shuffle_field(fs_rank, fsel.f_rank, new_size_node);
  std::vector<FieldSelection> fsels(fs_rank.size());
  for (int i = 0; i < (int)fs_rank.size(); ++i) {
    FieldM<int64_t, 1> fs_rank_i;
    fs_rank_i.init(fs_rank[i]);
    set_field_selection(fsels[i], fs_rank_i, fsel.n_per_tslice);
  }
  qassert(fsels.size() == fs_rank.size());
  clear(fs_rank);
  std::vector<Field<M> > fs;
  shuffle_field(fs, f, new_size_node);
  qassert(fs.size() == fsels.size());
  std::vector<SelectedField<M> > sfs(fsels.size());
  for (int i = 0; i < (int)fsels.size(); ++i) {
    set_selected_field(sfs[i], fs[i], fsels[i]);
  }
  qassert(sfs.size() == fsels.size());
  const int new_num_node = product(new_size_node);
  crc32_t crc = 0;
  if (sfs.size() > 0) {
    FILE* fp = qopen(path, "r");
    qassert(fp != NULL);
    fseek(
        fp,
        (sfs[0].geo.geon.id_node - new_num_node) * get_data(sfs[0].field).data_size(),
        SEEK_END);
    for (int i = 0; i < (int)sfs.size(); ++i) {
      const int new_id_node = sfs[i].geo.geon.id_node;
      qassert(sfs[i].geo.geon.num_node == new_num_node);
      Vector<M> v = get_data(sfs[i].field);
      qread_data(v, fp);
      crc ^= crc32_shift(crc32_par(v),
                         (new_num_node - new_id_node - 1) * v.data_size());
    }
    qclose(fp);
  }
  glb_sum_byte(crc);
  if (crc != crc_info) {
    displayln_info(
        fname +
        ssprintf(": crc of data = %08X ; crc of header = %08X", crc, crc_info));
    qassert(false);
  }
  for (int i = 0; i < (int)fsels.size(); ++i) {
    set_field_selected(fs[i], sfs[i], fsels[i]);
  }
  shuffle_field_back(f, fs, new_size_node);
  const long total_bytes =
      fsel.n_per_tslice * total_site[3] * geo.multiplicity * sizeof(M);
  timer.flops += total_bytes;
  return total_bytes;
}

template <class M>
long write_selected_field_64(
    const Field<M>& f, const std::string& path,
    const FieldSelection& fsel, const Coordinate& new_size_node_ = Coordinate())
{
  TIMER_VERBOSE_FLOPS("write_selected_field_64");
  Field<M> ff;
  ff.init(f);
  to_from_big_endian_64(get_data(ff));
  const long total_bytes = write_selected_field(ff, path, fsel, new_size_node_);
  timer.flops += total_bytes;
  return total_bytes;
}

template <class M>
long read_selected_field_64(Field<M>& f, const std::string& path,
                            const FieldSelection& fsel,
                            const Coordinate& new_size_node_ = Coordinate())
{
  TIMER_VERBOSE_FLOPS("read_selected_field_64");
  const long total_bytes = read_selected_field(f, path, fsel, new_size_node_);
  if (total_bytes == 0) {
    return 0;
  } else {
    to_from_big_endian_64(get_data(f));
    timer.flops += total_bytes;
    return total_bytes;
  }
}

template <class M>
long write_selected_field_float_from_double(
    const Field<M>& f, const std::string& path, const FieldSelection& fsel,
    const Coordinate& new_size_node_ = Coordinate())
{
  TIMER_VERBOSE_FLOPS("write_selected_field_float_from_double");
  Field<float> ff;
  convert_field_float_from_double(ff, f);
  to_from_big_endian_32(get_data(ff));
  const long total_bytes = write_selected_field(ff, path, fsel, new_size_node_);
  timer.flops += total_bytes;
  return total_bytes;
}

template <class M>
long read_selected_field_double_from_float(
    Field<M>& f, const std::string& path, const FieldSelection& fsel,
    const Coordinate& new_size_node_ = Coordinate())
{
  TIMER_VERBOSE_FLOPS("read_selected_field_double_from_float");
  Field<float> ff;
  const long total_bytes = read_selected_field(ff, path, fsel, new_size_node_);
  if (total_bytes == 0) {
    return 0;
  } else {
    to_from_big_endian_32(get_data(ff));
    convert_field_double_from_float(f, ff);
    timer.flops += total_bytes;
    return total_bytes;
  }
}

inline bool is_selected_field(const std::string& path)
{
  TIMER("is_selected_field");
  long nfile = 0;
  if (get_id_node() == 0) {
    FILE* fp = qopen(path, "r");
    if (fp != NULL) {
      const std::string header = "BEGIN_SELECTED_FIELD_HEADER\n";
      std::vector<char> check_line(header.size(), 0);
      if (1 == fread(check_line.data(), header.size(), 1, fp)) {
        if (std::string(check_line.data(), check_line.size()) == header) {
          nfile = 1;
        }
      }
    }
    qclose(fp);
  }
  bcast(get_data(nfile));
  return nfile > 0;
}

}  // namespace qlat