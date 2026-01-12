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

    shell "python exit_run_script.py"
  end
end
