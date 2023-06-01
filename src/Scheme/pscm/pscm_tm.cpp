
#ifdef OS_MINGW
  //FIXME: if this include is not here we have compilation problems on mingw32
  //       (probably name clashes with Windows headers)
  //#include "tree.hpp"
#endif
  //#include "Glue/glue.hpp"

#include "pscm_tm.hpp"
#include "blackbox.hpp"
#include "file.hpp"
#include "../Scheme/glue.hpp"
#include "convert.hpp" // tree_to_texmacs (should not belong here)
#include <pscm/common_def.h>
#include <pscm/scm_utils.h>
#include <pscm/Str.h>
#include <pscm/Scheme.h>
/******************************************************************************
 * Installation of guile and initialization of guile
 ******************************************************************************/
bool scm_busy= false;

#if (defined(GUILE_C) || defined(GUILE_D))
static void (*old_call_back) (int, char**)= NULL;
static void
new_call_back (void *closure, int argc, char** argv) {
  (void) closure;
  
  old_call_back (argc, argv);
}
#endif


int guile_argc;
char **guile_argv;
pscm::Scheme* scm;
void
start_scheme (int argc, char** argv, void (*call_back) (int, char**)) {
//   guile_argc = argc;
//   guile_argv = argv;
// #if (defined(GUILE_C) || defined(GUILE_D))
//   old_call_back= call_back;
//   scm_boot_guile (argc, argv, new_call_back, 0);
// #else
// #ifdef DOTS_OK
//   gh_enter (argc, argv, (void (*)(...)) ((void*) call_back));
// #else
//   gh_enter (argc, argv, call_back);
// #endif
// #endif
  call_back(argc, argv);
}



/******************************************************************************
 * Catching errors (with thanks to Dale P. Smith)
 ******************************************************************************/

tmscm
TeXmacs_lazy_catcher (void *data, SCM tag, SCM throw_args) {
  // SCM eport= scm_current_error_port();
  // scm_handle_by_message_noexit (data, tag, throw_args);
  // scm_force_output (eport);
  // scm_ithrow (tag, throw_args, 1);
  // return SCM_UNSPECIFIED; /* never returns */
  return pscm::Cell::none();
}

tmscm
TeXmacs_catcher (void *data, SCM tag, SCM args) {
  // (void) data;
  // return scm_cons (tag, args);
  return pscm::Cell::none();
}

/******************************************************************************
 * Evaluation of files
 ******************************************************************************/

#ifndef DEBUG_ON
static tmscm
TeXmacs_lazy_eval_file (char *file) {
  // return scm_internal_lazy_catch (SCM_BOOL_T,
                                  // (scm_t_catch_body) scm_c_primitive_load, file,
                                  // (scm_t_catch_handler) TeXmacs_lazy_catcher, file);
    return pscm::Cell::none();
}
#endif

static tmscm
TeXmacs_eval_file (char *file) {
  bool ok = scm->load(file);
  return pscm::Cell(ok);
}

tmscm
eval_scheme_file (string file) {
    //static int cumul= 0;
    //timer tm;
  if (DEBUG_STD) debug_std << "Evaluating " << file << "...\n";
  c_string _file (file);
  SCM result= TeXmacs_eval_file (_file);
    //int extra= tm->watch (); cumul += extra;
    //cout << extra << "\t" << cumul << "\t" << file << "\n";
  return result;
}

/******************************************************************************
 * Evaluation of strings
 ******************************************************************************/

#ifndef DEBUG_ON
static tmscm
TeXmacs_lazy_eval_string (char *s) {
  PSCM_THROW_EXCEPTION("not supported now");
  return pscm::Cell::none();
}
#endif

static tmscm
TeXmacs_eval_string (char *s) {
  auto ret = scm->eval(s);
return ret;
}

tmscm
eval_scheme (string s) {
    // cout << "Eval] " << s << "\n";
#ifdef DEBUG_ON
if ( ! scm_busy) {
#endif
  c_string _s (s);
  SCM result= TeXmacs_eval_string (_s);
  return result;
#ifdef DEBUG_ON
  } else return SCM_BOOL_F;
#endif
}

