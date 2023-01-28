
/******************************************************************************
* MODULE     : vau_editor.hpp
* DESCRIPTION: Vau editor. Typeset a given buffer.
* COPYRIGHT  : (C) 2023  Joris van der Hoeven and Massimiliano Gubinelli
*******************************************************************************
* This software falls under the GNU general public license version 3 or later.
* It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
* in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
******************************************************************************/

#include <climits>
#include "vau_editor.hpp"

#include "blackbox.hpp"
#include "convert.hpp"
#include "file.hpp"
#include "analyze.hpp"
#include "tm_timer.hpp"
#include "Bridge/impl_typesetter.hpp"
#include "new_style.hpp"
#include "iterator.hpp"
#include "merge_sort.hpp"
#ifdef EXPERIMENTAL
#include "../../Style/Environment/std_environment.hpp"
#endif // EXPERIMENTAL

#include "printer.hpp"
#include "new_data.hpp"
#include "new_document.hpp"

#include "vau_buffer.hpp"

//box empty_box (path ip, int x1=0, int y1=0, int x2=0, int y2=0);
bool enable_fastenv= false;


/******************************************************************************
* editor
******************************************************************************/

editor::editor (vau_buffer buf) : rep(tm_new<editor_rep> (buf)) {}

/******************************************************************************
* Contructors, destructors and notification of modifications
******************************************************************************/

editor_rep::editor_rep (vau_buffer buf2):
  buf (buf2),
  drd (buf->buf->title, std_drd), et (the_et), rp (buf2->rp),
  the_style (TUPLE),
  cur (hashmap<string,tree> (UNINIT)),
  stydef (UNINIT), pre (UNINIT), init (UNINIT), fin (UNINIT), grefs (UNINIT),
  env (drd, buf->buf->master,
       buf->data->ref, (buf->prj==NULL? grefs: buf->prj->data->ref),
       buf->data->aux, (buf->prj==NULL? buf->data->aux: buf->prj->data->aux),
       buf->data->att, (buf->prj==NULL? buf->data->att: buf->prj->data->att)),
  ttt (new_typesetter (env, subtree (et, rp), reverse (rp))) {
    init_update ();
}

editor_rep::~editor_rep () { delete_typesetter (ttt); }

typesetter editor_rep::get_typesetter () { return ttt; }
tree editor_rep::get_style () { return the_style; }
void editor_rep::set_style (tree t) { the_style= copy (t); }
hashmap<string,tree> editor_rep::get_init () { return init; }
hashmap<string,tree> editor_rep::get_fin () { return fin; }
hashmap<string,tree> editor_rep::get_ref () { return buf->data->ref; }
hashmap<string,tree> editor_rep::get_aux () { return buf->data->aux; }
hashmap<string,tree> editor_rep::get_att () { return buf->data->att; }
void editor_rep::set_fin (hashmap<string,tree> H) { fin= H; }
void editor_rep::set_ref (hashmap<string,tree> H) { buf->data->ref= H; }
void editor_rep::set_aux (hashmap<string,tree> H) { buf->data->aux= H; }
void editor_rep::set_att (hashmap<string,tree> H) { buf->data->att= H; }

void
editor_rep::get_data (new_data& data) {
  data->style= get_style ();
  data->init = get_init ();
  data->fin  = get_fin ();
  data->ref  = get_ref ();
  data->aux  = get_aux ();
  data->att  = get_att ();
}

void
editor_rep::set_data (new_data data) {
  set_style (data->style);
  set_init  (data->init);
  set_fin   (data->fin);
  set_ref   (data->ref);
  set_aux   (data->aux);
  set_att   (data->att);
  //FIXME: notify_page_change ();
  add_init (data->init);
  notify_change (THE_DECORATIONS);
  typeset_invalidate_env ();
  iterator<string> it = iterate (data->att);
  while (it->busy()) {
    string key= it->next ();
    (void) call (string ("notify-set-attachment"),
                 buf->buf->name, key, data->att [key]);
  }
}


tree
editor_rep::get_ref (string key) {
  return buf->data->ref[key];
}

void
editor_rep::set_ref (string key, tree im) {
  buf->data->ref (key)= im;
}

void
editor_rep::reset_ref (string key) {
  buf->data->ref->reset (key);
}

static string
concat_as_string (tree t) {
  if (is_atomic (t)) return t->label;
  else if (is_func (t, CONCAT)) {
    string r;
    for (int i=0; i<N(t); i++)
      r << concat_as_string (t[i]);
    return r;
  }
  else return "?";
}

array<string>
editor_rep::find_refs (string val, bool global) {
  tree a= (tree) buf->data->ref;
  if (global && buf->prj != NULL) a= buf->prj->data->ref;
  array<string> v;
  int i, n= N(a);
  for (i=0; i<n; i++)
    if (N(a[i]) >= 2 && N(a[i][1]) >= 1 &&
        concat_as_string (a[i][1][0]) == val)
      v << a[i][0]->label;
  return v;
}

