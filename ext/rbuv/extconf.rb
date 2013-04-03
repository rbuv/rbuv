require 'mkmf'
require 'rbconfig'

case RbConfig::CONFIG["host_os"]
when /darwin/
  have_framework("CoreFoundation")
  have_framework("CoreServices") if RUBY_ENGINE =~ /rbx/
end

dir_config('uv')
libuv_ok = have_library('uv', 'uv_run', ['uv.h'])

##
# Adds -DRBUV_DEBUG for compilation
# To turn it on, use: --with-debug or --enable-debug
#

if debug_arg = with_config("debug") || enable_config("debug")
  debug_def = "-DRBUV_DEBUG"
  $defs.push(debug_arg.is_a?(String) ? "#{debug_def}=#{debug_arg}" : debug_def) unless $defs.include?(/\A#{debug_arg}/)
end

case RUBY_ENGINE
when /rbx/
  $defs.push("-DRBUV_RBX") unless $defs.include? "-DRBUV_RBX"
end

create_header
create_makefile('rbuv/rbuv')

unless libuv_ok
  File.open('Makefile', 'a') do |mf|
    mf.puts('include ${srcdir}/libuv.mk')
  end
end
