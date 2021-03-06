#include "lib.h"

EXPORT(qremove, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const int ret = qremove(path);
  return py_convert(ret);
});

EXPORT(qremove_info, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const int ret = qremove_info(path);
  return py_convert(ret);
});

EXPORT(qremove_all, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const int ret = qremove_all(path);
  return py_convert(ret);
});

EXPORT(qremove_all_info, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const int ret = qremove_all_info(path);
  return py_convert(ret);
});

EXPORT(qmkdir, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const int ret = qmkdir(path);
  return py_convert(ret);
});

EXPORT(qmkdir_info, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const int ret = qmkdir_info(path);
  return py_convert(ret);
});

EXPORT(qmkdir_sync_node, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const int ret = qmkdir_sync_node(path);
  return py_convert(ret);
});

EXPORT(obtain_lock, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const bool ret = obtain_lock(path);
  return py_convert(ret);
});

EXPORT(release_lock, {
  using namespace qlat;
  release_lock();
  Py_RETURN_NONE;
});

EXPORT(does_file_exist, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const bool ret = does_file_exist(path);
  return py_convert(ret);
});

EXPORT(does_file_exist_sync_node, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const bool ret = does_file_exist_sync_node(path);
  return py_convert(ret);
});

EXPORT(is_directory, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const bool ret = is_directory(path);
  return py_convert(ret);
});

EXPORT(is_directory_sync_node, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const bool ret = is_directory_sync_node(path);
  return py_convert(ret);
});

EXPORT(qtouch, {
  using namespace qlat;
  PyObject* p_path = NULL;
  PyObject* p_content = NULL;
  if (!PyArg_ParseTuple(args, "O|O", &p_path, &p_content)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  if (p_content == NULL) {
    const int ret = qtouch(path);
    return py_convert(ret);
  } else {
    std::string content;
    py_convert(content, p_content);
    const int ret = qtouch(path, content);
    return py_convert(ret);
  }
});

EXPORT(qtouch_info, {
  using namespace qlat;
  PyObject* p_path = NULL;
  PyObject* p_content = NULL;
  if (!PyArg_ParseTuple(args, "O|O", &p_path, &p_content)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  if (p_content == NULL) {
    const int ret = qtouch_info(path);
    return py_convert(ret);
  } else {
    std::string content;
    py_convert(content, p_content);
    const int ret = qtouch_info(path, content);
    return py_convert(ret);
  }
});

EXPORT(qappend, {
  using namespace qlat;
  PyObject* p_path = NULL;
  PyObject* p_content = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_path, &p_content)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  std::string content;
  py_convert(content, p_content);
  const int ret = qappend(path, content);
  return py_convert(ret);
});

EXPORT(qappend_info, {
  using namespace qlat;
  PyObject* p_path = NULL;
  PyObject* p_content = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_path, &p_content)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  std::string content;
  py_convert(content, p_content);
  const int ret = qappend_info(path, content);
  return py_convert(ret);
});

EXPORT(qrename, {
  using namespace qlat;
  PyObject* p_old_path = NULL;
  PyObject* p_new_path = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_old_path, &p_new_path)) {
    return NULL;
  }
  std::string old_path, new_path;
  py_convert(old_path, p_old_path);
  py_convert(new_path, p_new_path);
  const int ret = qrename(old_path, new_path);
  return py_convert(ret);
});

EXPORT(qrename_info, {
  using namespace qlat;
  PyObject* p_old_path = NULL;
  PyObject* p_new_path = NULL;
  if (!PyArg_ParseTuple(args, "OO", &p_old_path, &p_new_path)) {
    return NULL;
  }
  std::string old_path, new_path;
  py_convert(old_path, p_old_path);
  py_convert(new_path, p_new_path);
  const int ret = qrename_info(old_path, new_path);
  return py_convert(ret);
});

EXPORT(qls, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const std::vector<std::string> ret = qls(path);
  return py_convert(ret);
});

EXPORT(qls_sync_node, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const std::vector<std::string> ret = qls_sync_node(path);
  return py_convert(ret);
});

EXPORT(qcat, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const std::string ret = qcat(path);
  return py_convert(ret);
});

EXPORT(qcat_sync_node, {
  using namespace qlat;
  PyObject* p_path = NULL;
  if (!PyArg_ParseTuple(args, "O", &p_path)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const std::string ret = qcat_sync_node(path);
  return py_convert(ret);
});

EXPORT(qload_datatable, {
  using namespace qlat;
  PyObject* p_path = NULL;
  bool is_par = false;
  if (!PyArg_ParseTuple(args, "O|O", &p_path, &is_par)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const DataTable dt = qload_datatable(path, is_par);
  return py_convert(dt);
});

EXPORT(qload_datatable_sync_node, {
  using namespace qlat;
  PyObject* p_path = NULL;
  bool is_par = false;
  if (!PyArg_ParseTuple(args, "O|b", &p_path, &is_par)) {
    return NULL;
  }
  std::string path;
  py_convert(path, p_path);
  const DataTable dt = qload_datatable_sync_node(path, is_par);
  return py_convert(dt);
});

EXPORT(check_time_limit, {
  using namespace qlat;
  check_time_limit();
  Py_RETURN_NONE;
});

EXPORT(check_stop, {
  using namespace qlat;
  check_stop();
  Py_RETURN_NONE;
});