/******************************************************************************
 * Using scheme objects as functions
 ******************************************************************************/

struct arg_list { int  n; SCM* a; };

static tmscm
TeXmacs_call (arg_list* args) {
  switch (args->n) {
    case 0: return scm->eval(pscm::list(args->a[0])); break;
    case 1: return scm->eval(pscm::list(args->a[0], args->a[1])); break;
    case 2: return scm->eval(pscm::list(args->a[0], args->a[1], args->a[2])); break;
    case 3:
      return scm->eval(pscm::list(args->a[0], args->a[1], args->a[2], args->a[3])); break;
    default:
    {
      int i;
      SCM l= pscm::nil;
      for (i=args->n; i>=0; i--)
        l= pscm::cons (args->a[i], l);
      return scm->eval(l);
    }
  }
}

#ifndef DEBUG_ON
static tmscm
TeXmacs_lazy_call_scm (arg_list* args) {
  PSCM_THROW_EXCEPTION("not supported now");
  // return scm_internal_lazy_catch (SCM_BOOL_T,
                                  // (scm_t_catch_body) TeXmacs_call, (void*) args,
                                  // (scm_t_catch_handler) TeXmacs_lazy_catcher, (void*) args);
}
#endif

static tmscm
TeXmacs_call_scm (arg_list *args) {
// #ifndef DEBUG_ON
  // return scm_internal_catch (SCM_BOOL_T,
                            //  (scm_t_catch_body) TeXmacs_lazy_call_scm, (void*) args,
                            //  (scm_t_catch_handler) TeXmacs_catcher, (void*) args);
// #else
  return TeXmacs_call(args);
// #endif
}

tmscm
call_scheme (tmscm fun) {
// uncomment block to display scheme call
/*
  SCM ENDLscm= scm_from_locale_string ("\n");
  SCM source=scm_procedure_source(fun);
  scm_call_2(scm_c_eval_string("display*"), source, ENDLscm);
  scm_call_2(scm_c_eval_string("display*"),  scm_procedure_environment(fun), ENDLscm);
  scm_call_2(scm_c_eval_string("display*"),  scm_procedure_properties(fun), ENDLscm);
  //DBGFMT1(debug_tmwidgets, source);
*/
  SCM a[]= { fun }; arg_list args= { 0, a };
  return TeXmacs_call_scm (&args);
}

SCM
call_scheme (SCM fun, SCM a1) {
  SCM a[]= { fun, a1 }; arg_list args= { 1, a };
  return TeXmacs_call_scm (&args);
}

SCM
call_scheme (SCM fun, SCM a1, SCM a2) {
  SCM a[]= { fun, a1, a2 }; arg_list args= { 2, a };
  return TeXmacs_call_scm (&args);
}

SCM
call_scheme (SCM fun, SCM a1, SCM a2, SCM a3) {
  SCM a[]= { fun, a1, a2, a3 }; arg_list args= { 3, a };
  return TeXmacs_call_scm (&args);
}

SCM
call_scheme (SCM fun, SCM a1, SCM a2, SCM a3, SCM a4) {
  SCM a[]= { fun, a1, a2, a3, a4 }; arg_list args= { 4, a };
  return TeXmacs_call_scm (&args);
}

SCM
call_scheme (SCM fun, array<SCM> a) {
  const int n= N(a);
  STACK_NEW_ARRAY(scm, SCM, n+1);
  int i;
  scm[0]= fun;
  for (i=0; i<n; i++) scm[i+1]= a[i];
  arg_list args= { n, scm };
  SCM ret= TeXmacs_call_scm (&args);
  STACK_DELETE_ARRAY(scm);
  return ret;
}


/******************************************************************************
 * Miscellaneous routines for use by glue only
 ******************************************************************************/

string
scheme_dialect () {
  return "pscm";
}