array<string>
editor_rep::list_refs (bool global) {
  tree a= (tree) buf->data->ref;
  if (global && buf->prj != NULL) a= buf->prj->data->ref;
  array<string> v;
  int i, n= N(a);
  for (i=0; i<n; i++)
    v << a[i][0]->label;
  merge_sort (v);
  return v;
}

tree
editor_rep::get_aux (string key) {
  return buf->data->aux[key];
}

void
editor_rep::set_aux (string key, tree im) {
  buf->data->aux (key)= im;
}

void
editor_rep::reset_aux (string key) {
  buf->data->aux->reset (key);
}

array<string>
editor_rep::list_auxs (bool global) {
  tree a= (tree) buf->data->aux;
  if (global && buf->prj != NULL) a= buf->prj->data->aux;
  array<string> v;
  int i, n= N(a);
  for (i=0; i<n; i++)
    v << a[i][0]->label;
  merge_sort (v);
  return v;
}

tree
editor_rep::get_att (string key) {
  return buf->data->att[key];
}

void
editor_rep::set_att (string key, tree im) {
  buf->data->att (key)= im;
}

void
editor_rep::reset_att (string key) {
  buf->data->att->reset (key);
}

array<string>
editor_rep::list_atts (bool global) {
  tree a= (tree) buf->data->att;
  if (global && buf->prj != NULL) a= buf->prj->data->att;
  array<string> v;
  int i, n= N(a);
  for (i=0; i<n; i++)
    v << a[i][0]->label;
  merge_sort (v);
  return v;
}

void
editor_rep::set_init (hashmap<string,tree> H) {
  init= hashmap<string,tree> (UNINIT);
  add_init (H);
}

void
editor_rep::add_init (hashmap<string,tree> H) {
  init->join (H);
  ::notify_assign (ttt, path(), subtree (et, rp));
  notify_change (THE_ENVIRONMENT);
}

void
editor_rep::clear_local_info () {
  buf->data->ref= hashmap<string,tree> ();
  buf->data->aux= hashmap<string,tree> ();
}

void
editor_rep::init_style (string name) {
  if ((name == "none") || (name == "") || (name == "style")) the_style= TUPLE;
  else if (arity (the_style) == 0) the_style= tree (TUPLE, name);
  else the_style= tree (TUPLE, name) * the_style (1, N(the_style));
  //FIXME: require_save ();
  notify_change (THE_ENVIRONMENT);
}

/******************************************************************************
* Processing preamble
******************************************************************************/

void
use_modules (tree t) {
  if (is_tuple (t))
    for (int i=0; i<N(t); i++) {
      string s= as_string (t[i]);
      if (starts (s, "(")) eval ("(use-modules " * s * ")");
      else if (s != "") eval ("(plugin-initialize '" * s * ")");
    }
}

void
editor_rep::typeset_style_use_cache (tree style) {
  style= preprocess_style (style, buf->buf->master);
  //cout << "Typesetting style using cache " << style << LF;
  bool ok;
  hashmap<string,tree> H;
  tree t;
  style_get_cache (style, H, t, ok);
  if (ok) {
    env->patch_env (H);
    ok= drd->set_locals (t);
    drd->set_environment (H);
  }
  if (!ok) {
    //cout << "Typeset without cache " << style << LF;
    if (!is_tuple (style)) FAILED ("tuple expected as style");
    H= get_style_env (style);
    drd= get_style_drd (style);
    style_set_cache (style, H, drd->get_locals ());
    env->patch_env (H);
    drd->set_environment (H);
  }
  use_modules (env->read (THE_MODULES));
}

void
editor_rep::typeset_preamble () {
  env->write_default_env ();
  typeset_style_use_cache (the_style);
  env->update ();
  env->read_env (stydef);
  env->patch_env (init);
  env->update ();
  env->read_env (pre);
  drd->heuristic_init (pre);
}

void
editor_rep::typeset_prepare () {
  env->base_file_name= buf->buf->master;
  env->read_only= buf->buf->read_only;
  env->write_default_env ();
  env->patch_env (pre);
  env->style_init_env ();
  env->update ();
}

void
editor_rep::init_update () {
  if (buf->prj != NULL) {
    string id= as_string (delta (buf->prj->buf->name, buf->buf->name));
    string lab= "part:" * id;
    hashmap<string,tree> aux= env->global_aux;
    hashmap<string,tree> ref= env->global_ref;
    if (aux->contains ("parts")) {
      tree parts= aux ["parts"];
      if (is_func (parts, DOCUMENT))
        for (int i=0; i<N(parts); i++)
          if (is_tuple (parts[i]) && N(parts[i]) >= 1)
            if (parts[i][0] == id)
              for (int j=1; j+1 < N(parts[i]); j+=2)
                if (is_atomic (parts[i][j])) {
                  buf->data->init (parts[i][j]->label)= copy (parts[i][j+1]);
                  init (parts[i][j]->label)= copy (parts[i][j+1]);
                }
    }
    if (ref->contains (lab)) {
      tree val= ref [lab];
      if (is_tuple (val) && N(val) >= 2 && val[1] != tree (UNINIT)) {
        buf->data->init (PAGE_FIRST)= copy (val[1]);
        init (PAGE_FIRST)= copy (val[1]);
      }
    }
  }
  else if (buf->data->init ["part-flag"] == "true")
    grefs= copy (buf->data->ref);
}

