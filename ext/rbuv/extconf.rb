require 'mkmf'

dir_config('uv')
have_framework("CoreFoundation")
libuv_ok = have_library('uv', 'uv_run', ['uv.h'])

##
# Adds -DRBUV_DEBUG for compilation
# To turn it on, use: --with-debug or --enable-debug
#
if with_config("debug") or enable_config("debug")
  $defs.push("-DRBUV_DEBUG") unless $defs.include? "-DRBUV_DEBUG"
end

create_header
create_makefile('rbuv/rbuv')

unless libuv_ok
  File.open('Makefile', 'a') do |mf|
    mf.puts('include ${srcdir}/libuv.mk')
  end
end
