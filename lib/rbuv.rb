require 'rbuv/rbuv'
require 'rbuv/version'
require 'rbuv/timer'

module Rbuv
  def self.run_loop
    Loop.run
  end

  def self.stop_loop
    Loop.stop
  end

  def self.run
    Timer.start 0, 0 do
      yield
    end
    self.run_loop
  end

  def self.run_block
    Timer.start 0, 0 do
      yield
    end
    Loop.run_once
  end
end
