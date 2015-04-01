/* libproxy.i */
%module Libproxy
%{
#include "../../libproxy/proxy.h"
%}

%typemap(out) char ** {
  VALUE arr = rb_ary_new2(255);
  int i;
  for (i=0; $1[i]; i++) {
    rb_ary_push(arr, rb_str_new2($1[i]));
  }
  $result = arr;
}

%typemap(freearg) char ** {
  free($source);
}

%include "../../libproxy/proxy.h"

