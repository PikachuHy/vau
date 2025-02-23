//
//  vau_glue.hpp
//  VauQt
//
//  Created by Massimiliano Gubinelli on 28/12/2022.
//

#ifndef VAU_GLUE_H
#define VAU_GLUE_H

#include "glue.hpp"

// conversion generics
template<typename T0> tmscm tmscm_from (T0 out);
template<typename T0> T0 tmscm_to (SCM in);

class glue_function;

class glue_function_rep : concrete_struct {
  const char *name;
  FN fn;
  int arity;
  static list<glue_function> glue_functions;
protected:
  glue_function_rep (const char *_name, FN _fn, int _ar);
  void instantiate () {
    tmscm_install_procedure (name, fn, arity, 0, 0);
  }
public:
  static void instantiate_all ();
  friend class glue_function;
};

class glue_function {
  CONCRETE(glue_function);
  glue_function (glue_function_rep *_rep) : rep(_rep) {}
};
CONCRETE_CODE(glue_function);

glue_function_rep::glue_function_rep (const char *_name, FN _fn, int _ar)
  : name (_name), fn (_fn), arity (_ar) {
  glue_functions= list<glue_function> (this, glue_functions);
}

// adaptor template class
template<typename T0, typename S0, S0 fn> struct tm_glue  {
  // we do not provide constructor to detect matching errors
};

template<typename S0, S0 f, typename ... Ts>
struct tm_glue<void (*) (Ts ...), S0, f> : public glue_function_rep {
  template<typename A> struct Arg { typedef tmscm Type; };
  static void wrap (Ts ... args) {
    f (args ...);
  }
  static tmscm func (typename Arg<Ts>::Type ... args) {
    wrap (tmscm_to<Ts> (args) ...);
    return TMSCM_UNSPECIFIED;
  }
  tm_glue (const char *_name) : glue_function_rep (_name, (FN)func, sizeof...(Ts)) {}
};

class scheme_tree_t;

template<typename S0, S0 f, typename T0, typename ... Ts>
struct tm_glue<T0 (*) (Ts ...), S0, f> : public glue_function_rep {
  template<typename A> struct Arg { typedef tmscm Type; };
  template<typename A> struct Res { typedef A Type; };
  template<> struct Res<scheme_tree_t> { typedef scheme_tree Type; };

  static typename Res<T0>::Type wrap (Ts ... args) {
    return f (args ...);
  }
  static tmscm func (typename Arg<Ts>::Type ... args) {
    T0 out= wrap (tmscm_to<Ts> (args) ...);
    return tmscm_from<T0> (out);
  }
  tm_glue (const char *_name) : glue_function_rep (_name, (FN)func, sizeof...(Ts)) {}
};
#if 1
template<typename S0, S0 f>
struct tm_glue<void (*) (), S0, f> : public glue_function_rep {
  template<typename A> struct Arg { typedef tmscm Type; };
  static void wrap () {
    f ();
  }
  static tmscm func (tmscm args) {
    wrap ();
    return TMSCM_UNSPECIFIED;
  }
  tm_glue (const char *_name) : glue_function_rep (_name, (FN)func, 0) {}
};

class scheme_tree_t;

template<typename S0, S0 f, typename T0>
struct tm_glue<T0 (*) (), S0, f> : public glue_function_rep {
  template<typename A> struct Arg { typedef tmscm Type; };
  template<typename A> struct Res { typedef A Type; };
  template<> struct Res<scheme_tree_t> { typedef scheme_tree Type; };

  static typename Res<T0>::Type wrap () {
    return f ();
  }
  static tmscm func (tmscm args) {
    T0 out= wrap ();
    return tmscm_from<T0> (out);
  }
  tm_glue (const char *_name) : glue_function_rep (_name, (FN)func, 0) {}
};

template<typename S0, S0 f, typename T1>
struct tm_glue<void (*) (T1), S0, f> : public glue_function_rep {
  template<typename A> struct Arg { typedef tmscm Type; };
  static void wrap (T1 args) {
    f (args);
  }
  static tmscm func (tmscm args) {
    auto arg1 = car(args);
    wrap (tmscm_to<T1> (arg1));
    return TMSCM_UNSPECIFIED;
  }
  tm_glue (const char *_name) : glue_function_rep (_name, (FN)func, 1) {}
};

class scheme_tree_t;

