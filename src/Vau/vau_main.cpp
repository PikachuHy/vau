//
//  vau_main.cpp
//  VauQt
//
//  Created by Massimiliano Gubinelli on 27/12/2022.
//

#include "scheme.hpp"
#include "vau_stuff.hpp"
#include "vau_buffer.hpp"
#include "vau_editor.hpp"


void
TeXmacs_main (int argc, char** argv) {
  
  set_env ("TEXMACS_PATH", "/Users/mgubi/t/svn-src/TeXmacs");
  
  initialize_scheme ();
  
  string tm_init_file= "/Users/mgubi/t/vau/src/Vau/init-vau.scm";
  bench_start ("initialize scheme");
  if (exists (tm_init_file)) exec_file (tm_init_file);
  bench_cumul ("initialize scheme");

  string name ("/Users/mgubi/t/svn-src/TeXmacs/examples/texts/accent-test.tm");
  string output ("/Users/mgubi/vau-test.pdf");
  tm_buffer buf= concrete_buffer_insist (name);
  editor ed (buf);
  ed->print_to_file (output);
}

int
main(int argc, char **argv) {
  the_et     = tuple ();
  the_et->obs= ip_observer (path ());
  //cache_initialize ();
  //bench_start ("initialize texmacs");
  //init_texmacs ();
  //bench_cumul ("initialize texmacs");
  start_scheme (argc, argv, TeXmacs_main);
  return 0;
}