void
editor_rep::drd_update () {
  typeset_exec_until (tp);
  drd->heuristic_init (cur[tp]);
}

#ifdef EXPERIMENTAL
void
editor_rep::environment_update () {
  hashmap<string,tree> h;
  typeset_prepare ();
  env->assign ("base-file-name", as_string (env->base_file_name));
  env->assign ("cur-file-name", as_string (env->cur_file_name));
  env->assign ("secure", bool_as_tree (env->secure));
  env->read_env (h);
  ::primitive (ste, h);
}
#endif

/******************************************************************************
* Routines for getting information
******************************************************************************/

void
editor_rep::typeset_invalidate_env () {
  cur= hashmap<path,hashmap<string,tree> > (hashmap<string,tree> (UNINIT));
}

static void
restricted_exec (edit_env env, tree t, int end) {
  if (is_func (t, ASSIGN, 2) && end == 2)
    env->exec (t);
  else if (is_document (t) || is_concat (t))
    for (int i=0; i < min (end, 10); i++)
      restricted_exec (env, t[i], arity (t[i]));
  else if (is_compound (t, "hide-preamble", 1) ||
           is_compound (t, "show-preamble", 1))
    env->exec (t[0]);
  else if (is_compound (t, "script-input", 4) && end == 2) {
    if (env->read (MODE) == "text") {
      env->write (MODE, "prog");
      env->write (PROG_LANGUAGE, t[0]);
    }
  }
}

static tree
filter_format (tree fm, int i, int n) {
  array<tree> r;
  for (int k=0; k<N(fm); k++)
    if (is_func (fm[k], CWITH) && N(fm[k]) >= 4 &&
        is_int (fm[k][0]) && is_int (fm[k][1])) {
      int j1= as_int (fm[k][0]->label);
      int j2= as_int (fm[k][1]->label);
      if (j1 > 0) j1--; else j1 += n;
      if (j2 > 0) j2--; else j2 += n;
      if (i >= j1 && i <= j2) r << fm[k] (2, N(fm[k]));
    }
  return tree (TFORMAT, r);
}

static void
table_descend (tree& t, path& p, tree& fm) {
  while (!is_nil (p)) {
    if (L(t) == TFORMAT && p->item == N(t) - 1) {
      array<tree> r;
      for (int k=0; k<N(t)-1; k++)
        if (is_func (t[k], CWITH, 6) &&
            is_atomic (t[k][4]) &&
            !starts (t[k][4]->label, "cell-"))
          r << t[k];
      fm= tree (TFORMAT, r);
      t= t[N(t)-1];
      p= p->next;
    }
    else if ((L(t) == TABLE || L(t) == ROW) &&
             p->item >= 0 && p->item < N(t)) {
      fm= filter_format (fm, p->item, N(t));
      t= t[p->item];
      p= p->next;
    }
    else break;
  }
}

static void
define_style_macros (edit_env& env, tree t) {
  if (is_document (t) || is_concat (t)) {
    int i, n=N(t);
    for (i=0; i<n; i++)
      define_style_macros (env, t[i]);
  }
  else if (is_func (t, ASSIGN, 2) && is_atomic (t[0]))
    env->write (t[0]->label, t[1]);
}

void
editor_rep::typeset_exec_until (path p) {
  // FIXME: we should ensure that p is inside the document
  // if (!(rp <= p)) p= correct_cursor (et, rp * 0);

  //time_t t1= texmacs_time ();
  if (has_changed (THE_TREE + THE_ENVIRONMENT))
    if (p != correct_cursor (et, rp * 0)) {
      if (DEBUG_STD) std_warning << "resynchronizing for path " << p << "\n";
      // apply_changes ();
    }
  if (p == tp && inside_graphics (true) && p != closest_inside (et, p)) {
    //cout << "TeXmacs] Warning: corrected cursor\n";
    tp= closest_inside (et, tp);
    p = tp;
  }

  //cout << "Exec until " << p << LF;
  if (N(cur[p])!=0) return;
  if (N(cur)>=25) // avoids out of memory in weird cases
    typeset_invalidate_env ();
  typeset_prepare ();
  if (enable_fastenv) {
    if (!(rp < p)) {
      failed_error << "Erroneous path " << p << "\n";
      FAILED ("invalid typesetting path");
    }
    tree t= subtree (et, rp);
    path q= path_up (p / rp);
    while (!is_nil (q)) {
      int i= q->item;
      restricted_exec (env, t, i);
      if (L(t) == TFORMAT && i == N(t) - 1) {
        tree fm= tree (TFORMAT);
        table_descend (t, q, fm);
        if (!is_nil (q))
          for (int k=0; k<N(fm); k++)
            if (is_func (fm[k], CWITH, 2))
              env->write (fm[k][0]->label, fm[k][1]);
      }
      else {
        tree w= drd->get_env_child (t, i, tree (ATTR));
        if (w == "") break;
        //cout << "t= " << t << "\n";
        //cout << "i= " << i << "\n";
        //cout << "w= " << w << "\n";
        tree ww (w, N(w));
        for (int j=0; j<N(w); j+=2) {
          //cout << w[j] << " := " << env->exec (w[j+1]) << "\n";
          ww[j+1]= env->exec (w[j+1]);
        }
        for (int j=0; j<N(w); j+=2)
          env->write (w[j]->label, ww[j+1]);
        t= t[i];
        q= q->next;
      }
    }
    if (env->read (PREAMBLE) == "true")
      env->write (MODE, "src");
  }
  else exec_until (ttt, p / rp);
  if (env->read (MODE) == "src" && env->read (PREAMBLE) != "true")
    define_style_macros (env, subtree (et, rp));
  env->read_env (cur (p));
  //time_t t2= texmacs_time ();
  //if (t2 - t1 >= 10) cout << "typeset_exec_until took " << t2-t1 << "ms\n";
}

