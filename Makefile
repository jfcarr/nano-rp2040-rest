ARDCMD=arduino-cli
FQBNSTR=arduino:mbed_nano:nanorp2040connect
PORT=/dev/ttyACM0
SKETCHNAME=nano_rp2040_rest

default:
	@echo 'Targets:'
	@echo '  compile  -- Compile sketch, but don''t upload it.'
	@echo '  upload   -- Compile and upload sketch.'
	@echo '  monitor  -- Open the serial port monitor.'
	@echo '  format   -- Beautify your sketch code.'
	@echo '  clean    -- Remove binaries and object files.'

compile:
	$(ARDCMD) compile --fqbn $(FQBNSTR) $(SKETCHNAME)

upload: compile
	$(ARDCMD) upload -p $(PORT) --fqbn $(FQBNSTR) $(SKETCHNAME)

monitor:
	$(ARDCMD) monitor -p $(PORT)