template<typename S0, S0 f, typename T0, typename T1>
struct tm_glue<T0 (*) (T1), S0, f> : public glue_function_rep {
  template<typename A> struct Arg { typedef tmscm Type; };
  template<typename A> struct Res { typedef A Type; };
  template<> struct Res<scheme_tree_t> { typedef scheme_tree Type; };

  static typename Res<T0>::Type wrap (T1 args) {
    return f (args);
  }
  static tmscm func (tmscm args) {
    auto arg1 = car(args);
    T0 out= wrap (tmscm_to<T1> (arg1));
    return tmscm_from<T0> (out);
  }
  tm_glue (const char *_name) : glue_function_rep (_name, (FN)func, 1) {}
};

template<typename S0, S0 f, typename T1, typename T2>
struct tm_glue<void (*) (T1, T2), S0, f> : public glue_function_rep {
  template<typename A> struct Arg { typedef tmscm Type; };
  static void wrap (T1 arg1, T2 arg2) {
    f (arg1, arg2);
  }
  static tmscm func (tmscm args) {
    auto arg1 = car(args);
    auto arg2 = cadr(args);
    wrap (tmscm_to<T1> (arg1), tmscm_to<T2>(arg2));
    return TMSCM_UNSPECIFIED;
  }
  tm_glue (const char *_name) : glue_function_rep (_name, (FN)func, 1) {}
};

class scheme_tree_t;

template<typename S0, S0 f, typename T0, typename T1, typename T2>
struct tm_glue<T0 (*) (T1, T2), S0, f> : public glue_function_rep {
  template<typename A> struct Arg { typedef tmscm Type; };
  template<typename A> struct Res { typedef A Type; };
  template<> struct Res<scheme_tree_t> { typedef scheme_tree Type; };

  static typename Res<T0>::Type wrap (T1 arg1, T2 arg2) {
    return f (arg1, arg2);
  }
  static tmscm func (tmscm args) {
    auto arg1 = car(args);
    auto arg2 = cadr(args);
    T0 out= wrap (tmscm_to<T1> (arg1), tmscm_to<T2>(arg2));
    return tmscm_from<T0> (out);
  }
  tm_glue (const char *_name) : glue_function_rep (_name, (FN)func, 1) {}
};
#endif
template<typename T0, typename S0, S0 fn> glue_function
declare_glue (const char *_name) {
  return tm_new<tm_glue<T0, S0, fn> > (_name);
}

// to implement unique labels for static variables in DECLARE_GLUE_NAME_TYPE
#define DECLARE_GLUE_CONCAT(a, b) DECLARE_GLUE_CONCAT_INNER(a, b)
#define DECLARE_GLUE_CONCAT_INNER(a, b) a ## b

// declarations macros
#define DECLARE_GLUE_NAME_TYPE_BASE(FUNC, NAME, TYPE, BASE) \
  glue_function DECLARE_GLUE_CONCAT(glue_decl_##FUNC,__COUNTER__) (declare_glue<TYPE, BASE, FUNC> (NAME));
#define DECLARE_GLUE_NAME_TYPE(FUNC, NAME, TYPE) DECLARE_GLUE_NAME_TYPE_BASE(FUNC, NAME, TYPE, TYPE)
#define DECLARE_GLUE_NAME_TYPE2(FUNC, NAME, TYPE) DECLARE_GLUE_NAME_TYPE_BASE(FUNC, NAME, TYPE, decltype(FUNC))
// #define DECLARE_GLUE_NAME_TYPE2(FUNC, NAME, TYPE,  BASE) DECLARE_GLUE_NAME_TYPE_BASE(FUNC, NAME, TYPE, decltype(FUNC))
#define DECLARE_GLUE_NAME_BASE(FUNC, NAME, TYPE) DECLARE_GLUE_NAME_TYPE_BASE(FUNC, NAME, TYPE, TYPE)
#define DECLARE_GLUE_NAME(FUNC, NAME) DECLARE_GLUE_NAME_TYPE(FUNC, NAME, decltype(FUNC))
#define DECLARE_GLUE(FUNC) DECLARE_GLUE_NAME(FUNC, #FUNC)

// old stuff
// glue_function glue_decl_##FUNC (declare_glue<decltype(FUNC), FUNC> (NAME));
// static glue_function glue_decl_##FUNC (declare_glue<decltype(FUNC), FUNC> (#FUNC));

#endif /* VAU_GLUE_H */
