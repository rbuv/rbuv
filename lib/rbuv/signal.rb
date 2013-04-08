module Rbuv
  class Signal
    ::Signal.list.each do |signame, signum|
      const_set signame, signum
    end
    def self.start(*args)
      self.new.start(*args) { |*block_args| yield(*block_args) }
    end
  end
end
