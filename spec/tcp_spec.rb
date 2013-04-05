require 'spec_helper'

describe Rbuv::Tcp do
  it { should be_a_kind_of Rbuv::Stream }

  context "#bind" do
    tcp = Rbuv::Tcp.new
    tcp.bind '127.0.0.1', 60000
  end
end
