class CMDTEST_fritz < Cmdtest::Testcase
  def setup
    import_directory "arsenal/", "arsenal/"
    @miniircd_pid = spawn("miniircd")
    sleep 1

    at_exit do
      begin
        Process.kill("TERM", -@miniircd_pid)
        Process.wait(@miniircd_pid)
      rescue Errno::ESRCH, Errno::ECHILD
      end
    end
  end

  def test_exit
    import_file "test/exit_run_script.py", "./"

    ii_pid = spawn("ii -i . -n emil -s localhost -p 6667")
    sleep 1

    shell "python exit_run_script.py"

    Process.kill("TERM", ii_pid)
    Process.wait(ii_pid)
  end

  #def test_hw
  #  import_file "test/hw_run_script.py", "./"

  #  ii_pid = spawn("ii -i . -n emil -s localhost -p 6667")
  #  sleep 1

  #  shell "python hw_run_script.py"

  #  Process.kill("TERM", ii_pid)
  #  Process.wait(ii_pid)
  #end
end
