require 'spec_helper'

describe Rbuv::Timer do
  it { should be_a_kind_of Rbuv::Handle }

  context "run in loop" do
    before(:each) { @skip_running_loop = false }
    after(:each) { Rbuv.run_loop unless @skip_running_loop }

    context "when timeout == 0" do
      context "#start" do
        it "when repeat == 0" do
          timer = Rbuv::Timer.new

          block = mock
          block.should_receive(:call).once

          timer.start 0, 0 do
            block.call
          end
        end

        it "when repeat != 0" do
          timer = Rbuv::Timer.new

          block = mock
          count_limit = 10
          block.should_receive(:call).exactly(count_limit)

          count = 0
          timer.start 0, 1 do |t|
            block.call
            count += 1
            if count >= count_limit
              t.stop
            end
          end
        end
      end # context "#start"

      it "#stop" do
        timer = Rbuv::Timer.new

        block = mock
        block.should_receive(:call).once

        timer.start 0, 1 do |t|
          block.call
          t.stop
        end
      end

      context "#active?" do
        it "should be false" do
          timer = Rbuv::Timer.new

          timer.start 0, 0 do |t|
            t.active?.should be_false
          end
        end

        it "should be true" do
          timer = Rbuv::Timer.new

          timer.start 0, 1 do |t|
            t.active?.should be_true
            t.stop
          end
        end
      end

      context "#repeat" do
        [0, 10, 100].each do |repeat|
          it "should eq #{repeat}" do
            timer = Rbuv::Timer.new

            timer.start 0, repeat do |t|
              t.repeat.should eq repeat
              t.stop
            end
          end
        end
      end

      context "#repeat=" do
        [0, 10, 100].each do |repeat|
          it "should eq #{repeat}" do
            timer = Rbuv::Timer.new

            timer.start 0, 0 do |t|
              t.repeat = repeat
            end
            Rbuv.run_loop
            timer.repeat.should eq repeat
            @skip_running_loop = true
          end
        end
      end

      context ".start" do
        context 'be valid' do
          it "when repeat == 0" do
            block = mock
            block.should_receive(:call).once

            Rbuv::Timer.start(0, 0) { block.call }
          end

          it "when repeat != 0" do
            block = mock
            block.should_receive(:call).once

            Rbuv::Timer.start 0, 1 do |t|
              block.call
              t.stop
            end
          end
        end

        context "won't segfault" do
          it "when repeat == 0" do
            Rbuv::Timer.start(0, 0) {}
            GC.start
          end

          it "when repeat != 0" do
            count = 0
            count_limit = 10
            Rbuv::Timer.start 0, 1 do |timer|
              GC.start
              count += 1
              if count >= count_limit
                timer.stop
              end
            end
            GC.start
          end
        end

      end # context ".start"

    end # context "timeout == 0"
  end # context "run in loop"
end
