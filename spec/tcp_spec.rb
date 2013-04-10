require 'spec_helper'
require 'socket'

def port_in_use?(port, host='127.0.0.1')
  s = TCPServer.new host, port
  s.close
  false
rescue Errno::EADDRINUSE
  true
end

def port_alive?(port, host='127.0.0.1')
  s = TCPSocket.new host, port
  s.close
  false
rescue Errno::ECONNREFUSED
  true
end

describe Rbuv::Tcp do
  it { should be_a_kind_of Rbuv::Stream }

  it "#bind" do
    port_in_use?(60000).should be_false

    Rbuv.run do
      pending "this spec does't pass on linux machines, see #1 on github"
      begin
        tcp = Rbuv::Tcp.new
        tcp.bind '127.0.0.1', 60000

        port_in_use?(60000).should be_true
      ensure
        tcp.close
      end

      port_in_use?(60000).should be_false
    end
  end

  context "#listen" do
    it "when address not in use" do
      port_in_use?(60000).should be_false

      Rbuv.run do
        begin
          tcp = Rbuv::Tcp.new
          tcp.bind '127.0.0.1', 60000
          tcp.listen(10) { Rbuv.stop_loop }

          port_in_use?(60000).should be_true
        ensure
          tcp.close
        end

        port_in_use?(60000).should be_false
      end
    end

    it "when address already in use" do
      port_in_use?(60000).should be_false

      Rbuv.run do
        begin
          s = TCPServer.new '127.0.0.1', 60000

          tcp = Rbuv::Tcp.new
          tcp.bind '127.0.0.1', 60000
          expect { tcp.listen(10) {} }.to raise_error
        ensure
          s.close
          tcp.close
        end
      end
    end

    it "should call the on_connection callback when connection coming" do
      on_connection = mock
      on_connection.should_receive(:call).once

      Rbuv.run do
        tcp = Rbuv::Tcp.new
        tcp.bind '127.0.0.1', 60000

        tcp.listen(10) do
          on_connection.call
          tcp.close
        end

        sock = TCPSocket.new '127.0.0.1', 60000
        sock.close
      end
    end
  end

  it "#accept" do
    port_in_use?(60000).should be_false

    Rbuv.run do
      tcp = Rbuv::Tcp.new
      tcp.bind '127.0.0.1', 60000

      sock = nil

      tcp.listen(10) do |s|
        c = Rbuv::Tcp.new
        expect { s.accept(c) }.not_to raise_error
        sock.close
        tcp.close
      end

      sock = TCPSocket.new '127.0.0.1', 60000
    end
  end

  context "#close" do
    it "affect #closing?" do
      Rbuv.run do
        tcp = Rbuv::Tcp.new
        tcp.close do
          tcp.closing?.should be_true
        end
        tcp.closing?.should be_true
      end
    end

    it "call once" do
      on_close = mock
      on_close.should_receive(:call).once

      Rbuv.run do
        tcp = Rbuv::Tcp.new

        tcp.close do
          on_close.call
        end
      end
    end

    it "call multi-times" do
      on_close = mock
      on_close.should_receive(:call).once

      no_on_close = mock
      no_on_close.should_not_receive(:call)

      Rbuv.run do
        tcp = Rbuv::Tcp.new

        tcp.close do
          on_close.call
        end

        tcp.close do
          no_on_close.call
        end
      end
    end

  end
end
