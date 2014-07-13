describe Rbuv::Signal do
  it "#start" do
    block = double
    expect(block).to receive(:call).once

    Rbuv.run do
      sig = Rbuv::Signal.new
      sig.start 2 do
        block.call
        sig.close
      end

      `kill -2 #{Process.pid}`
    end
  end

  it "#stop" do
    block = double
    expect(block).to receive(:call).once

    Rbuv.run do
      sig = Rbuv::Signal.new
      sig.start 2 do
        block.call
        sig.stop
      end

      `kill -2 #{Process.pid}`
    end
  end
end
