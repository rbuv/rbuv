module Rbuv
  class Timer
    def self.start(*args)
      Timer.new.start(*args) { |timer| yield timer }
    end
  end
end
