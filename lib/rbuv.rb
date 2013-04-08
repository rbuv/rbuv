require 'rbuv/rbuv'
require 'rbuv/version'
require 'rbuv/timer'

module Rbuv
  class << self

    def run_loop
      Loop.run
    end

    def stop_loop
      Loop.stop
    end

    alias stop stop_loop

    def run
      Timer.start 0, 0 do
        yield
      end
      self.run_loop
    end

    def run_block
      Timer.start 0, 0 do
        yield
      end
      Loop.run_once
    end

  end
end