#if (defined(GUILE_C) || defined(GUILE_D))
#define SET_SMOB(smob,data,type)   \
SCM_NEWSMOB (smob, SCM_UNPACK (type), data);
#else
#define SET_SMOB(smob,data,type)   \
SCM_NEWCELL (smob);              \
SCM_SETCAR (smob, (SCM) (type)); \
SCM_SETCDR (smob, (SCM) (data));
#endif


/******************************************************************************
 * Booleans
 ******************************************************************************/


tmscm
bool_to_scm (bool flag) {
  return tmscm (flag);
}

/******************************************************************************
 * Integers
 ******************************************************************************/

tmscm
int_to_scm (int i) {
  return new pscm::Number(i);
}

tmscm
long_to_scm (long l) {
  return new pscm::Number((int64_t)l);
}


/******************************************************************************
 * Floating point numbers
 ******************************************************************************/
#if 0
bool scm_is_double (scm o) {
  return SCM_REALP(o);
}
#endif

SCM
double_to_scm (double i) {
  return new pscm::Number (i);
}

/******************************************************************************
 * Strings
 ******************************************************************************/


tmscm
string_to_tmscm (string s) {
  c_string _s (s);
  auto data = (char*)_s;
  return new pscm::String(std::string(data, N(s)));
}

bool tmscm_to_bool (tmscm obj) {
  PSCM_ASSERT(obj.is_bool());
  return obj.to_bool();
}
int tmscm_to_int (tmscm obj) {
  PSCM_ASSERT(obj.is_num());
  auto num = obj.to_number();
  if (num->is_int()) {
    return num->to_int();
  }
  else {
    int n = num->to_float();
    return n;
  }
}
long tmscm_to_long (tmscm obj) {
  PSCM_ASSERT(obj.is_num());
  auto num = obj.to_number();
  if (num->is_int()) {
    return num->to_int();
  }
  else {
    long n = num->to_float();
    return n;
  }
}
double tmscm_to_double (tmscm obj) {
  PSCM_ASSERT(obj.is_num());
  auto num = obj.to_number();
  if (num->is_int()) {
    return num->to_int();
  }
  else {
    return num->to_float();
  }
}
string
tmscm_to_string (tmscm s) {
  PSCM_ASSERT(s.is_str());
  auto str = s.to_str();
  PSCM_ASSERT(str);
  auto ss = str->str();
  string r (ss.data(), ss.size());
  return r;
}

/******************************************************************************
 * Symbols
 ******************************************************************************/

#if 0
bool tmscm_is_symbol (tmscm s) {
  return SCM_NFALSEP (scm_symbol_p (s));
}
#endif

tmscm
symbol_to_tmscm (string s) {
  c_string _s (s);
  auto data = (char*)_s;
  return new pscm::Symbol(std::string(data, N(s)));
}

string
tmscm_to_symbol (tmscm s) {
  PSCM_ASSERT(s.is_sym());
  auto sym = s.to_symbol();
  PSCM_ASSERT(sym);
  auto name = sym->name();
  return string(name.data(), name.size());
}

/******************************************************************************
 * Blackbox
 ******************************************************************************/

static long blackbox_tag;

#define SCM_BLACKBOXP(t) \
(SCM_NIMP (t) && (((long) SCM_CAR (t)) == blackbox_tag))

bool
tmscm_is_blackbox (tmscm t) {
  return t.is_smob() && t.to_smob()->tag == blackbox_tag;
  // return SCM_BLACKBOXP (t);
}

tmscm
blackbox_to_tmscm (blackbox b) {
  auto smob = new pscm::SmallObject(blackbox_tag, tm_new<blackbox>(b));
  SCM blackbox_smob(smob);
  // SET_SMOB (blackbox_smob, (void*) (tm_new<blackbox> (b)), (SCM) blackbox_tag);
  return blackbox_smob;
}

blackbox
tmscm_to_blackbox (tmscm blackbox_smob) {
  PSCM_ASSERT(blackbox_smob.is_smob());
  auto smob = blackbox_smob.to_smob();
  PSCM_ASSERT(smob->tag == blackbox_tag);
  return *((blackbox*)smob->data);
  // return *((blackbox*) SCM_CDR (blackbox_smob));
}

