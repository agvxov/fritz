class CMDTEST_fritz < Cmdtest::Testcase
	def isolate_network_namespace
		# TODO
	end

	def float_sleep(t) # NOTE: sleep rounds to nearest second
		IO.select(nil, nil, nil, t)
	end

	def setup
	    @pid_list = []

	    import_directory "arsenal/", "arsenal/"

		isolate_network_namespace

	    @pid_list.push(spawn("miniircd"))
	    float_sleep(0.4)

	    at_exit do
	      begin
	    	@pid_list.each do |p|
	    		Process.kill("TERM", p)
	    		Process.wait(p)
	    	end
	      end
	    end
	end

	def test_exit
		import_file "test/exit_run_script.py", "./"

		shell "python exit_run_script.py"
	end

	#def test_hw
	#	import_file "test/hw_run_script.py", "./"

	#	@pid_list.push(spawn("ii -i . -n emil -s localhost -p 6667"))
	#	float_sleep(0.4)

	#	shell "python hw_run_script.py"
	#	float_sleep(2)

	#	puts File.read("localhost/out")
	#	raise if not file_equal("localhost/out", /.*/)
	#end
end
