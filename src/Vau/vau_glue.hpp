//
//  vau_glue.hpp
//  VauQt
//
//  Created by Massimiliano Gubinelli on 28/12/2022.
//

#ifndef VAU_GLUE_H
#define VAU_GLUE_H

#include "glue.hpp"
#include <pscm/Function.h>
#include <pscm/Scheme.h>
// conversion generics
template<typename T0> tmscm tmscm_from (T0 out);
template<typename T0> T0 tmscm_to (SCM in);

class glue_function;
extern pscm::Scheme* scm;
class glue_function_rep : concrete_struct {
  const char *name;
  pscm::Function::ScmFunc2 fn;
  int arity;
  static list<glue_function> glue_functions;
protected:
  glue_function_rep (const char *_name, pscm::Function::ScmFunc2 _fn, int _ar);
  void instantiate () {
    // tmscm_install_procedure (name, fn, arity, 0, 0);
    PSCM_ASSERT(scm);
    scm->add_func(new pscm::Symbol(name), new pscm::Function(name, fn));
  }
public:
  static void instantiate_all ();
  friend class glue_function;
};

class glue_function {
  CONCRETE(glue_function);
  glue_function (glue_function_rep *_rep = nullptr) : rep(_rep) {}
};
CONCRETE_CODE(glue_function);

glue_function_rep::glue_function_rep (const char *_name, pscm::Function::ScmFunc2 _fn, int _ar)
  : name (_name), fn (_fn), arity (_ar) {
  glue_functions= list<glue_function> (this, glue_functions);
}

// adaptor template class
template<typename fn, auto f> struct tm_glue  {
  // we do not provide constructor to detect matching errors
};
template<auto f, typename ReturnType>
struct tm_glue<ReturnType(), f>: public glue_function_rep {
  static tmscm func (pscm::Cell args, pscm::SourceLocation loc) {
    if constexpr (std::is_void_v<ReturnType>) {
      f();
      return pscm::Cell::none();
    }
    else {
      ReturnType ret = f();
      return tmscm_from<ReturnType>(ret);
    }
  }
  tm_glue (const char *_name) : glue_function_rep (_name, func, 0) {}
};

template<auto f, typename ReturnType, typename T1>
struct tm_glue<ReturnType(T1), f>: public glue_function_rep {
  static tmscm func (pscm::Cell args, pscm::SourceLocation loc) {
      T1 arg1 = tmscm_to<T1>(car(args));
    if constexpr (std::is_void_v<ReturnType>) {
      f(arg1);
      return pscm::Cell::none();
    }
    else {
      ReturnType ret = f(arg1);
      return tmscm_from<ReturnType>(ret);
    }
  }
  tm_glue (const char *_name) : glue_function_rep (_name, func, 0) {}
};

template<auto f, typename ReturnType, typename T1, typename T2>
struct tm_glue<ReturnType(T1, T2), f>: public glue_function_rep {
  static tmscm func (pscm::Cell args, pscm::SourceLocation loc) {
      T1 arg1 = tmscm_to<T1>(car(args));
      T2 arg2 = tmscm_to<T2>(cadr(args));
    if constexpr (std::is_void_v<ReturnType>) {
      f(arg1, arg2);
      return pscm::Cell::none();
    }
    else {
      ReturnType ret = f(arg1, arg2);
      return tmscm_from<ReturnType>(ret);
    }
  }
  tm_glue (const char *_name) : glue_function_rep (_name, func, 0) {}
};

template<auto f, typename ReturnType, typename T1, typename T2, typename T3>
struct tm_glue<ReturnType(T1, T2, T3), f>: public glue_function_rep {
  static tmscm func (pscm::Cell args, pscm::SourceLocation loc) {
      T1 arg1 = tmscm_to<T1>(car(args));
      T2 arg2 = tmscm_to<T2>(cadr(args));
      T3 arg3 = tmscm_to<T3>(caddr(args));
    if constexpr (std::is_void_v<ReturnType>) {
      f(arg1, arg2, arg3);
      return pscm::Cell::none();
    }
    else {
      ReturnType ret = f(arg1, arg2, arg3);
      return tmscm_from<ReturnType>(ret);
    }
  }
  tm_glue (const char *_name) : glue_function_rep (_name, func, 0) {}
};