static SCM
mark_blackbox (SCM blackbox_smob) {
  (void) blackbox_smob;
  return pscm::Cell::bool_false();
}

static std::size_t
free_blackbox (SCM blackbox_smob) {
  PSCM_ASSERT(blackbox_smob.is_smob());
  auto smob = blackbox_smob.to_smob();
  PSCM_ASSERT(smob->tag == blackbox_tag);
  blackbox *ptr = (blackbox*)smob->data;
#ifdef DEBUG_ON
  scm_busy= true;
#endif
  tm_delete (ptr);
#ifdef DEBUG_ON
  scm_busy= false;
#endif
  return 0;
}

int
print_blackbox (SCM blackbox_smob, SCM port) {
  // (void) pstate;
  string s = "<blackbox>";
  int type_ = type_box (tmscm_to_blackbox(blackbox_smob)) ;
  if (type_ == type_helper<tree>::id) {
    tree t= tmscm_to_tree (blackbox_smob);
    s= "<tree " * tree_to_texmacs (t) * ">";
  }
  else if (type_ == type_helper<observer>::id) {
    s= "<observer>";
  }
  else if (type_ == type_helper<widget>::id) {
    s= "<widget>";
  }
  else if (type_ == type_helper<promise<widget> >::id) {
    s= "<promise-widget>";
  }
  else if (type_ == type_helper<command>::id) {
    s= "<command>";
  }
  else if (type_ == type_helper<url>::id) {
    url u= tmscm_to_url (blackbox_smob);
    s= "<url " * as_string (u) * ">";
  }
  else if (type_ == type_helper<modification>::id) {
    s= "<modification>";
  }
  else if (type_ == type_helper<patch>::id) {
    s= "<patch>";
  }
  
  // scm_display (string_to_tmscm (s), port);

  return 1;
}

static SCM
cmp_blackbox (SCM t1, SCM t2) {
  return pscm::Cell (tmscm_to_blackbox (t1) == tmscm_to_blackbox (t2));
}



/******************************************************************************
 * Initialization
 ******************************************************************************/


#ifdef SCM_NEWSMOB
void
initialize_smobs () {
  blackbox_tag= scm_make_smob_type (const_cast<char*> ("blackbox"), 0);
  scm_set_smob_mark (blackbox_tag, mark_blackbox);
  scm_set_smob_free (blackbox_tag, free_blackbox);
  scm_set_smob_print (blackbox_tag, print_blackbox);
  scm_set_smob_equalp (blackbox_tag, cmp_blackbox);
}

#else

// scm_smobfuns blackbox_smob_funcs = {
//   mark_blackbox, free_blackbox, print_blackbox, cmp_blackbox
// };


void
initialize_smobs () {
  // blackbox_tag= scm_newsmob (&blackbox_smob_funcs);
  blackbox_tag = 101;
}

#endif

tmscm object_stack;

void
initialize_scheme () {
  spdlog::set_level(spdlog::level::err);
  const char* init_prg =
  ";(read-set! keywords 'prefix)\n"
  ";(read-enable 'positions)\n"
  ";(debug-enable 'debug)\n"
#ifdef DEBUG_ON
  "(debug-enable 'backtrace)\n"
#endif
  "\n"
  "(define (display-to-string obj)\n"
  "  (call-with-output-string\n"
  "    (lambda (port) (display obj port))))\n"
  "(define (object->string obj)\n"
  "  (call-with-output-string\n"
  "    (lambda (port) (write obj port))))\n"
  "\n"
  "(define (texmacs-version) \"" TEXMACS_VERSION "\")\n"
  "(define object-stack '(()))";
  
  scm = new pscm::Scheme();
  scm->eval_all(init_prg);
  // scm_c_eval_string (init_prg);
  // initialize_smobs ();
  initialize_glue ();
  object_stack= scm->eval("object-stack");
  
    // uncomment to have a guile repl available at startup	
    //	gh_repl(guile_argc, guile_argv);
    //scm_shell (guile_argc, guile_argv);
  
  
}
