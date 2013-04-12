lib = File.expand_path('../../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'rbuv'

puts "Rbuv version #{Rbuv::VERSION}"

Rbuv.run do
  client = Rbuv::Tcp.new
  client.connect "127.0.0.1", 1234 do
    client.read_start do |data, error|
      puts data unless error
      client.close
    end
    client.write "Hello, world"
  end
end

puts "Stopped!"
