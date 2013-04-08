module Rbuv
  class Timer
    def self.start(*args)
      self.new.start(*args) { |*block_args| yield(*block_args) }
    end
  end
end