tree
editor_rep::get_full_env () {
  typeset_exec_until (tp);
  return (tree) cur[tp];
}

bool
editor_rep::defined_at_cursor (string var) {
  typeset_exec_until (tp);
  return cur[tp]->contains (var);
}

tree
editor_rep::get_env_value (string var, path p) {
  typeset_exec_until (p);
  tree t= cur[p][var];
  return is_func (t, BACKUP, 2)? t[0]: t;
}

tree
editor_rep::get_env_value (string var) {
 /* FIXME: tp is wrong (and consequently, crashes TeXmacs)
  *   when we call this routine from inside the code which
  *   is triggered by a button, for example.
  *
  * Test: fire TeXmacs, then open a new Graphics, then click
  *   on the icon for going in spline mode. Then it crashes,
  *   because we call (get-env-tree) from inside the Scheme.
  *   If we call (get-env-tree-at ... (cDr (cursor-path))),
  *   then it works.
  */
  return get_env_value (var, tp);
}

bool
editor_rep::defined_at_init (string var) {
  if (init->contains (var)) return true;
  if (N(pre)==0) typeset_preamble ();
  return pre->contains (var);
}

bool
editor_rep::defined_in_init (string var) {
  return init->contains (var);
}

tree
editor_rep::get_init_value (string var) {
  if (init->contains (var)) {
    tree t= init [var];
    if (var == BG_COLOR && is_func (t, PATTERN)) t= env->exec (t);
    return is_func (t, BACKUP, 2)? t[0]: t;
  }
  if (N(pre)==0) typeset_preamble ();
  tree t= pre [var];
  if (var == BG_COLOR && is_func (t, PATTERN)) t= env->exec (t);
  return is_func (t, BACKUP, 2)? t[0]: t;
}

string
editor_rep::get_env_string (string var) {
  return as_string (get_env_value (var));
}

string
editor_rep::get_init_string (string var) {
  return as_string (get_init_value (var));
}

int
editor_rep::get_env_int (string var) {
  return as_int (get_env_value (var));
}

int
editor_rep::get_init_int (string var) {
  return as_int (get_init_value (var));
}

double
editor_rep::get_env_double (string var) {
  return as_double (get_env_value (var));
}

double
editor_rep::get_init_double (string var) {
  return as_double (get_init_value (var));
}

color
editor_rep::get_env_color (string var) {
  return named_color (as_string (get_env_value (var)));
}

color
editor_rep::get_init_color (string var) {
  return named_color (as_string (get_init_value (var)));
}

language
editor_rep::get_env_language () {
  string mode= get_env_string (MODE);
  if (mode == "text")
    return text_language (get_env_string (LANGUAGE));
  else if (mode == "math")
    return math_language (get_env_string (MATH_LANGUAGE));
  else return prog_language (get_env_string (PROG_LANGUAGE));
}

int
editor_rep::get_page_count () {
  return N (eb[0]);
}

SI
editor_rep::get_page_width (bool deco) {
  (void) get_env_string (PAGE_WIDTH);
  return (env->get_page_width (deco) + std_shrinkf - 1) / std_shrinkf;
}

SI
editor_rep::get_pages_width (bool deco) {
  (void) get_env_string (PAGE_WIDTH);
  return (env->get_pages_width (deco) + std_shrinkf - 1) / std_shrinkf;
}

SI
editor_rep::get_page_height (bool deco) {
  (void) get_env_string (PAGE_HEIGHT);
  return (env->get_page_height (deco) + std_shrinkf - 1) / std_shrinkf;
}

SI
editor_rep::get_total_width (bool deco) {
  SI w= eb->w();
  if (!deco) {
    SI w1= env->get_pages_width (false);
    SI w2= env->get_pages_width (true);
    w -= (w2 - w1);
  }
  return (w + std_shrinkf - 1) / std_shrinkf;
}

