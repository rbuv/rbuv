lib = File.expand_path('../../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'rbuv'

puts "Rbuv version #{Rbuv::VERSION}"

clients = []

Rbuv.run do
  signal = Rbuv::Signal.new
  server = Rbuv::Tcp.new

  signal.start Rbuv::Signal::INT do
    clients.each(&:close)
    signal.close
    server.close
  end

  server.bind "127.0.0.1", 1234
  server.listen 10 do
    client = Rbuv::Tcp.new
    server.accept client
    clients << client
    client.read_start do |data, error|
      if error
        client.close
        clients.delete client
      else
        client.write data
      end
    end
  end
end

puts "Stopped!"