template<auto f, typename ReturnType, typename T1, typename T2, typename T3, typename T4>
struct tm_glue<ReturnType(T1, T2, T3, T4), f>: public glue_function_rep {
  static tmscm func (pscm::Cell args, pscm::SourceLocation loc) {
      T1 arg1 = tmscm_to<T1>(car(args));
      T2 arg2 = tmscm_to<T2>(cadr(args));
      T3 arg3 = tmscm_to<T3>(caddr(args));
      T4 arg4 = tmscm_to<T4>(cadddr(args));
    if constexpr (std::is_void_v<ReturnType>) {
      f(arg1, arg2, arg3, arg4);
      return pscm::Cell::none();
    }
    else {
      ReturnType ret = f(arg1, arg2, arg3, arg4);
      return tmscm_from<ReturnType>(ret);
    }
  }
  tm_glue (const char *_name) : glue_function_rep (_name, func, 0) {}
};

template<auto f, typename ReturnType, typename T1, typename T2, typename T3, typename T4, typename T5>
struct tm_glue<ReturnType(T1, T2, T3, T4, T5), f>: public glue_function_rep {
  static tmscm func (pscm::Cell args, pscm::SourceLocation loc) {
      T1 arg1 = tmscm_to<T1>(car(args));
      T2 arg2 = tmscm_to<T2>(cadr(args));
      T3 arg3 = tmscm_to<T3>(caddr(args));
      T4 arg4 = tmscm_to<T4>(cadddr(args));
      args = cdddr(args);
      T5 arg5 = tmscm_to<T5>(cadr(args));
    if constexpr (std::is_void_v<ReturnType>) {
      f(arg1, arg2, arg3, arg4, arg5);
      return pscm::Cell::none();
    }
    else {
      ReturnType ret = f(arg1, arg2, arg3, arg4, arg5);
      return tmscm_from<ReturnType>(ret);
    }
  }
  tm_glue (const char *_name) : glue_function_rep (_name, func, 0) {}
};

class scheme_tree_t;

template<typename T0, typename S0, S0 fn> glue_function
declare_glue (const char *_name) {
  auto p = tm_new<tm_glue<S0, fn> > (_name);
  // auto p = new tm_glue<S0, fn>(_name);
  auto pp = (glue_function_rep*)p;
  return pp;
}

// to implement unique labels for static variables in DECLARE_GLUE_NAME_TYPE
#define DECLARE_GLUE_CONCAT(a, b) DECLARE_GLUE_CONCAT_INNER(a, b)
#define DECLARE_GLUE_CONCAT_INNER(a, b) a ## b

// declarations macros
#define DECLARE_GLUE_NAME_TYPE_BASE(FUNC, NAME, TYPE, BASE) \
  glue_function DECLARE_GLUE_CONCAT(glue_decl_##FUNC,__COUNTER__) (declare_glue<TYPE, BASE, FUNC> (NAME));
#define DECLARE_GLUE_NAME_TYPE(FUNC, NAME, TYPE) DECLARE_GLUE_NAME_TYPE_BASE(FUNC, NAME, TYPE, decltype(FUNC))
#define DECLARE_GLUE_NAME_BASE(FUNC, NAME, TYPE) DECLARE_GLUE_NAME_TYPE_BASE(FUNC, NAME, TYPE, TYPE)
#define DECLARE_GLUE_NAME(FUNC, NAME) DECLARE_GLUE_NAME_TYPE(FUNC, NAME, decltype(FUNC))
#define DECLARE_GLUE(FUNC) DECLARE_GLUE_NAME(FUNC, #FUNC)

// old stuff
// glue_function glue_decl_##FUNC (declare_glue<decltype(FUNC), FUNC> (NAME));
// static glue_function glue_decl_##FUNC (declare_glue<decltype(FUNC), FUNC> (#FUNC));

#endif /* VAU_GLUE_H */