SI
editor_rep::get_total_height (bool deco) {
  SI h= eb->h();
  if (!deco) {
    SI h1= env->get_page_height (false);
    SI h2= env->get_page_height (true);
    int nr= get_page_count ();
    int nx= env->page_packet;
    int ny= ((nr + env->page_offset) + nx - 1) / nx;
    h -= ny * (h2 - h1);
  }
  return (h + std_shrinkf - 1) / std_shrinkf;
}

bool
editor_rep::get_save_aux () {
  return as_bool (get_init_string (SAVE_AUX));
}

/******************************************************************************
* Execution without typesetting
******************************************************************************/

static tree
simplify_execed (tree t) {
  if (is_atomic (t)) return t;
  int i, n= N(t);
  tree r (t, n);
  for (i=0; i<n; i++)
    r[i]= simplify_execed (t[i]);
  if (is_func (r, QUOTE, 1) && is_atomic (r[0]))
    return r[0];
  else return r;
}

static tree
expand_references (tree t, hashmap<string,tree> h) {
  if (is_atomic (t)) return t;
  if (is_func (t, REFERENCE, 1) || is_func (t, PAGEREF)) {
    string ref= as_string (simplify_execed (t[0]));
    if (h->contains (ref)) {
      int which= is_func (t, REFERENCE, 1)? 0: 1;
      return tree (HLINK, copy (h[ref][which]), "#" * ref);
    }
    return tree (HLINK, "?", "#" * ref);
  }
  int i, n= N(t);
  tree r (t, n);
  for (i=0; i<n; i++)
    r[i]= expand_references (t[i], h);
  return r;
}

static void
prefix_specific (hashmap<string,tree>& H, string prefix) {
  hashmap<string,tree> R;
  iterator<string> it = iterate (H);
  while (it->busy()) {
    string key= it->next ();
    string var= prefix * key;
    if (H->contains (var)) R(key)= H[var];
    else R(key)= H[key];
  }
  H= R;
}

tree
editor_rep::exec (tree t, hashmap<string,tree> H, bool expand_refs) {
  hashmap<string,tree> H2;
  env->read_env (H2);
  env->write_env (H);
  t= env->exec (t);
  if (expand_refs)
    t= expand_references (t, buf->data->ref);
  t= simplify_execed (t);
  t= simplify_correct (t);
  env->write_env (H2);
  return t;
}

tree
editor_rep::exec_texmacs (tree t, path p) {
  typeset_exec_until (p);
  return exec (t, cur[p]);
}

tree
editor_rep::exec_texmacs (tree t) {
  return exec_texmacs (t, rp * 0);
}

tree
editor_rep::exec_verbatim (tree t, path p) {
  t= convert_OTS1_symbols_to_universal_encoding (t);
  typeset_exec_until (p);
  hashmap<string,tree> H= copy (cur[p]);
  H ("TeXmacs")= tree (MACRO, "TeXmacs");
  H ("LaTeX")= tree (MACRO, "LaTeX");
  H ("TeX")= tree (MACRO, "TeX");
  return exec (t, H);
}

tree
editor_rep::exec_verbatim (tree t) {
  return exec_verbatim (t, rp * 0);
}

static tree
search_doc_title (tree t) {
  if (is_atomic (t)) return "";
  else if (is_compound (t, "doc-title", 1)) return t[0];
  else {
    for (int i=0; i<N(t); i++) {
      tree r= search_doc_title (t[i]);
      if (r != "") return r;
    }
    return "";
  }
}

tree
editor_rep::exec_html (tree t, path p) {
  t= convert_OTS1_symbols_to_universal_encoding (t);
  if (p == (rp * 0)) typeset_preamble ();
  typeset_exec_until (p);
  hashmap<string,tree> H= copy (cur[p]);
  tree patch= as_tree (eval ("(stree->tree (tmhtml-env-patch))"));
  hashmap<string,tree> P (UNINIT, patch);
  H->join (P);
  prefix_specific (H, "tmhtml-");
  tree w (WITH);
  tree doc_title= search_doc_title (t);
  if (doc_title != "")
    w << string ("html-doc-title") << doc_title;
  if (H->contains ("html-title"))
    w << string ("html-title") << H["html-title"];
  if (H->contains ("html-css"))
    w << string ("html-css") << H["html-css"];
  if (H->contains ("html-head-javascript"))
    w << string ("html-head-javascript") << H["html-head-javascript"];
  if (H->contains ("html-head-javascript-src"))
    w << string ("html-head-javascript-src") << H["html-head-javascript-src"];
  if (H->contains ("html-head-favicon"))
    w << string ("html-head-favicon") << H["html-head-favicon"];
  if (H->contains ("html-extra-css"))
    w << string ("html-extra-css") << H["html-extra-css"];
  if (H->contains ("html-extra-javascript-src"))
    w << string ("html-extra-javascript-src")
      << H["html-extra-javascript-src"];
  if (H->contains ("html-extra-javascript"))
    w << string ("html-extra-javascript") << H["html-extra-javascript"];
  if (H->contains ("html-site-version"))
    w << string ("html-site-version") << H["html-site-version"];
  if (N(w) == 0) return exec (t, H);
  else {
    w << t;
    return exec (w, H);
  }
  //tree r= exec (t, H);
  //cout << "In: " << t << "\n";
  //cout << "Out: " << r << "\n";
  //return r;
}

