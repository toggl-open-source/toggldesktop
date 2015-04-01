require 'mkmf'
$libs = append_library($libs, "proxy")
create_makefile('Libproxy')