tree
editor_rep::exec_html (tree t) {
  return exec_html (t, rp * 0);
}

static tree
value_to_compound (tree t, hashmap<string,tree> h) {
  if (is_atomic (t)) return t;
  else if (is_func (t, VALUE, 1) &&
       is_atomic (t[0]) &&
       h->contains (t[0]->label))
    return compound (t[0]->label);
  else {
    int i, n= N(t);
    tree r (t, n);
    for (i=0; i<n; i++)
      r[i]= value_to_compound (t[i], h);
    return r;
  }
}

tree
editor_rep::exec_latex (tree t, path p) {
  t= convert_OTS1_symbols_to_universal_encoding (t);
  bool expand_unknown_macros= "on" == as_string (
      call ("get-preference", "texmacs->latex:expand-macros"));
  bool expand_user_macro= "on" == as_string (
      call ("get-preference", "texmacs->latex:expand-user-macros"));
  if (!expand_unknown_macros && !expand_user_macro)
    return t;
  if (p == (rp * 0)) typeset_preamble ();
  typeset_exec_until (p);
  hashmap<string,tree> H= copy (cur[p]);
  object l= null_object ();
  iterator<string> it= iterate (H);
  while (it->busy ()) l= cons (object (it->next ()), l);
  tree patch= as_tree (call ("stree->tree", call ("tmtex-env-patch", t, l)));
  hashmap<string,tree> P (UNINIT, patch);
  H->join (P);
  prefix_specific (H, "tmlatex-");

  if (!expand_user_macro &&
      is_document (t) && is_compound (t[0], "hide-preamble")) {
    tree r= copy (t);
    r[0]= "";
    r= exec (value_to_compound (r, P), H, false);
    r[0]= exec (t[0], H, false);
    return r;
  }
  else {
    tree r= exec (value_to_compound (t, P), H, false);
    return r;
  }
}

tree
editor_rep::exec_latex (tree t) {
  return exec_latex (t, rp * 0);
}

tree
editor_rep::texmacs_exec (tree t) {
  return ::texmacs_exec (env, t);
}

tree
editor_rep::var_texmacs_exec (tree t) {
  typeset_exec_until (tp);
  env->write_env (cur[tp]);
  env->update_frame ();
  return texmacs_exec (t);
}


/******************************************************************************
* Actual typesetting
******************************************************************************/

void
editor_rep::typeset_sub (SI& x1, SI& y1, SI& x2, SI& y2) {
  //time_t t1= texmacs_time ();
  typeset_prepare ();
  eb= empty_box (reverse (rp));
  // saves memory, also necessary for change_log update
  bench_start ("typeset");
#ifdef USE_EXCEPTIONS
  try {
#endif
    eb= ::typeset (ttt, x1, y1, x2, y2);
#ifdef USE_EXCEPTIONS
  }
  catch (string msg) {
    the_exception= msg;
    std_error << "Typesetting failure, resetting to empty document\n";
    assign (rp, tree (DOCUMENT, ""));
    ::notify_assign (ttt, path(), subtree (et, rp));
    eb= ::typeset (ttt, x1, y1, x2, y2);
  }
  handle_exceptions ();
#endif
  bench_end ("typeset");
  //time_t t2= texmacs_time ();
  //if (t2 - t1 >= 10) cout << "typeset took " << t2-t1 << "ms\n";
  picture_cache_clean ();
}

static void
report_missing (hashmap<string,tree> missing) {
  array<string> a;
  for (iterator<string> it= iterate (missing); it->busy(); a << it->next ()) {}
  merge_sort (a);
  for (int i=0; i<N(a); i++)
    if (!starts (a[i], "bib-"))
      typeset_warning << "Undefined reference " << a[i] << LF;
}

static void
report_redefined (array<tree> redefined) {
  for (int i=0; i<N(redefined); i++) {
    tree t= redefined[i];
    if (t[1]->label == "")
      typeset_warning << "Redefined " << t[0]->label << LF;
    else
      typeset_warning << "Redefined " << t[0]->label
                      << " as " << t[1]->label << LF;
  }
}

static void
clean_unused (hashmap<string,tree>& refs, hashmap<string,bool> used) {
  array<string> a;
  for (iterator<string> it= iterate (refs); it->busy(); ) {
    string key= it->next ();
    if (!used->contains (key)) a << key;
  }
  for (int i=0; i<N(a); i++)
    refs->reset (a[i]);
}

void
editor_rep::typeset (SI& x1, SI& y1, SI& x2, SI& y2) {
  int missing_nr= INT_MAX;
  int redefined_nr= INT_MAX;
  x1= MAX_SI; y1= MAX_SI; x2= MIN_SI; y2= MIN_SI;
  while (true) {
    SI sx1, sy1, sx2, sy2;
    typeset_sub (sx1, sy1, sx2, sy2);
    x1= min (x1, sx1); y1= min (y1, sy1);
    x2= max (x2, sx2); y2= max (y2, sy2);
    if (!env->complete) break;
    env->complete= false;
    clean_unused (env->local_ref, env->touched);
    if (N(env->missing) == 0 && N(env->redefined) == 0) break;
    if ((N(env->missing) == missing_nr && N(env->redefined) == redefined_nr) ||
        (N(env->missing) > missing_nr || N(env->redefined) > redefined_nr)) {
      report_missing (env->missing);
      report_redefined (env->redefined);
      break;
    }
    missing_nr= N(env->missing);
    redefined_nr= N(env->redefined);
    ::notify_assign (ttt, path(), ttt->br->st);
  }
}

void
editor_rep::typeset_forced () {
  //cout << "Typeset forced\n";
  SI x1, y1, x2, y2;
  typeset (x1, y1, x2, y2);
}

void
editor_rep::typeset_invalidate (path p) {
  if (rp <= p) {
    //cout << "Invalidate " << p << "\n";
    notify_change (THE_TREE);
    ::notify_assign (ttt, p / rp, subtree (et, p));
  }
}

void
editor_rep::typeset_invalidate_all () {
  //cout << "Invalidate all\n";
  notify_change (THE_ENVIRONMENT);
  typeset_preamble ();
  ::notify_assign (ttt, path(), subtree (et, rp));
}

void
editor_rep::typeset_invalidate_players (path p, bool reattach) {
  if (rp <= p) {
    tree t= subtree (et, p);
    blackbox bb;
    bool ok= t->obs->get_contents (ADDENDUM_PLAYER, bb);
    if (ok) {
      if (reattach) tree_addendum_delete (t, ADDENDUM_PLAYER);
      typeset_invalidate (p);
    }
    if (is_compound (t)) {
      int i, n= N(t);
      for (i=0; i<n; i++)
        typeset_invalidate_players (p * i, reattach);
    }
  }
}

/******************************************************************************
* Print document
******************************************************************************/

string printing_dpi ("600");
string printing_on ("a4");


void
editor_rep::print_doc (url name, bool conform, int first, int last) {

  url  orig= resolve (name, "");

  string medium = env->get_string (PAGE_MEDIUM);
  if (conform && (medium != "paper")) conform= false;
    // FIXME: better command for conform printing

  typeset_preamble ();
    // FIXME: when printing several files via aux buffers,
    // it seems that the style can be corrupted.  Why?
  
  // Set environment variables for printing

  typeset_prepare ();
  env->write (DPI, printing_dpi);
  env->write (PAGE_SHOW_HF, "true");
  env->write (PAGE_SCREEN_MARGIN, "false");
  env->write (PAGE_BORDER, "none");
  if (is_func (env->read (BG_COLOR), PATTERN))
    env->write (BG_COLOR, env->exec (env->read (BG_COLOR)));
  if (!conform) {
    env->write (PAGE_MEDIUM, "paper");
    env->write (PAGE_PRINTED, "true");
  }

  // Typeset pages for printing

  box the_box= typeset_as_document (env, subtree (et, rp), reverse (rp));

  // Determine parameters for printer

  string page_type = env->page_real_type;
  double w         = env->page_real_width;
  double h         = env->page_real_height;
  double cm        = env->as_real_length (string ("1cm"));
  bool   landsc    = env->page_landscape;
  int    dpi       = as_int (printing_dpi);
  int    start     = max (0, first-1);
  int    end       = min (N(the_box[0]), last);
  int    pages     = end-start;
  if (conform) {
    page_type= "user";
    SI bw= the_box[0][0]->w();
    SI bh= the_box[0][0]->h();
    string bws= as_string (bw) * "tmpt";
    string bhs= as_string (bh) * "tmpt";
    w= env->as_length (bws);
    h= env->as_length (bhs);
  }
  
  // Print pages
  renderer ren= printer (name, dpi, pages, page_type, landsc, w/cm, h/cm);
  
  if (ren->is_started ()) {
    int i;
    ren->set_metadata ("title", get_metadata ("title"));
    ren->set_metadata ("author", get_metadata ("author"));
    ren->set_metadata ("subject", get_metadata ("subject"));
    for (i=start; i<end; i++) {
      tree bg= env->read (BG_COLOR);
      ren->set_background (bg);
      if (bg != "white" && bg != "#ffffff")
        ren->clear_pattern (0, (SI) -h, (SI) w, 0);

      rectangles rs;
      the_box[0]->sx(i)= 0;
      the_box[0]->sy(i)= 0;
      the_box[0][i]->redraw (ren, path (0), rs);
      if (i<end-1) ren->next_page ();
    }
  }
  tm_delete (ren);
}

void
editor_rep::print_to_file (url name, string first, string last) {
  print_doc (name, false, as_int (first), as_int (last));
  //FIXME: set_message ("Done printing", "print to file");
}

string
editor_rep::get_metadata (string kind) {
  string var= "global-" * kind;
  string val= get_init_string (var);
  if (val != "") return val;
  val= search_metadata (subtree (et, rp), kind);
  if (val != "") return val;
  if (kind == "title") return as_string (tail (get_name ()));
#ifndef OS_MINGW
  if (kind == "author" &&
      !is_none (resolve_in_path ("finger")) &&
      !is_none (resolve_in_path ("sed"))) {
    string val= var_eval_system ("finger `whoami` | sed -e '/Name/!d' -e 's/.*Name: //'");
    if (N(val) > 1) return utf8_to_cork (val);
  }
#endif
  return "";
}

url
editor_rep::get_name () {
  return buf->buf->name;
}

tree
editor_rep::the_subtree (path p) {
  return subtree (et, p);
}

//FIXME: stubs
void editor_rep::notify_change (int changed) {}
bool editor_rep::has_changed (int question) { return false; }
bool editor_rep::inside_graphics (bool b) { return false; }



void
editor_rep::typeset_document (string image_dpi) {
  bool conform= true;

  typeset_preamble ();
  
  // Set environment variables for printing

  typeset_prepare ();
  env->write (DPI, image_dpi);
  env->write (PAGE_SHOW_HF, "true");
  env->write (PAGE_SCREEN_MARGIN, "false");
  env->write (PAGE_BORDER, "none");
  if (is_func (env->read (BG_COLOR), PATTERN))
    env->write (BG_COLOR, env->exec (env->read (BG_COLOR)));

  if (!conform) {
    env->write (PAGE_MEDIUM, "paper");
    env->write (PAGE_PRINTED, "true");
  }

  // Typeset pages for printing
  // FIXME: we want maybe to typeset only the page we need ?

  eb= typeset_as_document (env, subtree (et, rp), reverse (rp));
}

picture
editor_rep::get_page_picture (int page) {
  box the_box= eb;
  page= min(N(the_box[0]), max (0, page-1));
  {
    box b=  the_box[0][page];
    double zoomf= 5.0;
    
    SI pixel= 5*PIXEL;
    SI w= b->x4 - b->x3;
    SI h= b->y4 - b->y3;
    SI ww= (SI) round (zoomf * w);
    SI hh= (SI) round (zoomf * h);
    int pxw= (ww+pixel-1)/pixel;
    int pxh= (hh+pixel-1)/pixel;
    picture pic= native_picture (pxw, pxh, 0, 0);
    renderer ren= picture_renderer (pic, zoomf);
    
    {
      tree bg= env->read (BG_COLOR);
      ren->set_background (bg);
      if (bg != "white" && bg != "#ffffff")
        ren->clear_pattern (0, (SI) -h, (SI) w, 0);
      
      rectangles rs;
      the_box[0]->sx(page)= 0;
      the_box[0]->sy(page)= 0;
      the_box[0][page]->redraw (ren, path (0), rs);
      //      if (i<end-1) ren->next_page ();
    }
    return pic;
  }
}


void
editor_rep::get_page_image (url name, int page, string image_dpi) {

  bool conform= true;
  url  orig= resolve (name, "");

  typeset_preamble ();
  
  // Set environment variables for printing

  typeset_prepare ();
  env->write (DPI, image_dpi);
  env->write (PAGE_SHOW_HF, "true");
  env->write (PAGE_SCREEN_MARGIN, "false");
  env->write (PAGE_BORDER, "none");
  if (is_func (env->read (BG_COLOR), PATTERN))
    env->write (BG_COLOR, env->exec (env->read (BG_COLOR)));

  if (!conform) {
    env->write (PAGE_MEDIUM, "paper");
    env->write (PAGE_PRINTED, "true");
  }

  // Typeset pages for printing
  // FIXME: we want maybe to typeset only the page we need ?

  box the_box= typeset_as_document (env, subtree (et, rp), reverse (rp));

  page= min(N(the_box[0]), max (0, page-1));
  the_box[0]->sx(page)= 0;
  the_box[0]->sy(page)= 0;
  
  {
    box b=  the_box[0][page];
    double zoomf= 5.0;

    SI pixel= 5*PIXEL;
    SI w= b->x4 - b->x3;
    SI h= b->y4 - b->y3;
    SI ww= (SI) round (zoomf * w);
    SI hh= (SI) round (zoomf * h);
    int pxw= (ww+pixel-1)/pixel;
    int pxh= (hh+pixel-1)/pixel;
    picture pic= native_picture (pxw, pxh, 0, 0);
    renderer ren= picture_renderer (pic, zoomf);

    {
      tree bg= env->read (BG_COLOR);
      ren->set_background (bg);
      if (bg != "white" && bg != "#ffffff")
        ren->clear_pattern (0, (SI) -h, (SI) w, 0);
      
      rectangles rs;
      the_box[0]->sx(page)= 0;
      the_box[0]->sy(page)= 0;
      the_box[0][page]->redraw (ren, path (0), rs);
      //      if (i<end-1) ren->next_page ();
    }

    save_picture (name, pic);
    tm_delete (ren);

  }
}
